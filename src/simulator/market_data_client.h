/**
*Author: Steve Zhong
*Creation Date: 2015年06月22日 星期一 00时13分41秒
*Last Modified: 2015年07月06日 星期一 11时37分19秒
*Purpose:
**/

#ifndef CLIENT_H_
#define CLIENT_H_

#include <iostream>
#include <string>
#include <vector>
#include <string>
#include <functional>
#include <algorithm>
#include <memory>
#include <cstdint> // int32_t, uint32_t

#include <ev.h>

#include "gateway/crawler.h"
#include "instrument/stock.h"

#include "simulator/cmd_display.h"
#include "simulator/file_handler.h"

#include "common/utility.h"
#include "common/io_aux.h"
#include "common/configurator.h"
#include "common/logger.h"
#include "common/common_defs.h"

namespace simulator {

struct ev_timer_udata {
    ev_timer timeout_watcher;
};
template <typename code_db>
class market_data_client {
public:
    using self_type     = market_data_client<code_db>;
    using crawler       = gateway::crawler;
	using displayer		= simulator::cmd_display;
	using file_handler	= simulator::file_handler;
	using utility		= common::utility;
    using configurator  = common::configurator;
    using logger        = common::logger;
    using io_aux        = common::io_aux;
public:
    market_data_client(configurator* config_):
        config(config_),
        db(new code_db())
        
    {
        code_path = config->get_value("stock.market_data.code.code_path", std::string());
        option_path = config->get_value("stock.market_data.code.option_path", std::string());
        ev_loop_ = EV_DEFAULT;
    }
    // 检查数据文件是否存在
    bool check_prerequisite()
    {
        return io_aux::check_files_exist({code_path + "all_code.ds",
                code_path + "cyb_code.ds",
                code_path + "zxb_code.ds",
                code_path + "sz_code.ds",
                code_path + "sh_code.ds",
                code_path + "code_jp_name.ds"});
    }
    void initialize()
    {
        // 生成数据文件
        if (!check_prerequisite()) {
            logger::log_info("系统初始化...");
            gen_code();
            gen_code_jp_name();
        }
        db->configure(code_path);
    }
    void gen_code_jp_name()
    {
        file_handler::read_code(code_vec, code_path, "all");
        std::vector<stock_basic> code_jp_name_vec;
        crawler_.gen_code_jp_name(code_vec, code_jp_name_vec);
        file_handler::save_code_jp_name(code_jp_name_vec, code_path);
    }
	void show_md(const std::string& code_str)
	{
		crawler_.split_code(code_str);
		crawler_.run_code(stock_vec);
		display();
	}
    void show_market()
    {
        cc_vec_string market_codes;
        config->get_multi_value("stock.market_data.code.markets", market_codes);
        crawler_.show_market(market_codes, market_vec);
        display_market();
    }
    void show_md_bk(const std::string& bk, uint32_t speed, int32_t top_num, std::string& order)
    {
        file_handler::read_code(code_vec, code_path, bk);
        cc_vec_string sub_code;
        uint32_t idx = 0;
        while (utility::get_vec_by_step(code_vec, idx, speed, sub_code)) {
            crawler_.list_stock(sub_code, stock_vec);
            logger::log_debug_variadic("get stock ", idx - speed, "-", idx, " finished!");
        }
        if (!sub_code.empty()) {
            crawler_.list_stock(sub_code, stock_vec);
        }
        select_stock(top_num, order);
        display_basic(top_num);
    }
    void add_option(const std::string& options)
    {
        cc_vec_string code_vec;
        utility::split(options, ',', code_vec);
        file_handler::add_option(code_vec, option_path);
    }
    void del_option(const std::string& options)
    {
        cc_vec_string code_vec;
        utility::split(options, ',', code_vec);
        file_handler::del_option(code_vec, option_path);
    }
    void show_option()
    {
        cc_vec_string code_vec;
        file_handler::read_option(code_vec, option_path);
        crawler_.list_stock(code_vec, stock_vec);
        display_basic();
        ev_timer_init(&timer_data.timeout_watcher, market_data_client::timeout_cb, 1.5, 1);
        ev_timer_start(ev_loop_, &timer_data.timeout_watcher);
        ev_run(ev_loop_, 0);
    }
private:
    static void timeout_cb(EV_P_ ev_timer *w, int revents)
    {
        ev_timer_udata* data = (ev_timer_udata*) w;
        logger::log_info("timeout!");
    }
public:
    void gen_code()
    {
        cc_vec_string sh_code_range, sz_code_range, zxb_code_range, cyb_code_range, all_code_range;
        cc_vec_string sh_code, sz_code, zxb_code, cyb_code, all_code;

        gen_code_cyb(cyb_code_range);
        gen_code_bk(cyb_code, cyb_code_range, "cyb");
        logger::log_info("创业板股票代码生成完毕！");

        gen_code_zxb(zxb_code_range);
        gen_code_bk(zxb_code, zxb_code_range, "zxb");
        logger::log_info("中小板股票代码生成完毕！");

        gen_code_sz(sz_code_range);
        gen_code_bk(sz_code, sz_code_range, "sz");
        sz_code.insert(sz_code.end(), zxb_code.begin(), zxb_code.end());
        sz_code.insert(sz_code.end(), cyb_code.begin(), cyb_code.end());
        logger::log_info("深圳市场股票代码生成完毕！");

        gen_code_sh(sh_code_range);
        gen_code_bk(sh_code, sh_code_range, "sh");
        logger::log_info("上海市场股票代码生成完毕！");

        all_code.insert(all_code.end(), sz_code.begin(), sz_code.end());
        all_code.insert(all_code.end(), sh_code.begin(), sh_code.end());
		file_handler::save_code(all_code, code_path, "all_code.ds");
        logger::log_info("A股市场股票代码生成完毕！");
    }
private:
    bool select_stock(int32_t top_num, std::string& order)
    {
        if (top_num == -1) top_num = stock_vec.size();

        if (order == "") { }
        else if (order == "zdf") {
            std::partial_sort(stock_vec.begin(),
                    stock_vec.begin() + top_num,
                    stock_vec.end(),
                    [](const stock& lsh, const stock& rsh) { return lsh.inc > rsh.inc; });
        }
        else if (order == "xj") {
            std::partial_sort(stock_vec.begin(),
                    stock_vec.begin() + top_num,
                    stock_vec.end(),
                    [](const stock& lsh, const stock& rsh) { return lsh.curr_price > rsh.curr_price; });
        }
        else if (order == "cjl") {
            std::partial_sort(stock_vec.begin(),
                    stock_vec.begin() + top_num,
                    stock_vec.end(),
                    [](const stock& lsh, const stock& rsh) { return lsh.volumn > rsh.volumn; });
        }
        else if (order == "cje") {
            std::partial_sort(stock_vec.begin(),
                    stock_vec.begin() + top_num,
                    stock_vec.end(),
                    [](const stock& lsh, const stock& rsh) { return lsh.turnover > rsh.turnover; });
        }
        else if (order == "zdfi") {
            std::partial_sort(stock_vec.begin(),
                    stock_vec.begin() + top_num,
                    stock_vec.end(),
                    [](const stock& lsh, const stock& rsh) { return lsh.inc < rsh.inc; });
        }
        else if (order == "xji") {
            std::partial_sort(stock_vec.begin(),
                    stock_vec.begin() + top_num,
                    stock_vec.end(),
                    [](const stock& lsh, const stock& rsh) { return lsh.curr_price < rsh.curr_price; });
        }
        else if (order == "cjli") {
            std::partial_sort(stock_vec.begin(),
                    stock_vec.begin() + top_num,
                    stock_vec.end(),
                    [](const stock& lsh, const stock& rsh) { return lsh.volumn < rsh.volumn; });
        }
        else if (order == "cjei") {
            std::partial_sort(stock_vec.begin(),
                    stock_vec.begin() + top_num,
                    stock_vec.end(),
                    [](const stock& lsh, const stock& rsh) { return lsh.turnover < rsh.turnover; });
        }
        else {
            logger::log_error("排序标准有问题!");
            return false;
        }
        return true;
    }
	void gen_code_bk(cc_vec_string& final_code, cc_vec_string& init_code, const std::string& bk)
	{
        uint32_t idx = 0;
        uint32_t speed = config->get_value("stock.market_data.request_speed", uint32_t());
        cc_vec_string sub_code;
        while (utility::get_vec_by_step(init_code, idx, speed, sub_code)) {
            crawler_.list_code(sub_code, final_code);
        }
        if (!sub_code.empty()) {
            crawler_.list_code(sub_code, final_code);
        }
		file_handler::save_code(final_code, code_path, bk + "_code.ds");
	}
	void gen_code_sh(cc_vec_string& codes)
	{
        int beg = config->get_value("stock.market_data.code.sh.beg", int());
        int end = config->get_value("stock.market_data.code.sh.end", int());
        utility::gen_range_str(beg, end, codes);
	}
	void gen_code_sz(cc_vec_string& codes)
	{
        int beg = config->get_value("stock.market_data.code.sz.beg", int());
        int end = config->get_value("stock.market_data.code.sz.end", int());
        utility::gen_range_str(beg, end, codes);
        config->get_multi_value("stock.market_data.code.sz.specials", codes);
    }
	void gen_code_zxb(cc_vec_string& codes)
	{
        int beg = config->get_value("stock.market_data.code.sz.zxb.beg", int());
        int end = config->get_value("stock.market_data.code.sz.zxb.end", int());
        utility::gen_range_str(beg, end, codes);
	}
	void gen_code_cyb(cc_vec_string& codes)
    {
        int beg = config->get_value("stock.market_data.code.sz.cyb.beg", int());
        int end = config->get_value("stock.market_data.code.sz.cyb.end", int());
        utility::gen_range_str(beg, end, codes);
	}
private:
	void display() {
		displayer::stock_info(stock_vec);
	}
    void display_market() {
        displayer::market_info(market_vec);
    }
    void display_basic(int32_t top_num = -1) {
        if (top_num == -1) {
            displayer::stock_info_basic(stock_vec);
        } else {
            displayer::stock_info_basic(std::vector<stock>(stock_vec.begin(), stock_vec.begin() + top_num));
        }
    }
private:
	crawler crawler_;
	cc_vec_string code_vec;

    std::string code_path; // 股票代码文件存放
    std::string option_path; // 自选股代码存放

	std::vector<stock> stock_vec;
	std::vector<market> market_vec;
    gen_code_map gen_code_map_;
    std::shared_ptr<configurator> config;
private:
    code_db* db;
private:
    // ev_timer timeout_watcher;
    struct ev_loop *ev_loop_;
    ev_timer_udata timer_data;
};

}
#endif
/**
*Author: Steve Zhong
*Creation Date: 2015年06月22日 星期一 00时13分41秒
*Last Modified: 2015年07月06日 星期一 21时54分22秒
*Purpose:
**/

#include <iostream>
#include <string>

#include "ccstock_config.h"

class stock_service {
public:
    stock_service(configurator* config_): md_client_(config_)
    {
        md_client_.initialize();
    }
    void show_stock_data(const std::string& code)
    {
        md_client_.show_stock(code);
    }
    void show_market()
    {
        md_client_.show_market();
    }
    void show_market_data(const std::string& bk, uint32_t speed, uint32_t top_num, std::string& order)
    {
        md_client_.show_md_bk(bk, speed, top_num, order);
    }
    void add_option(const std::string& options)
    {
        md_client_.add_option(options);
    }
    void del_option(const std::string& options)
    {
        md_client_.del_option(options);
    }
    void show_option()
    {
        md_client_.show_option();
    }
private:
    md_client md_client_;
};

int main(int argc, char* argv[]) {
    configurator *configurator_ = new configurator();
    configurator_->add_option("code,a", "查看个股信息，代码用','分开", std::string());
    configurator_->add_option("config,c", "设置配置文件", std::string());
    configurator_->add_option("data,d", "批量显示行情", std::string());
    configurator_->add_plain_option("market,m", "显示大盘行情");
    configurator_->add_option("top,t", "筛选前几个信息", int());
    configurator_->add_option("order,o", "排序规则", std::string());
    configurator_->add_option("add-option", "追加自选股,股票代码用','分开", std::string());
    configurator_->add_option("del-option", "删除自选股,股票代码用','分开", std::string());
    configurator_->add_plain_option("show-option,O", "查看自选股");
    configurator_->parse_command_line(argc, argv);

    if (configurator_->is_option_set("help")) {
        configurator_->show_options();
        return 0;
    }

	stock_service service(configurator_);
    if (configurator_->is_option_set("code")) {
        service.show_stock_data(configurator_->get_string_option("code"));        
    }
    if (configurator_->is_option_set("market")) {
        service.show_market();
    }
    if (configurator_->is_option_set("add-option")) {
        service.add_option(configurator_->get_string_option("add-option"));
    }
    if (configurator_->is_option_set("del-option")) {
        service.del_option(configurator_->get_string_option("del-option"));
    }
    if (configurator_->is_option_set("show-option")) {
        service.show_option();
    }
    if (configurator_->is_option_set("data")) {
        int32_t top_num = -1;
        std::string order("");
        if (configurator_->is_option_set("top")) {
            top_num = configurator_->get_int_option("top");
        }
        if (configurator_->is_option_set("order")) {
            order= configurator_->get_string_option("order");
        }
        service.show_market_data(configurator_->get_string_option("data"),
                configurator_->get_value("stock.market_data.request_speed", uint32_t()),
                top_num,
                order);
    }

	return 0;
}

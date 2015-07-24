/**
*Author: Steve Zhong
*Creation Date: 2015年07月23日 星期四 23时08分02秒
*Last Modified: 2015年07月24日 星期五 21时42分08秒
*Purpose:
**/

#include "dc_service.h"

namespace dc {

void dc_service::configure()
{
    config->add_plain_option("history,H", "获取历史数据");
    // config->parse_command_line(argc, argv);
    config->parse_xml_file("dc.xml");
    // 传递配置信息
    io_watcher.config = config;
    // 股票代码位置
    std::string code_path = config->get_value("data.code_path", std::string());
    // 设置历史数据文件存放位置
    history_client_.configure(code_path,
            config->get_value("data.locate.file.yahoo", std::string()));
}

void dc_service::run()
{
    logger::log_info_inline("请输入相关命令> ");
    ev_run(ev_loop, 0); 
}

// 标准输入回调函数
void dc_service::read_cb(EV_P_ ev_io *w, int revents)
{
    dc_io_data* io_data = (dc_io_data*) w;
    std::string command;
    std::cin >> command;
    // 判断输入的命令
    if (command == "history" || command == "H")
    {
        io_data->history_client->get_history_all();
    }
    logger::log_info_inline("请输入相关命令> ");
}

}
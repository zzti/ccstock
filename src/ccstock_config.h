/**
*Author: Steve Zhong
*Creation Date: 2015年06月22日 星期一 00时13分41秒
*Last Modified: 2015年07月11日 星期六 20时27分05秒
*Purpose:
**/

#ifndef CCSTOCK_CONFIG_H_
#define CCSTOCK_CONFIG_H_

#include "gateway/market_data_crawler.h"
#include "gateway/code_crawler.h"

#include "simulator/market_data_client.h"
#include "simulator/code_initializer.h"
#include "simulator/option_manager.h"
#include "simulator/instrument/code_db.h"

#include "simulator/ui/text_based_interface.h"

#include "common/configurator.h"

using code_db       = simulator::code_db;
using md_crawler    = gateway::market_data_crawler;
using md_client_t     = simulator::market_data_client<code_db, md_crawler>;

using tb_displayer = simulator::ui::text_based_interface;
using option_manager_t = simulator::option_manager<code_db, md_crawler, tb_displayer>;

using code_crawler    = gateway::code_crawler;
using code_initializer_t     = simulator::code_initializer<code_crawler>;

using configurator  = common::configurator;

#endif

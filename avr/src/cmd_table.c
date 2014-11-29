/*
 * cmd_table.c - command table
 *
 * Written by
 *  Christian Vogelgsang <chris@vogelgsang.org>
 *
 * This file is part of plipbox.
 * See README for copyright notice.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307  USA.
 *
 */

#include "cmd_table.h"
#include "uartutil.h"
#include "util.h"

#include "net/net.h"
#include "param.h"
#include "stats.h"
#include "eth_state.h"
#include "pb_state.h"

COMMAND(cmd_quit)
{
  return CMD_QUIT;
}

COMMAND(cmd_device_reset)
{
  return CMD_RESET;
}

COMMAND(cmd_version)
{
  uart_send_pstring(PSTR(VERSION " " BUILD_DATE "\r\n"));
  return CMD_OK;
}

COMMAND(cmd_param_dump)
{
  param_dump();
  return CMD_OK;
}

COMMAND(cmd_param_save)
{
  u08 result = param_save();
  if(result == PARAM_OK) {
    return CMD_OK;
  } else {
    return CMD_MASK_ERROR | result;
  }
}

COMMAND(cmd_param_load)
{
  u08 result = param_load();
  if(result == PARAM_OK) {
    return CMD_OK;
  } else {
    return CMD_MASK_ERROR | result;
  }
}

COMMAND(cmd_param_reset)
{
  param_reset();
  return CMD_OK;
}

COMMAND(cmd_param_toggle)
{
  u08 group = argv[0][0];
  u08 type = argv[0][1];
  u08 *val = 0;
  
  if(group == 'd') {
    switch(type) {
      case 'd': val = &param.dump_dirs; break;
      case 'e': val = &param.dump_eth; break;
      case 'i': val = &param.dump_ip; break;
      case 'a': val = &param.dump_arp; break;
      case 'p': val = &param.dump_proto; break;
      case 'l': val = &param.dump_plip; break;
      default: return CMD_PARSE_ERROR;
    }
  }
  else if(group == 'f') {
    switch(type) {
      case 'd': val = &param.full_duplex; break;
      case 'c': val = &param.flow_ctl; break;
      case 'e': val = &param.filter_eth; break;
      case 'p': val = &param.filter_plip; break;
      case 'l': val = &param.loop_back; break;
      default: return CMD_PARSE_ERROR;
    }
  }
  else {
    return CMD_PARSE_ERROR;
  }
  
  if(argc == 1) {
    // toggle value if no argument is given
    *val = *val ? 0 : 1;
  } else {
    u08 new_val;
    if(parse_byte(argv[1],&new_val)) {
      *val = new_val;
    } else {
      return CMD_PARSE_ERROR;
    }
  }
  return CMD_OK;
}

COMMAND(cmd_param_word)
{
  u08 group = argv[0][0];
  u08 type = argv[0][1];
  u16 *val = 0;
  
  if(group == 't') {
    switch(type) {
      case 'l': val = &param.test_plen; break;
      case 't': val = &param.test_ptype; break;
      default: return CMD_PARSE_ERROR;
    }
  }
  else {
    return CMD_PARSE_ERROR;
  }
  
  if(argc == 1) {
    return CMD_PARSE_ERROR;
  } else {
    u16 new_val;
    if(parse_word(argv[1],&new_val)) {
      *val = new_val;
    } else {
      return CMD_PARSE_ERROR;
    }
  }
  return CMD_OK;
}

COMMAND(cmd_param_mac_addr)
{
  u08 mac[6];

  if(net_parse_mac(argv[1], mac)) {
    net_copy_mac(mac, param.mac_addr);
    return CMD_OK;
  } else {
    return CMD_PARSE_ERROR;
  }
}

COMMAND(cmd_param_ip_addr)
{
  u08 ip[4];

  if(net_parse_ip(argv[1], ip)) {
    net_copy_ip(ip, param.test_ip);
    return CMD_OK;
  } else {
    return CMD_PARSE_ERROR;
  }  
}

COMMAND(cmd_stats_dump)
{
  stats_dump();
  return CMD_OK;
}

COMMAND(cmd_stats_reset)
{
  stats_reset();
  return CMD_OK;
}

COMMAND(cmd_ether_configure)
{
  //eth_state_configure();
  return CMD_OK;
}

COMMAND(cmd_ether_init)
{
  //eth_state_init();
  return CMD_OK;
}

COMMAND(cmd_ether_shutdown)
{
  //eth_state_shutdown();
  return CMD_OK;
}

COMMAND(cmd_plipbox_init)
{
  //pb_state_init();
  return CMD_OK;
}

COMMAND(cmd_help)
{
  uart_send_pstring(PSTR(
    "q        quit command mode\r\n"
    "r        soft reset device\r\n"
    "v        print plipbox version\r\n"
    "\r\n"
    "p        print parameters\r\n"
    "ps       save parameters to EEPROM\r\n"
    "pl       load parameters from EEPROM\r\n"
    "\r\n"
    "sd       dump statistics\r\n"
    "sr       reset statistics\r\n"
    "\r\n"
    "ec       re-configure ethernet\r\n"
    "ei       re-init ethernet\r\n"
    "es       ethernet shutdown\r\n"
    "pi       re-init plipbox\r\n"
    "\r\n"
    "fd [on]  enable full duplex mode\r\n"
    "fl [on]  enable loop back\r\n"
    "fc [on]  enable flow control for ETH packets\r\n"
    "fp [on]  enable filtering of PLIP packets\r\n"
    "fe [on]  enable filtering of ETH packets\r\n"
    "\r\n"
    "dd <val> select diagnosis directions. add values:\r\n"
    "         [1=plip(rx),2=eth(rx),4=plip(tx),8=eth(tx)]\r\n"
    "de [on]  toggle dump ethernet packets\r\n"
    "di [on]  toggle dump IP contents\r\n"
    "da [on]  toggle dump ARP contents\r\n"
    "dp [on]  toggle dump TCP/UDP contents\r\n"
    "dl [on]  toggle dump PLIP info\r\n"
    "\r\n"
    "tl       length of test packets\r\n"
    "tt       eth type of test packets\r\n"
    "ti <ip>  test IP address\r\n"
  ));
  return CMD_OK;
}

// ----- Names -----
CMD_NAME("q", cmd_quit);
CMD_NAME("r", cmd_device_reset);
CMD_NAME("v", cmd_version);
  // param
CMD_NAME("p", cmd_param_dump );
CMD_NAME("ps", cmd_param_save );
CMD_NAME("pl", cmd_param_load );
CMD_NAME("pr", cmd_param_reset );
  // stats
CMD_NAME("sd", cmd_stats_dump );
CMD_NAME("sr", cmd_stats_reset );
  // ethernet
CMD_NAME("ec", cmd_ether_configure );
CMD_NAME("ei", cmd_ether_init );
CMD_NAME("es", cmd_ether_shutdown);
CMD_NAME("pi", cmd_plipbox_init );
  // options
CMD_NAME("m", cmd_gen_m );
CMD_NAME("fd", cmd_gen_fd );
CMD_NAME("fc", cmd_gen_fc );
CMD_NAME("fp", cmd_gen_fp );
CMD_NAME("fe", cmd_gen_fe );
CMD_NAME("fl", cmd_gen_fl );
  // dump commands
CMD_NAME("dd", cmd_gen_dd );
CMD_NAME("de", cmd_gen_de );
CMD_NAME("di", cmd_gen_di );
CMD_NAME("da", cmd_gen_da );
CMD_NAME("dp", cmd_gen_dp );
CMD_NAME("dl", cmd_gen_dl );
CMD_NAME("dy", cmd_gen_dy );
  // test
CMD_NAME("tl", cmd_gen_tl );
CMD_NAME("tt", cmd_gen_tt );
CMD_NAME("ti", cmd_gen_ti );
  // help
CMD_NAME("?", cmd_help );

// ----- Entries -----
const cmd_table_t PROGMEM cmd_table[] = {
  CMD_ENTRY(cmd_quit),
  CMD_ENTRY(cmd_device_reset),
  CMD_ENTRY(cmd_version),
  // param
  CMD_ENTRY(cmd_param_dump),
  CMD_ENTRY(cmd_param_save),
  CMD_ENTRY(cmd_param_load),
  CMD_ENTRY(cmd_param_reset),
  // stats
  CMD_ENTRY(cmd_stats_dump),
  CMD_ENTRY(cmd_stats_reset),
  // ethernet
  CMD_ENTRY(cmd_ether_configure),
  CMD_ENTRY(cmd_ether_init),
  CMD_ENTRY(cmd_ether_shutdown),
  CMD_ENTRY(cmd_plipbox_init),
  // options
  CMD_ENTRY_NAME(cmd_param_mac_addr, cmd_gen_m),
  CMD_ENTRY_NAME(cmd_param_toggle, cmd_gen_fd),
  CMD_ENTRY_NAME(cmd_param_toggle, cmd_gen_fc),
  CMD_ENTRY_NAME(cmd_param_toggle, cmd_gen_fp),
  CMD_ENTRY_NAME(cmd_param_toggle, cmd_gen_fe),
  CMD_ENTRY_NAME(cmd_param_toggle, cmd_gen_fl),
  // dump commands
  CMD_ENTRY_NAME(cmd_param_toggle, cmd_gen_dd),
  CMD_ENTRY_NAME(cmd_param_toggle, cmd_gen_de),
  CMD_ENTRY_NAME(cmd_param_toggle, cmd_gen_di),
  CMD_ENTRY_NAME(cmd_param_toggle, cmd_gen_da),
  CMD_ENTRY_NAME(cmd_param_toggle, cmd_gen_dp),
  CMD_ENTRY_NAME(cmd_param_toggle, cmd_gen_dl),
  CMD_ENTRY_NAME(cmd_param_toggle, cmd_gen_dy),
  // test
  CMD_ENTRY_NAME(cmd_param_word, cmd_gen_tl),
  CMD_ENTRY_NAME(cmd_param_word, cmd_gen_tt),
  CMD_ENTRY_NAME(cmd_param_ip_addr, cmd_gen_ti),
  // help
  CMD_ENTRY(cmd_help),
  { 0,0 } // last entry
};

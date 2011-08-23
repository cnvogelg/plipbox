/*
 * cmd.c - command parsing
 *
 * Written by
 *  Christian Vogelgsang <chris@vogelgsang.org>
 *
 * This file is part of plip2slip.
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

#include "cmd.h"
#include "uartutil.h"
#include "uart.h"
#include "util.h"
#include "ser_parse.h"
#include "param.h"
#include "stats.h"
#include "bench.h"

u08 cmd_parse(u08 len, const char *cmd)
{
  u08 status;
  
  switch(cmd[0]) {
    // ----- v) version -----
    case 'v':
      uart_send_string("plip2slip " VERSION);
      uart_send_crlf();
      return SER_PARSE_CMD_OK;
    
    // ----- x) exit -----
    case 'x':
      return SER_PARSE_CMD_EXIT;
    
    // ----- s) stats -----
    case 's':
      if(len==1) {
        // show stats
        stats_dump();
        return SER_PARSE_CMD_OK;
      } else {
        switch(cmd[1]) {
          case 'r': // stats reset
            stats_reset();
            return SER_PARSE_CMD_OK;        
          default:
            return SER_PARSE_CMD_UNKNOWN;
        }
      }
    
    // ----- b) bench -----
    case 'b':
      bench_dump();
      return SER_PARSE_CMD_OK;
    
    // ----- p) param -----
    case 'p':
      if(len==1) {
        // show params
        param_dump();
        return SER_PARSE_CMD_OK;
      } else {
        switch(cmd[1]) {
          case 's': // param save
            status = param_save();
            return (status == PARAM_OK) ? SER_PARSE_CMD_OK : SER_PARSE_CMD_FAIL;
          case 'l': // param load
            status = param_load();
            return (status == PARAM_OK) ? SER_PARSE_CMD_OK : SER_PARSE_CMD_FAIL;
          case 'r': // param reset
            param_reset();
            return SER_PARSE_CMD_OK;
          default:
            return SER_PARSE_CMD_UNKNOWN;
        }
      }
      return SER_PARSE_CMD_UNKNOWN;
    
    // ----- m) mode -----
    case 'm':
      if(len==2) {
        u08 value;
        status = parse_nybble(cmd[1],&value);
        if(!status || (value >= PARAM_MODE_TOTAL_NUMBER)) {
          return SER_PARSE_CMD_FAIL;
        }
        param.mode = value;
        return SER_PARSE_CMD_OK;
      }
      return SER_PARSE_CMD_UNKNOWN;
      
    // unknown command
    default:
      return SER_PARSE_CMD_UNKNOWN;
  }
}

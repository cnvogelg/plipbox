/*
 * cmd.c - command parsing
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

#include <string.h>

#include "cmd.h"
#include "cmd_table.h"
#include "cmdkey_table.h"
#include "uartutil.h"
#include "uart.h"

#define MAX_LINE  32
#define MAX_ARGS  4

u08 cmd_line[MAX_LINE];
u08 *cmd_args[MAX_ARGS];

static u08 enter_line(void)
{
  u08 cmd_pos = 0;
  while(1) {
    u08 c = uart_read();
    if(c=='\n') {
      uart_send_crlf();
      break;
    } 
    else if((c==127)||(c==8)) {
      if(cmd_pos > 0) {
        cmd_pos--;
        uart_send(c);
      }
    }
    else if((c>=32)&&(c<128)) {
      cmd_line[cmd_pos] = c;
      uart_send(c);
      cmd_pos ++;
      // max line reached -> end command
      if(cmd_pos == (MAX_LINE-1)) {
        uart_send_crlf();
        break;
      }
    }
  }
  cmd_line[cmd_pos] = '\0';
  return cmd_pos;
}

static u08 parse_args(u08 len)
{
  u08 pos = 0;
  u08 argc = 0;
  u08 stay = 1;
  while(stay) {
    // skip leading spaces
    while(cmd_line[pos] == ' ') {
      pos++;
    }
    // end reached?
    if(cmd_line[pos] == '\0') {
      break;
    }
    // start new arg
    cmd_args[argc] = cmd_line + pos;
    argc++;
    // seek end
    while(cmd_line[pos] != ' ') {
      if(cmd_line[pos] == '\0') {
        stay = 0;
        break;
      }
      pos++;
    }
    cmd_line[pos] = '\0';
    pos++;
  }
  return argc;
}

static void cmd_loop(void)
{
  uart_send_pstring(PSTR("cmd\r\n"));
  u08 num_chars = 1;
  u08 status = CMD_OK;
  while(status != CMD_QUIT) {
    // print prompt
    uart_send_pstring(PSTR("> "));
    // read line
    num_chars = enter_line();
    if(num_chars > 0) {
#ifdef DEBUG
      uart_send_hex_byte_crlf(num_chars);
#endif
      // parse line into args
      u08 argc = parse_args(num_chars);
      if(argc > 0) {
#ifdef DEBUG
        uart_send_hex_byte_crlf(argc);
        for(u08 i=0;i<argc;i++) {
          uart_send_string((const char *)cmd_args[i]);
          uart_send_crlf();
        }
#endif
        // find command
        cmd_table_t *ptr = cmd_table;
        cmd_table_t *found = 0;
        while(ptr->name != 0) {
          if(strcmp((const char *)cmd_args[0], ptr->name)==0) {
            found = ptr;
            break;
          }
          ptr ++;
        }
        // got a command
        if(found != 0) {
          // execute command
          status = found->func(argc, (const u08 **)&cmd_args);
          // show result
          uart_send_hex_byte_spc(status);
          u08 type = status & CMD_MASK;
          if(type == CMD_MASK_OK) {
            uart_send_pstring(PSTR("OK\r\n"));
          } else if(type == CMD_MASK_SYNTAX) {
            uart_send_pstring(PSTR("SYNTAX\r\n")); 
          } else if(type == CMD_MASK_ERROR) {
            uart_send_pstring(PSTR("ERROR\r\n"));
          } else {
            uart_send_pstring(PSTR("???\r\n"));
          }
        } else {
          uart_send_pstring(PSTR("HUH?\r\n"));
        }
      }
    }
  }
  uart_send_pstring(PSTR("bye\r\n"));
}

void cmd_worker(void)
{
  // small hack to enter commands
  if(uart_read_data_available()) {
    u08 cmd = uart_read();
    if(cmd == '\n') {
      // enter command loop
      cmd_loop();
    } else {
      // search command
      cmdkey_table_t *ptr = cmdkey_table;
      cmdkey_table_t *found = 0;
      while(ptr->key != 0) {
        if(ptr->key == cmd) {
          found = ptr;
          break;
        }
        ptr++;
      }
      // got a command?
      if(found != 0) {
        found->func();
      }
    }
  }  
}

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
#include "hw_uart.h"

#define MAX_LINE  32
#define MAX_ARGS  4

u08 cmd_line[MAX_LINE];
u08 *cmd_args[MAX_ARGS];

static u08 enter_line(void)
{
  u08 cmd_pos = 0;
  while(1) {
    u08 c = hw_uart_read();
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

static void show_cmd_help(void)
{
  uart_send_pstring(PSTR("Command Help:\r\n"));
  const cmd_table_t * ptr = cmd_table;
  while(1) {
    const char * name = (const char *)pgm_read_word(&ptr->name);
    if(name == 0) {
      break;
    }
    const char * help = (const char *)pgm_read_word(&ptr->help);

    // show command
    u08 pos = 0;
    while(1) {
      u08 c = pgm_read_byte(name);
      if(c==0) {
        break;
      }
      uart_send(c);
      pos++;
      name++;
    }

    // pad
    while(pos < 10) {
      uart_send(' ');
      pos++;
    }

    // show help
    uart_send_pstring(help);
    uart_send_crlf();

    ptr ++;
  }
}

static u08 cmd_loop(void)
{
  uart_send_pstring(PSTR("Command Mode. Enter <?>+<return> for help and <q>+<return> to leave.\r\n"));
  u08 num_chars = 1;
  u08 status = CMD_OK;
  u08 result = CMD_WORKER_DONE;
  while(status != CMD_QUIT) {
    // print prompt
    uart_send_pstring(PSTR("> "));
    // read line
    num_chars = enter_line();
    if(num_chars > 0) {
#ifdef DEBUG_CMD
      uart_send_hex_byte(num_chars);
      uart_send_crlf();
#endif
      // parse line into args
      u08 argc = parse_args(num_chars);
      if(argc > 0) {
#ifdef DEBUG_CMD
        uart_send_hex_byte(argc);
        uart_send_crlf();
        for(u08 i=0;i<argc;i++) {
          uart_send_string((const char *)cmd_args[i]);
          uart_send_crlf();
        }
#endif
        // help?
        if(cmd_args[0][0] == '?') {
          show_cmd_help();
        } else {
          // find command
          const cmd_table_t * ptr = cmd_table;
          const cmd_table_t * found = 0;
          while(1) {
            const char * name = (const char *)pgm_read_word(&ptr->name);
            if(name == 0) {
              break;
            }
            if(strcmp_P((const char *)cmd_args[0], name)==0) {
              found = ptr;
              break;
            }
            ptr ++;
          }
          // got a command
          if(found != 0) {
            // execute command
            cmd_table_func_t func = (cmd_table_func_t)pgm_read_word(&found->func);
            status = func(argc, (const u08 **)&cmd_args);
            // show result
            uart_send_hex_byte(status);
            uart_send_spc();
            u08 type = status & CMD_MASK;
            if(type == CMD_MASK_OK) {
              if(status == CMD_RESET) {
                uart_send_pstring(PSTR("RESET\r\n"));
                return CMD_WORKER_RESET;
              } else if(status == CMD_OK_RESTART) {
                uart_send_pstring(PSTR("OK (NEED RESTART)\r\n"));
                result = CMD_WORKER_RESTART;
              } else {
                uart_send_pstring(PSTR("OK\r\n"));
              }
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
  }
  if(result == CMD_WORKER_DONE) {
    uart_send_pstring(PSTR("bye\r\n"));
  } else {
    uart_send_pstring(PSTR("bye. restarting...\r\n"));
  }
  return result;
}

static void show_cmdkey_help(void)
{
  uart_send_pstring(PSTR("Command Key Help:\r\n"));
  const cmdkey_table_t *ptr = cmdkey_table;
  while(1) {
    u08 key = pgm_read_byte(&ptr->key);
    if(key == 0) {
      break;
    }
    const char *help = (const char *)pgm_read_word(&ptr->help);

    uart_send(key);
    uart_send_pstring(PSTR("   "));
    uart_send_pstring(help);
    uart_send_crlf();

    ptr++;
  }

}

u08 cmd_worker(void)
{
  u08 result = CMD_WORKER_IDLE;

  // small hack to enter commands
  if(hw_uart_read_data_available()) {
    u08 cmd = hw_uart_read();
    if(cmd == '\n') {
      // enter command loop
      result = cmd_loop();
    } else if(cmd == '?') {
      // show help
      show_cmdkey_help();
      result = CMD_WORKER_DONE;
    } else {
      // search command
      const cmdkey_table_t *ptr = cmdkey_table;
      const cmdkey_table_t *found = 0;
      while(1) {
        u08 key = pgm_read_byte(&ptr->key);
        if(key == cmd) {
          found = ptr;
          break;
        }
        if(key == 0) {
          break;
        }
        ptr++;
      }
      // got a key command?
      if(found != 0) {
        cmdkey_func_t func = (cmdkey_func_t)pgm_read_word(&found->func);
        func();
        result = CMD_WORKER_DONE;
      }
    }
  }

  return result;
}

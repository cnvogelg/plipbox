#include "cmd.h"
#include "uartutil.h"
#include "uart.h"
#include "util.h"
#include "ser_parse.h"
#include "param.h"

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

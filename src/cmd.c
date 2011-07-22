#include "cmd.h"
#include "uartutil.h"
#include "uart.h"

u08 cmd_parse(u08 len, const char *cmd)
{
  switch(cmd[0]) {
    // ----- v) version -----
    case 'v':
      uart_send_string("plip2slip " VERSION);
      uart_send_crlf();
      return CMD_OK;
    // ----- x) exit -----
    case 'x':
      return CMD_EXIT;
    // unknown command
    default:
      return CMD_UNKNOWN;  
  }
}

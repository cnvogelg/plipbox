#include "log.h"
#include "uart.h"
#include "uartutil.h"

#define LOG_SIZE  16

static u08 codes[LOG_SIZE];
static u08 num;
static u08 pos;
static u16 total;

void log_init(void)
{
  num = 0;
  pos = 0;
  total = 0;
}

void log_add(u08 code)
{
  total++;
  if(num == LOG_SIZE) {
    codes[pos] = code;
    pos ++;
    if(pos == LOG_SIZE) {
      pos = 0;
    }
  } else {
    codes[pos] = code;
    pos++;
    num++;
  }
}

void log_dump(void)
{
  uart_send_string("log: ");
  uart_send_hex_word_crlf(total);
  int off = pos;
  for(int i=0;i<num;i++) {
    uart_send_hex_byte_crlf(codes[off]);
    if(off == 0) {
      off = LOG_SIZE -1 ;
    } else {
      off--;
    }
  }
}

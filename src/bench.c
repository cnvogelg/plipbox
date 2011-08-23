#include "bench.h"
#include "timer.h"
#include "uart.h"
#include "uartutil.h"

typedef struct {
  u16 count;
  u16 bytes;
  u16 time;
} bench_t;
  
static bench_t current;
static bench_t stored;

void bench_begin(void)
{
  current.count = 0;
  current.bytes = 0;
  
  timer2_10ms = 0;
}

void bench_end(void)
{
  stored.time = timer2_10ms;
  stored.count = current.count;
  stored.bytes = current.bytes;
}

void bench_submit(u16 bytes)
{
  current.count ++;
  current.bytes += bytes;
}

void bench_dump(void)
{
  uart_send_string("count:");
  uart_send_hex_word_crlf(stored.count);
  uart_send_string("bytes:");
  uart_send_hex_word_crlf(stored.bytes);
  uart_send_string("time: ");
  uart_send_hex_word_crlf(stored.time);
}
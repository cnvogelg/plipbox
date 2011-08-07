#include "stats.h"
#include "timer.h"
#include "uartutil.h"
#include "uart.h"

stats_t stats;

void stats_reset(void)
{
  stats.pkt_count = 0;
  stats.pkt_tx_err = 0;
  stats.pkt_rx_err = 0;
  stats.pkt_bytes = 0;
  stats.pkt_time = 0;
  
  timer2_10ms = 0;
}

void stats_capture(void)
{
  stats.pkt_time = timer2_10ms;
}

void stats_dump(void)
{
  stats_capture();
  
  uart_send_crlf();
  uart_send('c');
  uart_send_hex_word_crlf(stats.pkt_count);

  u16 rx = stats.pkt_rx_err;
  if(rx > 0) {
    uart_send('r');
    uart_send_hex_word_crlf(rx);
    uart_send(':');
    uart_send_hex_byte_crlf(stats.pkt_last_rx_err);
  }
  u16 tx = stats.pkt_tx_err;
  if(tx > 0) {
    uart_send('t');
    uart_send_hex_word_crlf(tx);
    uart_send(':');
    uart_send_hex_byte_crlf(stats.pkt_last_tx_err);
  }
  
  uart_send('b');
  uart_send_hex_dword_crlf(stats.pkt_bytes);
  uart_send('t');
  uart_send_hex_word_crlf(stats.pkt_time);

  stats_reset();
}

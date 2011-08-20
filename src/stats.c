#include "stats.h"
#include "timer.h"
#include "uartutil.h"
#include "uart.h"

stats_t stats;

void stats_reset(void)
{
  stats.pkt_rx_cnt = 0;
  stats.pkt_tx_cnt = 0;
  stats.pkt_rx_err = 0;
  stats.pkt_tx_err = 0;
  stats.pkt_rx_bytes = 0;
  stats.pkt_tx_bytes = 0;
  
  stats.pkt_time = 0;
}

void stats_reset_timing(void)
{  
  timer2_10ms = 0;
}

void stats_capture_timing(void)
{
  stats.pkt_time = timer2_10ms;
}

void stats_dump(void)
{
  uart_send_string("rx_cnt: ");
  uart_send_hex_word_crlf(stats.pkt_rx_cnt);
  uart_send_string("rx_byte:");
  uart_send_hex_dword_crlf(stats.pkt_rx_bytes);

  u16 rx = stats.pkt_rx_err;
  if(rx > 0) {
    uart_send_string("rx_err: ");
    uart_send_hex_word_crlf(rx);
    uart_send_string(" last:");
    uart_send_hex_byte_crlf(stats.pkt_last_rx_err);
  }

  uart_send_string("tx_cnt: ");
  uart_send_hex_word_crlf(stats.pkt_tx_cnt);
  uart_send_string("tx_byte:");
  uart_send_hex_dword_crlf(stats.pkt_tx_bytes);

  u16 tx = stats.pkt_tx_err;
  if(tx > 0) {
    uart_send_string("tx_err:");
    uart_send_hex_word_crlf(tx);
    uart_send_string(" last:");
    uart_send_hex_byte_crlf(stats.pkt_last_tx_err);
  }
  
  uart_send_string("t(10ms):");
  uart_send_hex_word_crlf(stats.pkt_time);
}

#include "arch.h"
#include "types.h"

#include "nic.h"
#include "nic_test.h"
#include "uartutil.h"
#include "pkt_buf.h"
#include "param.h"
#include "arp.h"
#include "eth.h"
#include "param.h"
#include "dump.h"

void nic_test_status(void)
{
  uart_send_time_stamp_spc();
  uart_send_pstring(PSTR("nic_test_status:num="));
  u08 num_pkt = nic_rx_num_pending();
  uart_send_hex_byte(num_pkt);
  uart_send_crlf();
}

void nic_test_tx(void)
{
  mac_t my_mac;
  ip_addr_t my_ip;

  param_get_cur_mac(my_mac);
  param_get_ip_addr(my_ip);

  u16 off = eth_make_bcast(pkt_buf, my_mac);
  eth_set_pkt_type(pkt_buf, ETH_TYPE_ARP);
  u16 size = arp_make_reply(pkt_buf + off, my_mac, my_ip);

  u16 total_size = size + off;

  uart_send_time_stamp_spc();
  uart_send_pstring(PSTR("nic_test_tx:res="));
  u08 res = nic_tx_data(pkt_buf, total_size);
  uart_send_hex_byte(res);
  uart_send_crlf();

  if(res == NIC_OK) {
    dump_pkt(pkt_buf, total_size);
    uart_send_crlf();
  }
}

void nic_test_rx(void)
{
  uart_send_time_stamp_spc();
  uart_send_pstring(PSTR("nic_test_rx:"));
  u08 num = nic_rx_num_pending();
  if(num == 0) {
    uart_send_pstring(PSTR("NONE!"));
    uart_send_crlf();
  } else {
    u16 size = 0;
    u08 res = nic_rx_size(&size);
    uart_send_pstring(PSTR("res="));
    uart_send_hex_byte(res);
    uart_send_pstring(PSTR(",size="));
    uart_send_hex_word(size);
    if(size > 0) {
      res = nic_rx_data(pkt_buf, size);
      uart_send_pstring(PSTR(",res="));
      uart_send_hex_byte(res);
      if(res == NIC_OK) {
        uart_send_crlf();
        dump_pkt(pkt_buf, size);
      }
    }
    uart_send_crlf();
  }
}

void nic_test_toggle_duplex(void)
{
  uart_send_time_stamp_spc();
  uart_send_pstring(PSTR("nic_test_toggle_duplex:"));
  u16 caps = param_get_nic_caps();
  if(caps & NIC_CAP_FULL_DUPLEX) {
    caps &= NIC_CAP_FULL_DUPLEX;
    uart_send_pstring(PSTR("OFF"));
  } else {
    caps |= NIC_CAP_FULL_DUPLEX;
    uart_send_pstring(PSTR("ON"));
  }
  uart_send_crlf();
}

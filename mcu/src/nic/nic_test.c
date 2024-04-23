#include "arch.h"
#include "types.h"

#ifdef DEBUG_NIC
#define DEBUG
#endif

#include "hw_spi.h"
#include "debug.h"
#include "nic.h"
#include "nic_test.h"
#include "nic_mod.h"
#include "uartutil.h"
#include "param.h"
#include "arp.h"
#include "eth.h"
#include "param.h"
#include "net_dump.h"

void nic_test_status(void)
{
  uart_send_time_stamp_spc();
  uart_send_pstring(PSTR("nic_test_status:num="));
  u08 num_pkt = nic_rx_num_pending();
  uart_send_hex_byte(num_pkt);

  u16 link_status = 0;
  u08 ok = nic_ioctl(NIC_IOCTL_GET_LINK_STATUS, &link_status);
  uart_send_pstring(PSTR(",link="));
  if(ok == NIC_OK) {
    uart_send_hex_word(link_status);
  } else {
    uart_send_pstring(PSTR("err="));
    uart_send_hex_byte(ok);
  }

  uart_send_crlf();

  nic_status();
}

void nic_test_tx(void)
{
  mac_t my_mac;
  ip_addr_t my_ip;
  ip_addr_t peer_ip;

  param_get_cur_mac(my_mac);
  param_get_ip_addr(my_ip);
  param_get_peer_addr(peer_ip);

  u16 size = ARP_SIZE + ETH_HDR_SIZE;
  u08 *buf = nic_tx_begin(size);

  eth_make_bcast(buf, my_mac, ETH_TYPE_ARP);
  arp_make_request(buf + ETH_HDR_SIZE, my_mac, my_ip, peer_ip);

  net_dump_pkt(buf, size);

  uart_send_time_stamp_spc();
  uart_send_pstring(PSTR("nic_test_tx:res="));
  u08 res = nic_tx_end(size);
  uart_send_hex_byte(res);
  uart_send_crlf();
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
      const u08 *buf = nic_rx_begin(size);
      uart_send_pstring(PSTR(",res="));
      uart_send_hex_byte(res);
      if(res == NIC_OK) {
        uart_send_crlf();
        net_dump_pkt(buf, size);
      }
      nic_rx_end(size);
    }
    uart_send_crlf();
  }
}

void nic_test_toggle_duplex(void)
{
  uart_send_time_stamp_spc();
  uart_send_pstring(PSTR("duplex:"));
  u16 opts = param_get_nic_opts();
  if(opts & NIC_OPT_FULL_DUPLEX) {
    opts &= NIC_OPT_FULL_DUPLEX;
    uart_send_pstring(PSTR("OFF"));
  } else {
    opts |= NIC_OPT_FULL_DUPLEX;
    uart_send_pstring(PSTR("ON"));
  }
  param_set_nic_opts(opts);
  uart_send_crlf();
}

void nic_test_toggle_nic(void)
{
  uart_send_time_stamp_spc();
  uart_send_pstring(PSTR("nic:"));
  u08 nic = param_get_nic();
  nic++;
  if(nic >= nic_mod_get_num_nics()) {
    nic=0;
  }
  param_set_nic(nic);
  uart_send_hex_byte(nic);
  uart_send_crlf();
}

void nic_test_toggle_port(void)
{
  uart_send_time_stamp_spc();
  uart_send_pstring(PSTR("port:"));
  u08 port = param_get_nic_port();
  port++;
  if(port >= HW_SPI_NUM_CS) {
    port=0;
  }
  param_set_nic_port(port);
  uart_send_hex_byte(port);
  uart_send_crlf();
}

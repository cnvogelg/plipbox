/*
 * pio_test.c - test packet I/O
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

#include "pio_test.h"

#include "timer.h"
#include "pio.h"
#include "param.h"
#include "uartutil.h"
#include "pkt_buf.h"

#include "net/net.h"
#include "net/eth.h"
#include "net/arp.h"

void pio_test_begin(void)
{
  uart_send_time_stamp_spc();
  uart_send_pstring(PSTR("[PIO_TEST] on\r\n"));

  pio_init(param.mac_addr, PIO_INIT_BROAD_CAST);
}

void pio_test_end(void)
{
  pio_exit();

  uart_send_time_stamp_spc();
  uart_send_pstring(PSTR("[PIO_TEST] off\r\n"));
}

static u08 recv_packet(u16 *size)
{
  // measure packet receive
  timer_hw_reset();
  u08 result = pio_recv(pkt_buf, PKT_BUF_SIZE, size);
  u16 delta = timer_hw_get();

  uart_send_time_stamp_spc();
  uart_send_pstring(PSTR("recv: "));
  if(result == PIO_OK) {
    // speed
    uart_send_pstring(PSTR("v="));
    u16 rate = timer_hw_calc_rate_kbs(*size, delta);
    uart_send_rate_kbs(rate);

    // size
    uart_send_pstring(PSTR(" n="));
    uart_send_hex_word(*size);
    uart_send_crlf();
  } else {
    uart_send_pstring(PSTR("ERROR="));
    uart_send_hex_byte(result);
    uart_send_crlf();
  }
  return result;
}

static u08 send_packet(u16 size)
{
  timer_hw_reset();
  u08 result = pio_send(pkt_buf, size);
  u16 delta = timer_hw_get();

  uart_send_time_stamp_spc();
  uart_send_pstring(PSTR("send: "));
  if(result == PIO_OK) {
    // speed
    uart_send_pstring(PSTR("v="));
    u16 rate = timer_hw_calc_rate_kbs(size, delta);
    uart_send_rate_kbs(rate);

    // size
    uart_send_pstring(PSTR(" n="));
    uart_send_hex_word(size);
    uart_send_crlf();
  } else {
    uart_send_pstring(PSTR("ERROR="));
    uart_send_hex_byte(result);
    uart_send_crlf();
  }
  return result;
}

static void handle_arp(u08 *buf, u16 size)
{
  // is an ARP request
  if(arp_is_ipv4(buf, size) && (arp_get_op(buf) == ARP_REQUEST)) {
    // is our IP?
    const u08 *tgt_ip = arp_get_tgt_ip(buf);

    uart_send_time_stamp_spc();
    uart_send_pstring(PSTR("ARP REQ: IP="));
    net_dump_ip(tgt_ip);
    uart_send_crlf();

    if(net_compare_ip(tgt_ip, param.test_ip)) {
      arp_make_reply(buf, param.mac_addr, param.test_ip);
      eth_make_bcast(pkt_buf, param.mac_addr);
      send_packet(size + ETH_HDR_SIZE);

      uart_send_time_stamp_spc();
      uart_send_pstring(PSTR("ARP REPLY!\r\n"));
    }
  }
}

void pio_test_worker(void)
{
  // incoming packet?
  if(pio_has_recv()) {
    u16 size;
    if(recv_packet(&size) == PIO_OK) {
      // large enough?
      if(size > ETH_HDR_SIZE) {
        // is a arp request?
        u16 type = eth_get_pkt_type(pkt_buf);
        if(type == ETH_TYPE_ARP) {
          handle_arp(pkt_buf+ETH_HDR_SIZE, size-ETH_HDR_SIZE);
        }
      }
    }
  }
}

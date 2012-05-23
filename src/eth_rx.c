/*
 * eth_rx.c: handle eth receiption and plip tx
 *
 * Written by
 *  Christian Vogelgsang <chris@vogelgsang.org>
 *
 * This file is part of plip2slip.
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

#include <avr/delay.h>

#include "eth_rx.h"

#include "enc28j60.h"
#include "icmp.h"
#include "arp.h"
#include "eth.h"
#include "net.h"
#include "pkt_buf.h"
#include "uartutil.h"
#include "uart.h"

const u08 mac[6] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };
const u08 ip[4] = { 192, 168, 2, 133 };
const u08 gw[4] = { 192, 168, 2, 1 };
const u08 nm[4] = { 255, 255, 255, 0 };

void eth_rx_init(void)
{
  /* init ethernet controller */
  u08 rev = enc28j60_init(mac);
  uart_send_hex_byte_crlf(rev);

  /* setup network addressing */
  net_init(mac, ip, gw, nm);
  
  /* wait for link up */
  for(int i = 0 ; i<10;i++) {
    if(enc28j60_is_link_up()) {
      break;
    }
    _delay_ms(250);
    uart_send('.');
  }
  uart_send_crlf();
  
  /* init ARP: send query for GW MAC */
  arp_cache_init();
  arp_init(pkt_buf, PKT_BUF_SIZE);   
}

void eth_rx_worker(void)
{
  // get next packet
  u16 len = enc28j60_packet_receive(pkt_buf, PKT_BUF_SIZE);
  if(len == 0) {
    return;
  }

#define DUMP_ETH_HDR
#ifdef DUMP_ETH_HDR
  // show length and dump eth header
  uart_send_hex_word_spc(len);
  eth_dump(pkt_buf);
  uart_send_crlf();
#endif

  // handle ARP packets
  if(!arp_handle_packet(pkt_buf,len)) {
    // IPv4
    if(eth_is_ipv4_pkt(pkt_buf)) {
      u08 *ip_buf = pkt_buf + ETH_HDR_SIZE;
      //u16 ip_size = len - ETH_HDR_SIZE;
      // reply ping
      if(icmp_is_ping_request(ip_buf)) {
        uart_send_string("PING!");
        uart_send_crlf();
            
        icmp_ping_request_to_reply(ip_buf);
        eth_make_reply(pkt_buf);
        enc28j60_packet_send(pkt_buf, len);
      }
          
    }
  }
}

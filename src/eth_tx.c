/*
 * eth_tx.c: handle eth packet sends
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

#include "global.h"
   
#include "net.h"
#include "eth.h"
#include "ip.h"
#include "arp.h"
#include "icmp.h"
#include "pkt_buf.h"
#include "enc28j60.h"
  
#include "uart.h"
#include "uartutil.h"
   
#define DEBUG_ETH_TX   

u08 eth_tx_send_ping_request(const u08 *ip)
{
#ifdef DEBUG_ETH_TX
  uart_send_string("send ping: ");
  net_dump_ip(ip);
#endif

  const u08 *mac = arp_find_mac(pkt_buf, ip);
  if(mac == 0) {
#ifdef DEBUG_ETH_TX
    uart_send_string("no mac!");
    uart_send_crlf();
#endif
    return 0;
  }
  
#ifdef DEBUG_ETH_TX
  uart_send_string(" -> ");
  net_dump_mac(mac);
  uart_send_crlf();
#endif
  
  eth_make_to_tgt(pkt_buf, ETH_TYPE_IPV4, mac);
  u08 *ip_pkt = pkt_buf + ETH_HDR_SIZE;
  u16 size = icmp_make_ping_request(ip_pkt, ip, 0, 0);
  enc28j60_packet_send(pkt_buf, size + ETH_HDR_SIZE);
  
  return 1;
}

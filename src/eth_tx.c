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

#include "plip.h"

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
  enc28j60_packet_tx(pkt_buf, size + ETH_HDR_SIZE);
  
  return 1;
}

static u08 offset;
static u08 size;
static u08 valid;

static u08 begin_rx(plip_packet_t *pkt)
{
  // start writing packet after ETH header
  offset = ETH_HDR_SIZE;
  size = 0;
  valid = 0;
  
  // start writing after ETH header
  enc28j60_packet_tx_prepare();
  enc28j60_packet_tx_begin_range(ETH_HDR_SIZE);
  
  return PLIP_STATUS_OK;
}

static u08 transfer_rx(u08 *data)
{
  // first we need to fill our buffer until IP header is complete
  if(size < IP_MIN_HDR_SIZE) {
    pkt_buf[offset] = *data;
    offset ++;
    size ++;
  }
  
  // always copy to TX buffer of enc28j60
  enc28j60_packet_tx_byte(*data);
  
  return PLIP_STATUS_OK;
}

static u08 end_rx(plip_packet_t *pkt)
{
  // end range
  enc28j60_packet_tx_end_range();

  uart_send_string("ip: ");
  const u08 *ip = ip_get_tgt_ip(pkt_buf + ETH_HDR_SIZE);
  net_dump_ip(ip);
  const u08 *mac = arp_find_mac(pkt_buf, ip);
  if(mac == 0) {
    uart_send_string("??");
    uart_send_crlf();
    return PLIP_STATUS_OK;
  }
  uart_send_string(" -> mac ");
  net_dump_mac(mac);
  uart_send_crlf();
  
  // now build ethernet header
  eth_make_to_tgt(pkt_buf, ETH_TYPE_IPV4, mac);
  
  // copy eth header to packet buffer
  enc28j60_packet_tx_begin_range(0);
  enc28j60_packet_tx_blk(pkt_buf, ETH_HDR_SIZE);
  enc28j60_packet_tx_end_range();
  
  // finally send packet
  u16 pkt_size = pkt->size + ETH_HDR_SIZE;
  enc28j60_packet_tx_send(pkt_size);
  
  return PLIP_STATUS_OK;
}

void eth_tx_init(void)
{
  plip_recv_init(begin_rx, transfer_rx, end_rx);
}

void eth_tx_worker(void)
{
  // do we have a PLIP packet waiting?
  if(plip_can_recv() == PLIP_STATUS_OK) {
    u08 status = plip_recv(&pkt);
    
    uart_send('T');
    uart_send_hex_byte_crlf(status);
  }
}



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

static void handle_my_packet(u08 *ip_buf, u16 size)
{
  // TODO
}

static u08 offset;
static u08 size;

static u08 begin_rx(plip_packet_t *pkt)
{
  // start writing packet after (potential) ETH header
  offset = ETH_HDR_SIZE;
  size = 0;
  
  // start writing after ETH header
  enc28j60_packet_tx_begin_range(ETH_HDR_SIZE);
  
  return PLIP_STATUS_OK;
}

static u08 transfer_rx(u08 *data)
{
  // clone packet to our packet buffer
  if(size < (PKT_BUF_SIZE - ETH_HDR_SIZE)) {
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
  // end range in enc28j60 buffer
  enc28j60_packet_tx_end_range();

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
    
    // prepare eth chip to get its tx buffer filled
    enc28j60_packet_tx_prepare();
    
    // receive PLIP packet and store it in eth chip tx buffer
    // also keep a copy in our local pkt_buf (up to max size)
    u08 status = plip_recv(&pkt);
    
    // if PLIP rx was ok then have a look at the packet
    if(status == PLIP_STATUS_OK) {
      
      // fetch tgt ip of incoming packet
      u08 *ip_buf = pkt_buf + ETH_HDR_SIZE;
      const u08 *tgt_ip = ip_get_tgt_ip(ip_buf);
      u08 ltgt_ip[4];
      net_copy_ip(tgt_ip, ltgt_ip);
  
      // if its a packet for me (p2p or eth address) then handle it now
      if(net_compare_ip(ltgt_ip, net_get_p2p_me()) ||
         net_compare_ip(ltgt_ip, net_get_ip())) {
        uart_send_string("plip_rx: ME!");
        uart_send_crlf();
        handle_my_packet(ip_buf, pkt.size);
      } 
      // pass this packet on to ethernet
      else {
        uart_send_string("plip_rx:");
        net_dump_ip(ltgt_ip);
        
        // find a mac address for the target 
        const u08 *mac = arp_find_mac(pkt_buf, ltgt_ip);
        if(mac == 0) {
          uart_send_string("??");
          uart_send_crlf();
        }
        // found mac -> we can send packet
        else {
          uart_send_string(" -> mac ");
          net_dump_mac(mac);
          uart_send_crlf();
        
          // adjust source ip to our eth IP
          ip_set_src_ip(ip_buf, net_get_ip());
          ip_hdr_set_checksum(ip_buf);
        
          // now build ethernet header
          eth_make_to_tgt(pkt_buf, ETH_TYPE_IPV4, mac);
        
          // copy eth header and ip header to packet buffer
          enc28j60_packet_tx_begin_range(0);
          enc28j60_packet_tx_blk(pkt_buf, ETH_HDR_SIZE + IP_MIN_HDR_SIZE);
          enc28j60_packet_tx_end_range();
  
          // finally send packet
          u16 pkt_size = pkt.size + ETH_HDR_SIZE;
          enc28j60_packet_tx_send(pkt_size);
        }
      }
    }
  }
}



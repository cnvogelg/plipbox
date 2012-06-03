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
#include "ping.h"

#include "uart.h"
#include "uartutil.h"

static void handle_my_packet(u08 *ip_buf, u16 size)
{
  // TODO
}

static u08 offset;

static u08 begin_rx(plip_packet_t *pkt)
{
  // start writing packet after (potential) ETH header
  offset = ETH_HDR_SIZE;
  
  // start writing after ETH header
  enc28j60_packet_tx_begin_range(ETH_HDR_SIZE);
  
  return PLIP_STATUS_OK;
}

static u08 transfer_rx(u08 *data)
{
  // clone packet to our packet buffer
  if(offset < PKT_BUF_SIZE) {
    pkt_buf[offset] = *data;
    offset ++;
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

void plip_rx_init(void)
{
  plip_recv_init(begin_rx, transfer_rx, end_rx);
}

static void send_prefix(void)
{
  uart_send_pstring(PSTR("plip(rx): "));
}

//#define DEBUG

void plip_rx_worker(u08 plip_state, u08 eth_online)
{
  // do we have a PLIP packet waiting?
  if(plip_can_recv() == PLIP_STATUS_OK) {
    
    // prepare eth chip to get its tx buffer filled
    enc28j60_packet_tx_prepare();
    
    // receive PLIP packet and store it in eth chip tx buffer
    // also keep a copy in our local pkt_buf (up to max size)
    u08 status = plip_recv(&pkt);
    
    // if PLIP rx was ok then have a look at the packet
    if(status != PLIP_STATUS_OK) {
      send_prefix();
      uart_send_hex_byte_crlf(status);
    } else {
      // fetch tgt ip of incoming packet
      u08 *ip_buf = pkt_buf + ETH_HDR_SIZE;
      const u08 *tgt_ip = ip_get_tgt_ip(ip_buf);
      u08 ltgt_ip[4];
      net_copy_ip(tgt_ip, ltgt_ip);
      const u08 *src_ip = ip_get_src_ip(ip_buf);
      u16 ip_size = pkt.size;
  
      // make sure all packets are coming from amiga
      if(!net_compare_ip(src_ip, net_get_p2p_amiga())) {
        send_prefix();
        uart_send_pstring(PSTR("not amiga!\r\n"));
        return;
      }
  
      // if its a packet for me (plip address)
      if(net_compare_ip(ltgt_ip, net_get_p2p_me())) {
        if(ip_is_ipv4_protocol(ip_buf, IP_PROTOCOL_ICMP)) {
          ping_plip_handle_packet(ip_buf, ip_size);
        } else {
          send_prefix();
          uart_send_pstring(PSTR("ME (plip)!\r\n"));        
          handle_my_packet(ip_buf, ip_size);
        }
      }
      // its a packet for me (eth address)
      else if(net_compare_ip(ltgt_ip, net_get_ip())) {
        if(eth_online) {
          // ping if eth link is up
          if(ip_is_ipv4_protocol(ip_buf, IP_PROTOCOL_ICMP)) {
            ping_plip_handle_packet(ip_buf, ip_size);
          } 
        }
      } 
      // pass this packet on to ethernet
      else {
#ifdef DEBUG
        send_prefix();
        net_dump_ip(ltgt_ip);
        uart_send_pstring(PSTR(" size="));
        uart_send_hex_word_spc(ip_size);
#endif
        
        // find a mac address for the target 
        const u08 *mac = arp_find_mac(pkt_buf, ltgt_ip);
        if(mac == 0) {
#ifdef DEBUG
          uart_send_pstring(PSTR("no mac\r\n"));
#endif
        }
        // found mac -> we can send packet
        else {
#ifdef DEBUG
          uart_send_pstring(PSTR(" -> mac "));
          net_dump_mac(mac);
          uart_send_crlf();
#endif
        
          // ----- NAT -----
          // adjust source ip to our eth IP
          ip_adjust_checksum(ip_buf, IP_CHECKSUM_OFF, net_get_p2p_amiga(), net_get_ip());
          
          // get protocol
          u08 protocol = ip_get_protocol(ip_buf);
          u16 copy_size = ETH_HDR_SIZE + IP_MIN_HDR_SIZE;
          if(protocol == IP_PROTOCOL_TCP) {
            // adjust TCP checksum
            u16 off = ip_get_hdr_length(ip_buf) + TCP_CHECKSUM_OFF;
            ip_adjust_checksum(ip_buf, off, net_get_p2p_amiga(), net_get_ip());
            copy_size += TCP_CHECKSUM_OFF + 2;
          } else if(protocol == IP_PROTOCOL_UDP) {
            // adjust UDP checksum
            u16 off = ip_get_hdr_length(ip_buf) + UDP_CHECKSUM_OFF;
            ip_adjust_checksum(ip_buf, off, net_get_p2p_amiga(), net_get_ip());
            copy_size += UDP_CHECKSUM_OFF + 2;
          }

          // change src ip to my eth ip
          ip_set_src_ip(ip_buf, net_get_ip());
          
#ifdef DEBUG
          // make sure our checksum was corrected correctly
          u16 chk = ip_hdr_calc_checksum(ip_buf);
          if(chk != 0xffff) {
            send_prefix(),
            uart_send_pstring(PSTR("CHECKSUM?"));
            uart_send_hex_word_crlf(chk);
          }
#endif
        
          // now build ethernet header
          eth_make_to_tgt(pkt_buf, ETH_TYPE_IPV4, mac);
        
          // copy (created) eth header and (modified) ip header back to packet buffer
          enc28j60_packet_tx_begin_range(0);
          enc28j60_packet_tx_blk(pkt_buf, copy_size);
          enc28j60_packet_tx_end_range();
  
          // finally send packet
          u16 pkt_size = ip_size + ETH_HDR_SIZE;
          if(eth_online) {
            enc28j60_packet_tx_send(pkt_size);
          } else {
            // can't actually send packet because eth is not online :(
            send_prefix();
            uart_send_pstring(PSTR("DROP"));
          }
        }
      }
    }
  }
}



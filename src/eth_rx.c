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

#include "net/icmp.h"
#include "net/arp.h"
#include "net/eth.h"
#include "net/net.h"
#include "net/ip.h"
#include "net/arp_cache.h"
#include "net/dhcp_client.h"
#include "net/udp.h"

#include "enc28j60.h"
#include "pkt_buf.h"
#include "uartutil.h"
#include "uart.h"
#include "ping.h"
#include "plip_tx.h"
#include "timer.h"
#include "eth_state.h"

static u16 my_timer;

static u08 helper_time_passed(void)
{
  if((my_timer ^ timer_10ms) & HELPER_WORKER_TIMER_MASK) {
    my_timer = timer_10ms;
    return 1;
  }
  return 0;
}

void eth_rx_init(void)
{
  // prepare cache
  arp_cache_init();
}

static void send_prefix(void)
{
  uart_send_pstring(PSTR("eth(rx): "));
}

u08 retry = 0;

void eth_rx_worker(u08 eth_state, u08 plip_online)
{
  if(eth_state == ETH_STATE_OFFLINE) {
    return;
  }
    
  // get next packet
  u16 len = enc28j60_packet_rx_begin();
  if(len == 0) {
    // shall we retry to send the last packet
    if(plip_online && (retry > 0)) {
      u08 status = plip_send(&pkt);
      if(status == PLIP_STATUS_OK) {
        uart_send_pstring(PSTR("plip(tx): retry ok"));
        uart_send_crlf();
        retry = 0;
      } else {
        uart_send_pstring(PSTR("plip(tx): retry #"));
        uart_send_hex_byte_crlf(status);
        retry --;
      }
    }  
    
    // trigger helper worker for ARP and DHCP
    if(helper_time_passed()) {
      // ARP
      arp_cache_worker(pkt_buf, enc28j60_packet_tx);
      
      // DHCP
      if(net_dhcp_enabled()) {
        dhcp_client_worker(pkt_buf, PKT_BUF_SIZE, enc28j60_packet_tx);
      }
    }
    return;
  }

  // first read only the ethernet header into our buffer
  enc28j60_packet_rx_blk(pkt_buf, ETH_HDR_SIZE);
  
  // ---- early packet filtering ----
  // packet is only interesting if its an ARP broadcast or ARP/IPv4 to our MAC
  u08 is_tgt_me = eth_is_tgt_me(pkt_buf);
  u08 is_broadcast = 0;
  if(!is_tgt_me) {
    is_broadcast = eth_is_broadcast_tgt(pkt_buf);
  }
  u08 is_ipv4 = eth_is_ipv4_pkt(pkt_buf);
  u08 is_arp = 0;
  if(!is_ipv4) {
    is_arp = eth_is_arp_pkt(pkt_buf);
  }
  u08 keep = (is_tgt_me && (is_arp || is_ipv4)) || (is_broadcast && is_arp);
  if(!keep) {
    // packet rejected
    enc28j60_packet_rx_end();
    return;
  }

  // ---- packet accepted ----
  // now read missing bytes (up to pkt buffer size)
  u16 read_len = len;
  if(read_len > PKT_BUF_SIZE) {
    read_len = PKT_BUF_SIZE;
  }
  u16 missing = read_len - ETH_HDR_SIZE;
  u16 offset = ETH_HDR_SIZE;

  //#define DUMP_ETH_HDR
#ifdef DUMP_ETH_HDR
  // show length and dump eth header
  uart_send_hex_word_spc(len);
  eth_dump(pkt_buf);
  uart_send_crlf();
#endif

  // ----- handle ARP -----
  if(is_arp) {
    // read missing bytes and finish packet rx
    enc28j60_packet_rx_blk(pkt_buf + offset, missing);
    enc28j60_packet_rx_end();

    // now do ARP stuff
    arp_cache_handle_packet(pkt_buf, len, enc28j60_packet_tx);
  }
  // ----- handle IPv4 -----
  else if(missing >= IP_MIN_HDR_SIZE) {
    // read IP header
    enc28j60_packet_rx_blk(pkt_buf + offset, IP_MIN_HDR_SIZE);
    offset  += IP_MIN_HDR_SIZE;
    missing -= IP_MIN_HDR_SIZE;
    
    // prepare IP packet pointer/size
    u08 *ip_buf = pkt_buf + ETH_HDR_SIZE;
    u16 ip_size = len - ETH_HDR_SIZE;
    u08 finished = 0;
      
    // check IP packet size
    u16 total_len = ip_get_total_length(ip_buf);
    if(ip_size < total_len) {
      send_prefix();
      uart_send_pstring(PSTR("IP PKT SIZE? "));
      uart_send_hex_word_spc(ip_size);
      uart_send_hex_word_crlf(total_len);      
    }
    // check IP header checksum
    else if(!ip_hdr_validate_checksum(ip_buf)) {
      send_prefix();
      uart_send_pstring(PSTR("IP HDR CHECK?\r\n"));
    }
    // check tgt IP
    else if(!net_compare_my_ip(ip_get_tgt_ip(ip_buf))) {
      // UDP? -> could be BOOTP/DHCP
      u08 handled = 0;
      if(ip_is_ipv4_protocol(ip_buf, IP_PROTOCOL_UDP)) {
        // read missing bytes and finish packet rx
        enc28j60_packet_rx_blk(pkt_buf + offset, missing);
        enc28j60_packet_rx_end();
        
        // handle DHCP packet
        if(dhcp_is_dhcp_pkt(ip_buf)) {
          handled = dhcp_client_handle_packet(pkt_buf, len, enc28j60_packet_tx);
        }
      }
      // warn on unhandled packets
      if(!handled) {
        send_prefix(),
        uart_send_pstring(PSTR("TGT NOT ME? "));
        net_dump_ip(ip_get_tgt_ip(ip_buf));
        uart_send_crlf();
      }
    }
    // IP packet seems to be ok!
    else {
      u08 protocol = ip_get_protocol(ip_buf);
      u08 hdr_len = ip_get_hdr_length(ip_buf);

      // do we need some extra bytes for UDP/TCP checksum correction?
      u08 extra_off = 0;
      if(protocol == IP_PROTOCOL_TCP) {
        extra_off = TCP_CHECKSUM_OFF + hdr_len;
#ifdef DEBUG
        uart_send('T');
        uart_send_hex_byte_spc(extra);
#endif
      }
      else if(protocol == IP_PROTOCOL_UDP) {
        extra_off = UDP_CHECKSUM_OFF + hdr_len;
#ifdef DEBUG
        uart_send('U');
        uart_send_hex_byte_spc(extra);
#endif  
      }
      
      // size of packet we send from pkt_buf
      if(extra_off != 0) {
        u08 extra = extra_off + 2 - IP_MIN_HDR_SIZE;
        enc28j60_packet_rx_blk(pkt_buf + offset, extra);
        missing -= extra;
        offset += extra;
      }

      // is DHCP for me?
      if(bootp_is_bootp_pkt(ip_buf)) {
        // read missing bytes and finish as icmp might send a return
        enc28j60_packet_rx_blk(pkt_buf + offset, missing);
        enc28j60_packet_rx_end();
        finished = 1;
        
        // handle DHCP packet
        if(dhcp_is_dhcp_pkt(ip_buf)) {
          dhcp_client_handle_packet(pkt_buf, len, enc28j60_packet_tx);
        } else {
          send_prefix();
          uart_send_pstring(PSTR("BOOTP for ME?\r\n"));
        }
      }
      // if plip is not online then handle pings and drop other packets
      else if(!plip_online) {
        if(ip_is_ipv4_protocol(ip_buf, IP_PROTOCOL_ICMP)) {
          // read missing bytes and finish as icmp might send a return
          enc28j60_packet_rx_blk(pkt_buf + offset, missing);
          enc28j60_packet_rx_end();
          finished = 1;
        
          // my custom PING handler
          ping_eth_handle_packet(ip_buf, ip_size);
        } else {
          // drop packet
          send_prefix();
          uart_send_pstring(PSTR("DROP\r\n"));
        }
      } 
      // plip is online -> do PLIP transfer of packet
      else {
        // adjust ip: eth ip -> p2p amiga ip
        ip_adjust_checksum(ip_buf, IP_CHECKSUM_OFF, net_get_ip(), net_get_p2p_amiga());
        if(extra_off != 0) {
          ip_adjust_checksum(ip_buf, extra_off, net_get_ip(), net_get_p2p_amiga());
        }
        ip_set_tgt_ip(ip_buf, net_get_p2p_amiga());

#ifdef DEBUG
        // make sure our checksum was corrected correctly
        u16 chk = ip_hdr_calc_checksum(ip_buf);
        if(chk != 0xffff) {
          send_prefix();
          uart_send_pstring(PSTR("CHECK?"));
          uart_send_hex_word_crlf(chk);          
        }
#endif

        // send packet via PLIP
        u08 status = plip_tx_send(ETH_HDR_SIZE, offset, ip_size);
        if(status != PLIP_STATUS_OK) {
          uart_send_pstring(PSTR("plip(tx): "));
          uart_send_hex_byte_crlf(status);
          retry = 5;
        } else {
          retry = 0;
        }
        
#ifdef DEBUG
        uart_send_pstring(PSTR("plip_tx:"));
        net_dump_ip(ip_get_src_ip(ip_buf));
        uart_send_pstring(PSTR(" -> "));
        net_dump_ip(ip_get_tgt_ip(ip_buf));
        uart_send_pstring(PSTR(" : "));
        uart_send_hex_byte_crlf(status);
#endif
      }
    }
    // finish IP packet transfer
    if(!finished) {
      enc28j60_packet_rx_end();
    }
  } else {
    send_prefix();
    uart_send_pstring(PSTR("SHORT PKT?\r\n"));
  }
}

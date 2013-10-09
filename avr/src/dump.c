/*
 * dump.c - helper functions for debugging
 *
 * Written by
 *  Christian Vogelgsang <chris@vogelgsang.org>
 *
 * This file is part of pbprotobox.
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

#include "dump.h"
#include "uart.h"
#include "uartutil.h"
#include "net/net.h"
#include "net/arp.h"
#include "net/eth.h"
#include "net/ip.h"
#include "net/udp.h"
#include "net/tcp.h"
#include "param.h"
#include "util.h"
#include "pb_proto.h"

void dump_eth_pkt(const u08 *eth_buf, u16 size)
{
  u08 buf[4];
  
  uart_send('[');
  dword_to_dec(size, buf, 4, 4);
  uart_send_data(buf,4);
  uart_send(',');
  uart_send_hex_word(eth_get_pkt_type(eth_buf));
  uart_send(',');
  net_dump_mac(eth_get_src_mac(eth_buf));
  uart_send('>');
  net_dump_mac(eth_get_tgt_mac(eth_buf));
  uart_send(']');
  uart_send(' ');
}

void dump_arp_pkt(const u08 *arp_buf)
{
  uart_send_pstring(PSTR("[ARP:"));

  // ARP op
  u16 op = arp_get_op(arp_buf);
  if(op == ARP_REQUEST) {
    uart_send_pstring(PSTR("REQ "));
  } else if(op == ARP_REPLY) {
    uart_send_pstring(PSTR("REPL"));
  } else {
    uart_send_hex_word(op);
  }
  uart_send(',');
  
  // src pair
  uart_send('(');
  net_dump_mac(arp_get_src_mac(arp_buf));
  uart_send(',');
  net_dump_ip(arp_get_src_ip(arp_buf));
  uart_send(')');
  uart_send('>');

  // tgt pair
  uart_send('(');
  net_dump_mac(arp_get_tgt_mac(arp_buf));
  uart_send(',');
  net_dump_ip(arp_get_tgt_ip(arp_buf));
  uart_send(')');
  
  uart_send(']');
  uart_send(' ');
}

void dump_ip_pkt(const u08 *ip_buf)
{
  uart_send_pstring(PSTR("[IP4:"));

  // size
  uart_send_hex_word(ip_get_total_length(ip_buf));

  // ip proto
  u08 proto = ip_get_protocol(ip_buf);
  if(proto == IP_PROTOCOL_ICMP) {
    uart_send_pstring(PSTR(",ICMP"));
  } else if(proto == IP_PROTOCOL_TCP) {
    uart_send_pstring(PSTR(",TCP "));
  } else if(proto == IP_PROTOCOL_UDP) {
    uart_send_pstring(PSTR(",UDP "));
  } else {
    uart_send(',');
    uart_send_hex_word(proto);
  }

  // src/tgt ip
  uart_send(',');
  net_dump_ip(ip_get_src_ip(ip_buf));
  uart_send('>');
  net_dump_ip(ip_get_tgt_ip(ip_buf)),

  uart_send(']');
  uart_send(' ');
}

static void dump_udp_port(u16 port)
{
  if(port == 67) {
    uart_send_pstring(PSTR("BOOTPS"));
  } else if(port == 68) {
    uart_send_pstring(PSTR("BOOTPC"));
  } else {
    uart_send(' ');
    uart_send_hex_word(port);
    uart_send(' ');
  }
}

static void dump_tcp_port(u16 port)
{
  if(port == 21) {
    uart_send_pstring(PSTR("FTPctl"));
  } else if(port == 20) {
    uart_send_pstring(PSTR("FTPdat"));
  } else {
    uart_send(' ');
    uart_send_hex_word(port);
    uart_send(' ');
  }
}

extern void dump_ip_protocol(const u08 *ip_buf)
{
  const u08 *proto_buf = ip_buf + ip_get_hdr_length(ip_buf);
  u08 proto = ip_get_protocol(ip_buf);
  if(proto == IP_PROTOCOL_UDP) {
    uart_send_pstring(PSTR("[UDP:"));
    u16 src_port = udp_get_src_port(proto_buf);
    u16 tgt_port = udp_get_tgt_port(proto_buf);
    dump_udp_port(src_port);
    uart_send('>');
    dump_udp_port(tgt_port);
    uart_send(']');
    uart_send(' ');
  }
  else if(proto == IP_PROTOCOL_TCP) {
    uart_send_pstring(PSTR("[TCP:"));
    u16 src_port = tcp_get_src_port(proto_buf);
    u16 tgt_port = tcp_get_tgt_port(proto_buf);
    dump_tcp_port(src_port);
    uart_send('>');
    dump_tcp_port(tgt_port);

    u16 flags = tcp_get_flags(proto_buf);
    uart_send_pstring(PSTR(",flags="));
    uart_send_hex_word(flags);

    uart_send_pstring(PSTR(",seq="));
    u32 seq = tcp_get_seq_num(proto_buf);
    uart_send_hex_dword(seq);
    
    if(flags & TCP_FLAGS_ACK) {
      u32 ack = tcp_get_ack_num(proto_buf);
      uart_send_pstring(PSTR(",ack="));
      uart_send_hex_dword(ack);
    }
    
    uart_send(']');
    uart_send(' ');
  }
}

extern void dump_pb_proto(void)
{
  u32 d;
  u08 buf[8];

  uart_send('{');

  d = pb_proto_timestamps.enter - pb_proto_timestamps.can_enter;
  dword_to_dec(d,buf,5,4);
  uart_send_data(buf,6);

  uart_send(',');

  d = pb_proto_timestamps.data_begin - pb_proto_timestamps.enter;
  dword_to_dec(d,buf,5,4);
  uart_send_data(buf,6);

  uart_send(',');
  
  d = pb_proto_timestamps.data_end - pb_proto_timestamps.data_begin;
  dword_to_dec(d,buf,5,4);
  uart_send_data(buf,6);
  
  uart_send(',');
  
  d = pb_proto_timestamps.leave - pb_proto_timestamps.data_end;
  dword_to_dec(d,buf,5,4);
  uart_send_data(buf,6);
  
  uart_send('}');
  uart_send(' ');
}

dump_latency_t dump_latency_data; 

extern void dump_latency(void)
{
  u32 d;
  u08 buf[8];
  
  uart_send('<');
  
  d = dump_latency_data.rx_leave - dump_latency_data.rx_enter;
  dword_to_dec(d,buf,5,4);
  uart_send_data(buf,6);

  uart_send(',');
  
  d = dump_latency_data.tx_enter - dump_latency_data.rx_leave;
  dword_to_dec(d,buf,5,4);
  uart_send_data(buf,6);

  uart_send(',');

  d = dump_latency_data.tx_leave - dump_latency_data.tx_enter;
  dword_to_dec(d,buf,5,4);
  uart_send_data(buf,6);
  
  uart_send('>');
  uart_send(' ');
}

extern void dump_line(const u08 *eth_buf, u16 size)
{
  if(param.dump_eth) {
    dump_eth_pkt(eth_buf, size);
  }
  const u08 *ip_buf = eth_buf + ETH_HDR_SIZE;
  u16 type = eth_get_pkt_type(eth_buf);
  if(type == ETH_TYPE_ARP) {
    if(param.dump_arp) {
      dump_arp_pkt(ip_buf);
    }
  } else if(type == ETH_TYPE_IPV4) {
    if(param.dump_ip) {
      dump_ip_pkt(ip_buf);
      if(param.dump_proto) {
        dump_ip_protocol(ip_buf);
      }
    }
  }
}

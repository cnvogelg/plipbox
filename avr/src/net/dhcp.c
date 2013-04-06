/*
 * dhcp.c - handle DHCP messages
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

#include "dhcp.h"
#include "bootp.h"
#include "eth.h"
#include "ip.h"
#include "udp.h"
#include "uart.h"
#include "uartutil.h"

u16 dhcp_begin_pkt(u08 *buf, u08 op)
{
  u08 offset = bootp_begin_pkt(buf, op);
  u08 *ptr = buf + offset + BOOTP_OFF_VEND;
  
  // DHCP magic
  ptr[0] = 0x63;
  ptr[1] = 0x82;
  ptr[2] = 0x53;
  ptr[3] = 0x63;
  
  return offset + BOOTP_OFF_VEND + 4;
}

u16 dhcp_finish_pkt(u08 *buf, u16 size)
{
  return bootp_finish_pkt(buf, size);
}

u16 dhcp_make_discover_eth_pkt(u08 *buf)
{
  u16 off = dhcp_begin_eth_pkt_multicast(buf, BOOTP_REQUEST);
  u08 *opt = buf + off;
  opt = dhcp_add_type(opt, DHCP_TYPE_DISCOVER);
  opt = dhcp_add_end(opt);
  return dhcp_finish_eth_pkt(buf, BOOTP_MIN_SIZE);
}

static void make_req_from_off(u08 *buf, u16 offset)
{
  u16 opt_off = offset + BOOTP_OFF_VEND + 4;

  // clear CIADDR
  u08 *ptr = buf + offset;
  u08 my_ip[4];
  net_copy_ip(&ptr[BOOTP_OFF_YIADDR], my_ip);
  net_copy_ip(net_zero_ip, &ptr[BOOTP_OFF_CIADDR]);
  net_copy_ip(net_zero_ip, &ptr[BOOTP_OFF_YIADDR]);
  
  u08 *options = buf + opt_off;

  // get server id
  const u08 *result;
  u08 srv_id[4];
  dhcp_get_option_type(options, 54, &result); // server ID
  net_copy_ip(result, srv_id);

#ifdef DEBUG
  net_dump_ip(srv_id);
  uart_send_crlf();
  net_dump_ip(my_ip);
  uart_send_crlf();
#endif
  
  // setup DHCP options
  options = dhcp_add_type(options, DHCP_TYPE_REQUEST);
  options = dhcp_add_ip(options, 54, srv_id); // server ID
  options = dhcp_add_ip(options, 50, my_ip); // requested IP addr
  dhcp_add_end(options);
}

static u16 make_renew_from_off(u08 *buf, u16 offset, const u08 *srv_ip, const u08 *srv_id)
{
  // bootp pointer
  u08 *ptr = buf + (offset - BOOTP_OFF_VEND - 4);
  net_copy_ip(srv_id, ptr + BOOTP_OFF_XID);
  net_copy_ip(srv_ip, ptr + BOOTP_OFF_SIADDR);
  net_copy_ip(net_get_ip(), ptr + BOOTP_OFF_CIADDR);
  
  u08 *options = buf + offset;
  options = dhcp_add_type(options, DHCP_TYPE_REQUEST);
  dhcp_add_end(options);
  
  return BOOTP_MIN_SIZE;
}

u16 dhcp_make_renew_pkt(u08 *ip_buf, const u08 *srv_ip, const u08 *srv_id)
{
  u08 offset = dhcp_begin_pkt(ip_buf, BOOTP_REQUEST);
  u16 size = make_renew_from_off(ip_buf, offset, srv_ip, srv_id);
  return dhcp_finish_pkt(ip_buf, size);
}

void dhcp_make_request_from_offer_pkt(u08 *ip_buf)
{
  u08 offset = bootp_begin_swap_pkt(ip_buf);
  make_req_from_off(ip_buf, offset);
  bootp_finish_swap_pkt(ip_buf);
}

u16 dhcp_begin_eth_pkt_multicast(u08 *buf, u08 op)
{
  eth_make_to_any(buf, ETH_TYPE_IPV4);
  return dhcp_begin_pkt(buf + ETH_HDR_SIZE, op) + ETH_HDR_SIZE;
}

u16 dhcp_begin_eth_pkt_unicast(u08 *buf, u08 op, const u08 *mac)
{
  eth_make_to_tgt(buf, ETH_TYPE_IPV4, mac);
  return dhcp_begin_pkt(buf + ETH_HDR_SIZE, op) + ETH_HDR_SIZE;
}

u16 dhcp_finish_eth_pkt(u08 *buf, u16 size)
{
  return dhcp_finish_pkt(buf + ETH_HDR_SIZE, size) + ETH_HDR_SIZE;
}

void dhcp_make_request_from_offer_eth_pkt(u08 *eth_buf)
{
  u08 offset = bootp_begin_swap_eth_pkt(eth_buf);
  make_req_from_off(eth_buf, offset);
  bootp_finish_swap_eth_pkt(eth_buf);
}

u16 dhcp_make_renew_eth_pkt(u08 *eth_buf, const u08 *srv_mac, const u08 *srv_ip, const u08 *srv_id)
{
  u16 offset = dhcp_begin_eth_pkt_unicast(eth_buf, BOOTP_REQUEST, srv_mac);
  u16 size = make_renew_from_off(eth_buf, offset, srv_ip, srv_id);
  return dhcp_finish_eth_pkt(eth_buf, size);
}

u08 *dhcp_add_type(u08 *buf, u08 type)
{
  buf[0] = 53;
  buf[1] = 1;
  buf[2] = type;
  return buf+3;
}

u08 *dhcp_add_ip(u08 *buf, u08 type, const u08 *ip)
{
  buf[0] = type;
  buf[1] = 4;
  net_copy_ip(ip, buf+2);
  return buf + 6;
}

u08 *dhcp_add_end(u08 *buf)
{
  buf[0] = 0xff;
  return buf+1;
}

u08 dhcp_is_dhcp_pkt(u08 *buf)
{
  if(bootp_is_bootp_pkt(buf)) {
    u08 *magic_buf = buf + ip_get_hdr_length(buf) + UDP_DATA_OFF + BOOTP_OFF_VEND;
    return (magic_buf[0] == 0x63) && (magic_buf[1] == 0x82) &&
      (magic_buf[2] == 0x53) && (magic_buf[3] == 0x63);
  } else {
    return 0;
  }
}

void dhcp_parse_options(const u08 *opt_buf, dhcp_handle_option_func func)
{
  while(*opt_buf != 0xff) {
    u08 c = *opt_buf;
    if(c == 0) {
      // skip
      opt_buf++;
    } else {
      u08 type = opt_buf[0];
      u08 size = opt_buf[1];
      func(type, size, opt_buf + 2);
      opt_buf += 2 + size;
    }
  }
}

u08 dhcp_set_message_type(u08 *opt_buf, u08 msg_type)
{
  u08 old_type = 0;
  while(*opt_buf != 0xff) {
    u08 c = *opt_buf;
    if(c==0) {
      opt_buf++;
    } else {
      u08 type = opt_buf[0];
      u08 size = opt_buf[1];
      // DHCP message type
      if(type == 53) {
        old_type = opt_buf[2];
        opt_buf[2] = msg_type;
      }
      opt_buf += 2 + size;      
    }
  }
  return old_type;
}

u08 dhcp_get_message_type(const u08 *opt_buf)
{
  const u08 *ptr;
  u08 size = dhcp_get_option_type(opt_buf, 53, &ptr);
  if(size == 1) {
    return *ptr;
  } else {
      return 0;
  }
}

u08 dhcp_get_option_type(const u08 *opt_buf, u08 msg_type, const u08 **ptr)
{
  while(*opt_buf != 0xff) {
    u08 c = *opt_buf;
    if(c==0) {
      opt_buf++;
    } else {
      u08 type = opt_buf[0];
      u08 size = opt_buf[1];
      if(type == msg_type) {
        *ptr = opt_buf + 2;
        return size;
      }
      opt_buf += 2 + size;      
    }
  }
  return 0;
}

void dhcp_dump_options(u08 option, u08 size, const u08 *data)
{
  uart_send_hex_byte_spc(option);
  uart_send_hex_byte_crlf(size);
}


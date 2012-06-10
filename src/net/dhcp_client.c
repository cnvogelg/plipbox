/*
 * dhcp_client.c - act as a DHCP client to get a lease
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

#include "dhcp_client.h"
#include "param.h"
#include "uart.h"
#include "uartutil.h"
#include "eth.h"
#include "ip.h"
#include "udp.h"

#define DHCP_STATE_INIT         0   
#define DHCP_STATE_WAIT_OFFER   1
#define DHCP_STATE_WAIT_ACK     2
#define DHCP_STATE_HAVE_ACK     3

static u08 state = DHCP_STATE_INIT;

static u16 make_discover(u08 *buf, u16 max_size)
{
  u16 off = dhcp_begin_eth_pkt(buf, BOOTP_REQUEST);
  u08 *opt = buf + off;
  opt = dhcp_add_type(opt, DHCP_TYPE_DISCOVER);
  dhcp_add_end(opt);
  return dhcp_finish_eth_pkt(buf);
}

void dhcp_client_worker(u08 *eth_buf, u16 max_size, net_tx_packet_func tx_func)
{
  u16 size;
  switch(state) {
    case DHCP_STATE_INIT:
      // clear my ip
      net_copy_ip(net_zero_ip, param.ip_eth_addr);
      net_copy_ip(net_zero_ip, param.ip_net_mask);
      net_copy_ip(net_zero_ip, param.ip_gw_addr);
      // send a discover message
      size = make_discover(eth_buf, max_size);
      tx_func(eth_buf, size);
      uart_send_pstring(PSTR("DHCP: discover\r\n"));
      state = DHCP_STATE_WAIT_OFFER;
      break;
    default:
      break;
  }
}

static void get_ip_settings(const u08 *bootp_buf)
{
  // get my ip
  net_copy_ip(bootp_buf + BOOTP_OFF_YIADDR, param.ip_eth_addr);
  
  // DHCP options
  const u08 *ptr;
  const u08 *options = bootp_buf + BOOTP_OFF_VEND + 4;
  
  // subnet mask
  u08 size = dhcp_get_option_type(options, 1, &ptr); // subnet mask
  if(size == 4) {
    net_copy_ip(ptr, param.ip_net_mask);
  }
  
  // router addr
  size = dhcp_get_option_type(options, 3, &ptr); // router
  if(size == 4) {
    net_copy_ip(ptr, param.ip_gw_addr);
  }
  
  param_dump();
}

u08 dhcp_client_handle_packet(u08 *eth_buf, u16 eth_size, net_tx_packet_func tx_func)
{
  u08 *ip_buf = eth_buf + ETH_HDR_SIZE;
  u08 *udp_data = ip_buf + ip_get_hdr_length(ip_buf) + UDP_DATA_OFF;
  const u08 *options = dhcp_get_options(udp_data);
  
  u08 msg_type = dhcp_get_message_type(options);  
  uart_send_pstring(PSTR("DHCP: packet "));
  uart_send_hex_byte_spc(msg_type);
  
  // OFFER -> REQUEST
  if(msg_type == DHCP_TYPE_OFFER) {
    uart_send_pstring(PSTR("=offer"));
    if(state == DHCP_STATE_WAIT_OFFER) {
      // send a REQUEST
      uart_send_pstring(PSTR(" -> request\r\n"));
      state = DHCP_STATE_WAIT_ACK;
      
      dhcp_make_request_from_offer_eth_pkt(eth_buf);
      tx_func(eth_buf, eth_size);      
    } else {
      uart_send_pstring(PSTR(" ignore!\r\n"));  
    }
  } 
  // ACK
  else if(msg_type == DHCP_TYPE_ACK) {
    uart_send_pstring(PSTR("=ack"));
    if(state == DHCP_STATE_WAIT_ACK) {
      // extract settings
      uart_send_pstring(PSTR(" -> got values\r\n"));
      get_ip_settings(udp_data);
      state = DHCP_STATE_HAVE_ACK;
    } else {
      uart_send_pstring(PSTR(" ignore!\r\n"));
    }
  }
  // unknown DHCP packet
  else {
    uart_send_pstring(PSTR(" ??\r\n"));
  }
  
  return 1;
}

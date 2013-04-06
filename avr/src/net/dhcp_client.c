/*
 * dhcp_client.c - act as a DHCP client to get a lease
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

#include "dhcp_client.h"
#include "uart.h"
#include "uartutil.h"
#include "eth.h"
#include "ip.h"
#include "udp.h"
#include "timer.h"

#define DHCP_STATE_INIT         0   
#define DHCP_STATE_WAIT_OFFER   1
#define DHCP_STATE_WAIT_ACK     2
#define DHCP_STATE_HAVE_LEASE   3
#define DHCP_STATE_DO_RENEW     4

static u08 state = DHCP_STATE_INIT;
static u32 alarm_time = 0;
static u32 lease_time = 10;
static u08 renew_retries = 0;

static u08 server_ip[4];
static u08 server_mac[6];
static u08 server_id[4];

#define RETRY_DELTA_SEC   5
#define MAX_RENEW_RETRIES 3

u08 dhcp_client_worker(u08 *eth_buf, u16 max_size, net_tx_packet_func tx_func)
{
  u16 size;
  switch(state) {
    case DHCP_STATE_INIT:
      // clear my ip
      net_copy_ip(net_zero_ip, net_get_ip());
      net_copy_ip(net_zero_ip, net_get_netmask());
      net_copy_ip(net_zero_ip, net_get_gateway());
      // send a discover message
      size = dhcp_make_discover_eth_pkt(eth_buf);
      tx_func(eth_buf, size);
      uart_send_pstring(PSTR("DHCP: discover\r\n"));
      state = DHCP_STATE_WAIT_OFFER;
      break;
    case DHCP_STATE_HAVE_LEASE:
      // check for lease renew time
      if(clock_1s >= alarm_time) {
        uart_send_pstring(PSTR("DHCP: renew lease\r\n"));
        state = DHCP_STATE_DO_RENEW;
        renew_retries = 0;
        alarm_time = clock_1s;
      }
      break;  
    case DHCP_STATE_DO_RENEW:
      // send a request to old server
      if(clock_1s >= alarm_time) {
        if(renew_retries == MAX_RENEW_RETRIES) {
          // do a real discover
          state = DHCP_STATE_INIT;
          uart_send_pstring(PSTR("DHCP: rebinding\r\n"));
        }
        else {
          uart_send_pstring(PSTR("DHCP: sending renew request\r\n"));
          size = dhcp_make_renew_eth_pkt(eth_buf, server_mac, server_ip, server_id);
          tx_func(eth_buf, size);
          renew_retries++;
        }
        alarm_time = clock_1s + RETRY_DELTA_SEC;
      }
      break;
    default:
      break;
  }
  return (state == DHCP_STATE_HAVE_LEASE);
}

static void get_ip_settings(const u08 *bootp_buf)
{
  // get my ip
  net_copy_ip(bootp_buf + BOOTP_OFF_YIADDR, net_get_ip());
  uart_send_pstring(PSTR("DHCP: my ip="));
  net_dump_ip(net_get_ip());
  
  // DHCP options
  const u08 *ptr;
  const u08 *options = bootp_buf + BOOTP_OFF_VEND + 4;
  
  // subnet mask
  u08 size = dhcp_get_option_type(options, 1, &ptr); // subnet mask
  if(size == 4) {
    net_copy_ip(ptr, net_get_netmask());
    uart_send_pstring(PSTR(" mask="));
    net_dump_ip(net_get_netmask());
  }
  
  // router addr
  size = dhcp_get_option_type(options, 3, &ptr); // router
  if(size == 4) {
    net_copy_ip(ptr, net_get_gateway());
    uart_send_pstring(PSTR(" gw="));
    net_dump_ip(net_get_gateway());
  }
  
  // lease time
  size = dhcp_get_option_type(options, 51, &ptr); // lease time
  if(size == 4) {
    lease_time = net_get_long(ptr);
    
    // calc the lease renew time
    alarm_time = clock_1s + (lease_time >> 1);
  }
  
  // copy server id (as "ip" type because its also 4 bytes :)
  size = dhcp_get_option_type(options, 54, &ptr); // server id
  if(size == 4) {
    net_copy_ip(ptr, server_id);
  }
  
  uart_send_crlf();
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
      // copy server mac
      net_copy_mac(eth_get_src_mac(eth_buf), server_mac);
      uart_send_pstring(PSTR(", server mac="));
      net_dump_mac(server_mac);
      
      // copy server ip
      net_copy_ip(udp_data + BOOTP_OFF_SIADDR, server_ip);
      uart_send_pstring(PSTR(" ip="));
      net_dump_ip(server_ip);
      
      // send a REQUEST
      uart_send_pstring(PSTR(" -> request\r\n"));
      state = DHCP_STATE_WAIT_ACK;
      
      dhcp_make_request_from_offer_eth_pkt(eth_buf);
      tx_func(eth_buf, eth_size);            
    } else {
      uart_send_pstring(PSTR(" ignore offer!\r\n"));  
    }
  } 
  // ACK
  else if(msg_type == DHCP_TYPE_ACK) {
    uart_send_pstring(PSTR("=ack"));
    if((state == DHCP_STATE_WAIT_ACK) || (state == DHCP_STATE_DO_RENEW)) {
      // extract settings
      uart_send_pstring(PSTR(" -> got values\r\n"));
      get_ip_settings(udp_data);
      state = DHCP_STATE_HAVE_LEASE;
    } else {
      uart_send_pstring(PSTR(" ignore ack!\r\n"));
    }
  }
  // unknown DHCP packet
  else {
    uart_send_pstring(PSTR(" ??\r\n"));
  }
  
  return 1;
}

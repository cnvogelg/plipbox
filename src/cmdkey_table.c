/*
 * cmdkey_table.c - command key table
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

#include "cmdkey_table.h"

#include "ping.h"
#include "uartutil.h"
#include "timer.h"
   
#include "net/net.h"
#include "net/arp_cache.h"
#include "net/udp.h"
#include "net/eth.h"
#include "net/bootp.h"
#include "net/dhcp.h"

#include "pkt_buf.h"
#include "enc28j60.h"

// ping server
COMMAND_KEY(cmd_ping_server)
{
  u16 id = timer_10ms;
  ping_eth_send_request(net_get_srv_ip(), id, 0);
}

// ping gateway
COMMAND_KEY(cmd_ping_gw)
{
  u16 id = timer_10ms;
  ping_eth_send_request(net_get_gateway(), id, 0);
}

// ping amiga
COMMAND_KEY(cmd_ping_amiga)
{
  u16 id = timer_10ms;
  ping_plip_send_request(net_get_p2p_amiga(), id, 0);
}

// arp cache
COMMAND_KEY(cmd_dump_arp_cache)
{
  arp_cache_dump();
}

COMMAND_KEY(cmd_clear_arp_cache)
{
  arp_cache_clear();
  uart_send_pstring(PSTR("ARP clear\r\n"));
}

// ----- tests -----
// send a udp packet
COMMAND_KEY(cmd_udp_test)
{
  const u08 *mac = arp_cache_find_mac(net_get_srv_ip());
  if(mac != 0) {
    u08 *buf = pkt_buf + ETH_HDR_SIZE;
    u08 off = udp_begin_pkt(buf, net_get_ip(), 42, net_get_srv_ip(), 6800);
    buf[off] = 'C';
    buf[off+1] = 'V';
    udp_finish_pkt(buf, 2);
    eth_make_to_tgt(pkt_buf, ETH_TYPE_IPV4, mac);
    u16 size = off + 2 + ETH_HDR_SIZE;
    enc28j60_packet_tx(pkt_buf, size);
    uart_send_pstring(PSTR("UDP!\r\n"));
  }
}

COMMAND_KEY(cmd_bootp_test)
{
  bootp_begin_eth_pkt(pkt_buf, BOOTP_REQUEST);
  u16 size = bootp_finish_eth_pkt(pkt_buf, BOOTP_MIN_SIZE);
  enc28j60_packet_tx(pkt_buf, size);
  uart_send_pstring(PSTR("BOOTP!\r\n"));
}

COMMAND_KEY(cmd_dhcp_test)
{
  u16 off = dhcp_begin_eth_pkt_multicast(pkt_buf, BOOTP_REQUEST);
  u08 *opt = pkt_buf + off;
  opt = dhcp_add_type(opt, DHCP_TYPE_DISCOVER);
  dhcp_add_end(opt);
  u16 size = dhcp_finish_eth_pkt(pkt_buf, BOOTP_MIN_SIZE);
  enc28j60_packet_tx(pkt_buf, size);
  uart_send_pstring(PSTR("DHCP!\r\n"));
}

cmdkey_table_t cmdkey_table[] = {
    /* arp handling */
  { 'c', cmd_dump_arp_cache },
  { 'C', cmd_clear_arp_cache },
    /* ping machines */
  { 's', cmd_ping_server },
  { 'g', cmd_ping_gw },
  { 'a', cmd_ping_amiga },
    /* internal tests */
  { 'd', cmd_dhcp_test },
  { 'b', cmd_bootp_test },
  { 'u', cmd_udp_test },
  { 0,0 }
};

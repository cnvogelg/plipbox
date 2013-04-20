/*
 * eth_tx.c: handle eth sends
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

#include "eth_tx.h"

#include "net/net.h"
#include "net/eth.h"
#include "pkt_buf.h"
#include "enc28j60.h"
#include "uartutil.h"
#include "debug.h"

#ifdef DEBUG
static void uart_send_prefix(void)
{
  uart_send_pstring(PSTR(" eth(TX): "));
}
#endif

void eth_tx_send(u16 eth_type, u16 ip_size, u16 copy_size, const u08 *tgt_mac)
{
  // now build ethernet header
  eth_make_to_tgt(pkt_buf, eth_type, tgt_mac);

#ifdef DEBUG
  debug_dump_eth_pkt(pkt_buf, ip_size, uart_send_prefix);
  if(eth_type == ETH_TYPE_ARP) {
    debug_dump_arp_pkt(pkt_buf + ETH_HDR_SIZE, uart_send_prefix);
  }
#endif

  // wait for tx is possible
  enc28j60_packet_tx_prepare();

  // copy (newly created) eth header
  enc28j60_packet_tx_begin_range(0);
  enc28j60_packet_tx_blk(pkt_buf, copy_size);
  enc28j60_packet_tx_end_range();

  // finally send ethernet packet
  u16 pkt_size = ip_size + ETH_HDR_SIZE;
  enc28j60_packet_tx_send(pkt_size);
}

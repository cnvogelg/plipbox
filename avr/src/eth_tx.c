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

#include "net/eth.h"
#include "pkt_buf.h"
#include "enc28j60.h"

void eth_tx_send(u16 eth_type, u16 ip_size, u16 copy_size, const u08 *tgt_mac)
{
  // now build ethernet header
  eth_make_to_tgt(pkt_buf, eth_type, tgt_mac);

  // wait for tx is possible
  enc28j60_packet_tx_prepare();

  // copy (newly created) eth header
  // [and with nat: (modified) ip header] back to packet buffer
  enc28j60_packet_tx_begin_range(0);
  enc28j60_packet_tx_blk(pkt_buf, copy_size);
  enc28j60_packet_tx_end_range();

  // finally send ethernet packet
  u16 pkt_size = ip_size + ETH_HDR_SIZE;
  enc28j60_packet_tx_send(pkt_size);
}

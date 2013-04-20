/*
 * eth.c - working with ethernet packets
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

#include "eth.h"
#include "net.h"
#include "uartutil.h"

u08  eth_is_broadcast_tgt(const u08 *pkt)
{
  const u08 *mac = eth_get_tgt_mac(pkt);
  return net_compare_bcast_mac(mac);
}

u08  eth_is_tgt_me(const u08 *pkt)
{
  const u08 *mac = eth_get_tgt_mac(pkt);
  return net_compare_my_mac(mac);  
}

void eth_make_reply(u08 *pkt)
{
  /* old src -> new tgt */
  net_copy_mac(pkt + ETH_OFF_SRC_MAC, pkt + ETH_OFF_TGT_MAC);
  /* my mac is src */
  net_copy_my_mac(pkt + ETH_OFF_SRC_MAC);
}

void eth_make_to_tgt(u08 *pkt, u16 type, const u08 mac[6])
{
  net_copy_mac(mac, pkt + ETH_OFF_TGT_MAC);
  net_copy_my_mac(pkt + ETH_OFF_SRC_MAC);
  net_put_word(pkt + ETH_OFF_TYPE, type);
}

void eth_make_to_any(u08 *pkt, u16 type)
{
  net_copy_bcast_mac(pkt + ETH_OFF_TGT_MAC);
  net_copy_my_mac(pkt + ETH_OFF_SRC_MAC);
  net_put_word(pkt + ETH_OFF_TYPE, type);
}

extern void eth_dump(const u08 *pkt)
{
  uart_send_string("eth:tgt=");
  net_dump_mac(pkt + ETH_OFF_TGT_MAC);
  uart_send_string(",src=");
  net_dump_mac(pkt + ETH_OFF_SRC_MAC);
  uart_send_string(",type=");
  u16 type = eth_get_pkt_type(pkt);
  uart_send_hex_word_spc(type);
}

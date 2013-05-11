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
#include "dump.h"
#include "param.h"

static void uart_send_prefix(void)
{
  uart_send_time_stamp_spc();
  uart_send_pstring(PSTR(" eth(TX): "));
}

void eth_tx_send(u16 eth_size)
{
  // dump eth packet
  if(param.dump_dirs & DUMP_DIR_ETH_TX) {
    uart_send_prefix();
    dump_line(rx_pkt_buf, eth_size);
    uart_send_crlf();
  }

  // wait for tx is possible
  enc28j60_packet_tx_prepare();

  // finally send ethernet packet
  enc28j60_packet_tx_send(eth_size);
}

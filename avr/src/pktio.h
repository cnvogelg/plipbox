/*
 * pktio.h - generic packet io interface
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

#ifndef PKTIO_H
#define PKTIO_H

#include "global.h"

#ifdef HAVE_NETIO_eth

#include "eth/enc28j60.h"
 
/* consts */
#define PKTIO_NAME              "enc28j60"
#define PKTIO_RX_ERR            ENC28J60_RX_ERR
#define PKTIO_TX_ERR            ENC28J60_TX_ERR
 
/* main */
#define pktio_init(x)           enc28j60_init(x)
#define pktio_start(x)          enc28j60_start(x)
#define pktio_stop()            enc28j60_stop()
#define pktio_get_status()      enc28j60_get_status()
#define pktio_flow_control(x)   enc28j60_flow_control(x)
#define pktio_is_link_up()		enc28j60_is_link_up()

/* rx */       
#define pktio_rx_num_waiting()  enc28j60_packet_rx_num_waiting()
#define pktio_rx_begin()        enc28j60_packet_rx_begin()
#define pktio_rx_blk(d,s)       enc28j60_packet_rx_blk(d,s)
#define pktio_rx_end()          enc28j60_packet_rx_end()
#define pktio_rx_data_begin()   enc28j60_packet_rx_data_begin()
#define pktio_rx_byte()         enc28j60_packet_rx_byte()
#define pktio_rx_data_end()     enc28j60_packet_rx_data_end()

/* tx */
#define pktio_tx_begin(x)       enc28j60_packet_tx_begin()
#define pktio_tx_end()          enc28j60_packet_tx_end()
#define pktio_tx_byte(b)        enc28j60_packet_tx_byte(b)
#define pktio_tx_send(s)        enc28j60_packet_tx_send(s)
#define pktio_tx_reject()

#else /* HAVE_NETIO_eth */

#error undefined HAVE_NETIO

#endif /* HAVE_NETIO_eth */

#endif

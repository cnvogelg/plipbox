/*
 * pio_util.h - high level calls for using pio
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

#ifndef PIO_UTIL_H
#define PIO_UTIL_H

#include "global.h"

/* get the configured init flags for PIO */
extern u08 pio_util_get_init_flags(void);

/* receive packet from current PIO and store in pkt_buf.
   also update stats and is verbose if enabled.
   only call if pio_has_recv() ist not 0!
   returns packet size and pio status.
*/
extern u08 pio_util_recv_packet(u16 *size);

/* send packet to current PIO from pkt_buf
   aöso updates stats and is verbose if enabled.
   return pio status.
*/
extern u08 pio_util_send_packet(u16 size);

/* check current packet in pkt_buf if its an ARP packet.
   return 1 if its ARP.
   if its an ARP request for me then reply it and
   create a reply packet in pkt_buf first.
*/
extern u08 pio_util_handle_arp(u16 size);

/* check if its an incoming UDP test packet.
   if yes prepare reply packet in pkt_buf but do
   NOT send it.
   returns 1 if test packet was handled.
*/
extern u08 pio_util_handle_udp_test(u16 size);

#endif

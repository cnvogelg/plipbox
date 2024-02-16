/*
 * enc28j60.h - pio_dev implementaton for Microchip ENC28J60
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

#ifndef ENC28J60_H
#define ENC28J60_H

#include "arch.h"
#include "types.h"

#define ENC28J60_FLAG_FULL_DUPLEX     1
#define ENC28J60_FLAG_FLOW_CONTROL    2
#define ENC28J60_FLAG_RX_BROADCAST    4

#define ENC28J60_OK                   0
#define ENC28J60_ERROR_NOT_FOUND      1
#define ENC28J60_ERROR_RX             2

// reset and return OK or NOT_FOUND
u08 enc28j60_num_ports(void);
u08 enc28j60_reset_and_find(u08 spi_cs);

u08 enc28j60_hw_revision(void);

void enc28j60_setup_buffers(void);
void enc28j60_setup_mac_phy(const mac_t mac, u08 flags);

void enc28j60_enable_rx(void);
void enc28j60_disable_rx(void);

void enc28j60_control_flow(u08 flags, u08 on);

u08 enc28j60_link_up(void);

void enc28j60_tx_begin(void);
void enc28j60_tx_data(const u08 *data, u16 size);
void enc28j60_tx_end(u16 size);

void enc28j60_tx_begin_loop_back(void);
void enc28j60_tx_end_loop_back(void);

u08 enc28j60_rx_num_pending(void);
u08 enc28j60_rx_size(u16 *got_size);
void enc28j60_rx_begin(void);
void enc28j60_rx_data(u08 *data, u16 size);
void enc28j60_rx_end(void);

void enc28j60_rx_begin_loop_back(void);
void enc28j60_rx_end_loop_back(void);

#endif

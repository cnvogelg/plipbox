/*
 * nic.h - network interface connector
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

#ifndef NIC_H
#define NIC_H

#include "types.h"
#include "nic_shared.h"

/* ioctl */
#define NIC_IOCTL_GET_HW_VERSION      0
#define NIC_IOCTL_GET_LINK_STATUS     1

/* --- API --- */

extern void nic_init(void);
extern void nic_set_device(u08 device);

extern u08 nic_attach_params(void);
extern u08 nic_attach(u16 caps, u08 port, mac_t mac);
extern void nic_detach(void);

extern void nic_ping(void);
extern void nic_status(void);

extern u16 nic_caps_available(void);
extern u16 nic_caps_in_use(void);
extern u08 nic_is_direct(void);

extern u08 nic_rx_num_pending(void);
extern u08 nic_rx_size(u16 *got_size);

// buffer API rx/tx
extern u08 nic_rx_data(u08 *buf, u16 size);
extern u08 nic_tx_data(const u08 *buf, u16 size);

// direct API rx/tx
extern void nic_rx_direct_begin(u16 size);
extern u08 nic_rx_direct_end(u16 size);
extern void nic_tx_direct_begin(u16 size);
extern u08 nic_tx_direct_end(u16 size);

extern u08 nic_ioctl(u08 status_id, u08 *value);

#endif

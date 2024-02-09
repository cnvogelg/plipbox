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

/* result values */
#define NIC_OK                0
#define NIC_ERROR_NOT_FOUND   1

/* capabilities */
#define NIC_CAP_DIRECT_IO       1
#define NIC_CAP_BROADCAST       2
#define NIC_CAP_FULL_DUPLEX     4
#define NIC_CAP_FLOW_CONTROL    8

/* init flags */
#define NIC_FLAG_FULL_DUPLEX    1
#define NIC_FLAG_LOOP_BACK      2
#define NIC_FLAG_BROADCAST      4
#define NIC_FLAG_FLOW_CONTROL   8

/* ioctl */
#define NIC_IOCTL_GET_HW_VERSION      0
#define NIC_IOCTL_GET_LINK_STATUS     1

/* --- API --- */

extern void nic_init(void);
extern void nic_set_device(u08 device);

extern u08 nic_attach_params(void);
extern u08 nic_attach(u08 flags, mac_t mac);
extern void nic_detach(void);

extern u16 nic_capabilites(void);
extern u08 nic_is_direct(void);

extern void nic_enable(void);
extern void nic_disable(void);

extern u08 nic_rx_num_pending(void);
extern u16 nic_rx_size(void);

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

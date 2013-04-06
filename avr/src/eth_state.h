/*
 * eth_tx.h: handle state of ethernet adapter
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

#ifndef ETH_STATE_H
#define ETH_STATE_H

#include "global.h"

#define ETH_STATE_OFFLINE   0
#define ETH_STATE_ONLINE    1
#define ETH_STATE_START_NET 2
#define ETH_STATE_STOP_NET  3
#define ETH_STATE_LINK_UP   4
#define ETH_STATE_LINK_DOWN 5
#define ETH_STATE_WAIT_ARP  6
#define ETH_STATE_NONE      99
   
// 64*10 ms
#define ETH_STATE_TIMER_MASK    0xffc0   

extern u08 eth_state_worker(void);
   
#endif

/*
 * stats.h - manage device statistics
 *
 * Written by
 *  Christian Vogelgsang <chris@vogelgsang.org>
 *
 * This file is part of plip2slip.
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

#ifndef STATS_H
#define STATS_H

#include "global.h"

/*
  rx: PLIP rx, SLIP tx
  tx: PLIP tx, SLIP rx
*/

typedef struct {
  u16 rx_cnt;
  u16 tx_cnt;
  u32 rx_bytes;
  u32 tx_bytes;
  u16 tx_err;
  u16 rx_err;
  u08 last_tx_err;
  u08 last_rx_err;
  u16 rx_drop;
  u16 tx_drop;
  u16 rx_coll;
  u16 tx_coll;
} stats_t;

extern stats_t stats;

extern void stats_reset(void);
extern void stats_dump(void);

#endif

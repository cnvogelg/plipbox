/*
 * stats.h - manage device statistics
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

#ifndef STATS_H
#define STATS_H

#include "global.h"

#define STATS_ID_PB_RX  0
#define STATS_ID_PB_TX  1
#define STATS_ID_PIO_RX 2
#define STATS_ID_PIO_TX 3
#define STATS_ID_NUM    4

typedef struct {
  u32 bytes;
  u16 cnt;
  u16 err;
  u16 drop;
  u16 max_rate;
} stats_t;

extern stats_t stats[STATS_ID_NUM];

extern void stats_reset(void);
extern void stats_dump_all(void);
extern void stats_dump(u08 pb, u08 pio);
extern void stats_update_ok(u08 id, u16 size, u16 rate);

inline stats_t *stats_get(u08 id)
{
  return &stats[id];
}

#endif

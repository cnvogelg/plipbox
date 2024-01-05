/*
 * param.h - handle device parameters
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

#ifndef PARAM_H
#define PARAM_H

#include "types.h"
#include "arch.h"
#include "hw_persist.h"

struct param_def {
  // --- shared via protocol
  u08   index;
  u08   type;
  u08   format;
  u16   size;
  u32   tag;
  // --- internal
  u08  *data;
  rom_pchar desc;
};
typedef struct param_def param_def_t;

// init parameters. try to load from eeprom or use default (returns hw_persist_status)
u08 param_init(void);
// save param to eeprom (returns hw_persist status)
u08 param_save(void);
// load param from eeprom (returns hw_persist status)
u08 param_load(void);
// reset param
void param_reset(void);
// show params
void param_dump(void);

// MAC parameter
void param_get_def_mac(mac_t mac);
void param_get_cur_mac(mac_t mac);
void param_set_cur_mac(mac_t mac);

// generic param functions
u08 param_get_num(void);
u08 param_find_tag(u32 tag);
void param_get_def(u08 index, param_def_t *def);
u08 *param_get_data(u08 index);

#endif

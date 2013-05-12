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

#ifndef _PARAM_H
#define _PARAM_H

#include "global.h"

#define DUMP_DIR_PLIP_RX  1
#define DUMP_DIR_ETH_RX   2
#define DUMP_DIR_PLIP_TX  4
#define DUMP_DIR_ETH_TX   8

typedef struct {
  u08 mac_addr[6];
  u08 tx_retries;

  u08 dump_dirs;
  u08 dump_eth;
  u08 dump_ip;
  u08 dump_arp;
  u08 dump_proto;
  u08 dump_plip;
  u08 dump_latency;
} param_t;
  
extern param_t param;  

// param result
#define PARAM_OK                  0
#define PARAM_EEPROM_NOT_READY    1
#define PARAM_EEPROM_CRC_MISMATCH 2

// init parameters. try to load from eeprom or use default
void param_init(void);
// save param to eeprom (returns param result) 
u08 param_save(void);
// load param from eeprom (returns param result)
u08 param_load(void);
// reset param
void param_reset(void);
// show params
void param_dump(void);

#endif

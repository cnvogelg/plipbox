/*
 * transfer.c - main mode for PLIP 2 SLIP bridging
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

#include "transfer.h"

#include "board.h"
#include "slip_rx.h"
#include "plip_rx.h"
#include "ser_parse.h"
#include "param.h"
#include "error.h"

void transfer_loop(void)
{
  plip_rx_init();
  slip_rx_init();

  led_green_on();
  while(param.mode == PARAM_MODE_TRANSFER) {
    // handle receiving PLIP packets
    plip_rx_worker();
    // handle incoming serial
    ser_parse_worker();
    // handle receiving SLIP packets
    slip_rx_worker();
    // error handler
    error_worker();
  }
  led_green_off();
}

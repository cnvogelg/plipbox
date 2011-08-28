/*
 * only_slip_rx.c - test mode: only plip tx, slip rx
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

#include "only_slip_rx.h"

#include "param.h"
#include "ser_parse.h"
#include "slip_rx.h"
#include "board.h"

void only_slip_rx_loop(void)
{
  slip_rx_init();
  
  led_green_on(); 
  while(param.mode == PARAM_MODE_ONLY_SLIP_RX) {
    ser_parse_worker();
    slip_rx_worker();
  }
  led_green_off();
}

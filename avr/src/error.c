/*
 * error.h - error handler
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

#include "error.h"
#include "board.h"

#define ERROR_SPEED   0xfff
#define MAX_ERROR     10

static u08 count;
static u16 delay;
static u08 toggle;

void error_init(void)
{
  count = 0;
  toggle = 0;
  delay = 0;
  led_red_off();
}

void error_worker(void)
{
  if(count > 0) {
    if(delay == 0) {
      toggle = 1 - toggle;
      if(toggle) {
        led_red_on();
      } else {
        led_red_off();
        count--;
      }
      delay = ERROR_SPEED;
    } else {
      delay--;
    }
  }
}

void error_add(void)
{
  if(count < MAX_ERROR) {
    count++;
  }
}

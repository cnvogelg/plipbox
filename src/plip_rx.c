/*
 * plip_rx.c: handle plip receiption and slip tx
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

#include "plip_rx.h"
#include "slip.h"
#include "board.h"
#include "stats.h"
#include "param.h"
#include "pkt_buf.h"
#include "error.h"
#include "log.h"

static u08 begin_rx(plip_packet_t *pkt)
{
  slip_send_end();
  return PLIP_STATUS_OK;
}

static u08 fill_rx(u08 *data)
{
  slip_send(*data);
  return PLIP_STATUS_OK;
}

static u08 end_rx(plip_packet_t *pkt)
{
  slip_send_end();
  return PLIP_STATUS_OK;
}

static u08 fake_rx(plip_packet_t *pkt)
{
  return PLIP_STATUS_OK;
}

static u08 fake_fill_rx(u08 *data)
{
  return PLIP_STATUS_OK;
}

void plip_rx_init(void)
{
  // check fake_tx
  if(param.fake_tx) {
    plip_recv_init(fake_rx, fake_fill_rx, fake_rx);
  } else {
    plip_recv_init(begin_rx, fill_rx, end_rx);
  }  
}

u08 plip_rx_worker(void)
{
    // do we have a PLIP packet waiting?
    if(plip_can_recv() == PLIP_STATUS_OK) {
      led_green_off();
      
      u08 status = plip_recv(&pkt);
      // packet error
      if(status != PLIP_STATUS_OK) {
        stats.last_rx_err = status;
        stats.rx_err++;
        log_add(status);
        error_add();
      } 
      // packet ok
      else {
        stats.rx_cnt++;
        stats.rx_bytes+=pkt.size;
      }
      
      led_green_on();
      return status;
    } 
    // nothing to do - idle
    else {
      return PLIP_STATUS_IDLE;
    }
}

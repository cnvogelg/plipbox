/*
 * eth_tx.c: handle state of ethernet adapter
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

#include "eth_state.h"

#include "timer.h"
#include "uartutil.h"
#include "uart.h"
#include "pktio.h"
#include "param.h"
#include "stats.h"
#include "net/net.h"

static u08 state = ETH_STATE_INIT;
static u08 flow_control = 0;
static u08 shutdown_now = 0;
static u08 configure_now = 0;

void eth_state_init(void)
{
  state = ETH_STATE_INIT;
}

void eth_state_configure(void)
{
  configure_now = 1;
}

void eth_state_shutdown(void)
{
  shutdown_now = 1;
}

static void do_flow_control(void)
{
  u08 num = pktio_rx_num_waiting();

  // too many packets
  if(num >= 3) {
    if(!flow_control) {
      flow_control = 1;
      pktio_flow_control(1);

      if(param.dump_dirs & DUMP_FLOW_CTL) {
        uart_send_time_stamp_spc();
        uart_send_pstring(PSTR("flow on. num_pkt="));
        uart_send_hex_byte(num);
        uart_send_crlf();
      }
    }
  } 
  // no packet waiting lets enable flow
  else if(num <= 1) {
    if(flow_control) {
      flow_control = 0;
      pktio_flow_control(0);

      if(param.dump_dirs & DUMP_FLOW_CTL) {
        uart_send_time_stamp_spc();
        uart_send_pstring(PSTR("flow off. num_pkt="));
        uart_send_hex_byte(num);
        uart_send_crlf();
      }
    }
  }    
}

static void update_status(void)
{
  u08 status = pktio_get_status();
  if(status != 0) {
    if(status & PKTIO_RX_ERR) {
      stats.rx_err ++;
    }
    if(status & PKTIO_TX_ERR) {
      stats.tx_err ++;
    }
    if(param.dump_dirs & DUMP_ERRORS) {
      uart_send_time_stamp_spc();
      uart_send_pstring(PSTR("error="));
      uart_send_hex_byte(status);
      uart_send_crlf();
    }
  }
}

u08 eth_state_worker(void)
{
  switch(state) {
    
    /* init ethernet */
    case ETH_STATE_INIT:
    {
      u08 fd = param.full_duplex;
      u08 lb = param.loop_back;
      u08 rev = pktio_init(fd, lb);

      uart_send_time_stamp_spc();
      uart_send_pstring(PSTR("eth: init " PKTIO_NAME));
      if(rev == 0) {
         uart_send_pstring(PSTR(": ERROR SETTING UP!!\r\n"));
      } else {
        uart_send_pstring(PSTR(" rev "));
        uart_send_hex_byte(rev);
        uart_send_pstring(fd ? PSTR(" full ") : PSTR(" half "));
        uart_send_pstring(PSTR("duplex"));
        if(lb) {
          uart_send_pstring(PSTR(" loop back"));
        }
        uart_send_crlf();
      }
      state = ETH_STATE_CONFIGURE;
      break;
    }

    /* configure eth */
    case ETH_STATE_CONFIGURE:
    {
      // configure mac
      const uint8_t *mac = param.mac_addr;
      pktio_start(mac);
      uart_send_time_stamp_spc();
      uart_send_pstring(PSTR("eth: configure mac="));
      net_dump_mac(mac);      
      uart_send_crlf();
      // restore flow control
      flow_control = 0;
      pktio_flow_control(0);
      state = ETH_STATE_LINK_DOWN;
      configure_now = 0;
      break;
    }

    /* link not yet up */
    case ETH_STATE_LINK_DOWN:
      if(shutdown_now) {
        state = ETH_STATE_SHUTDOWN;
      } else if(configure_now) {
        state = ETH_STATE_CONFIGURE;
      } else {
        /* check if link is up */
        if(pktio_is_link_up()) {
          uart_send_time_stamp_spc();
          uart_send_pstring(PSTR("eth: link up\r\n"));
          state = ETH_STATE_LINK_UP;
        }
      }
      break;

    /* link is up */
    case ETH_STATE_LINK_UP:
      if(!pktio_is_link_up() || shutdown_now || configure_now) {
        uart_send_time_stamp_spc();
        uart_send_pstring(PSTR("eth: link down\r\n"));
        state = ETH_STATE_LINK_DOWN;
      }
      else {
        /* link is alive */
        if(param.flow_ctl) {
          do_flow_control();
        }
        update_status();
      }
      break;

    /* shutdown */
    case ETH_STATE_SHUTDOWN:
      pktio_stop();
      uart_send_time_stamp_spc();
      uart_send_pstring(PSTR("eth: shutting down\r\n"));
      state = ETH_STATE_OFF;
      shutdown_now = 0;
      break;

    /* off state - wait for reinit or configure */
    case ETH_STATE_OFF:
      break;

  }
  return state;
}
    
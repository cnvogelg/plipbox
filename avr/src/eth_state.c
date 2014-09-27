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
#include "pb_io.h"
#include "param.h"
#include "stats.h"

static u08 state = ETH_STATE_LINK_DOWN;
static u08 flow_control = 0;

u08 eth_state_worker(u08 plip_online)
{
  switch(state) {
    case ETH_STATE_LINK_DOWN:
      if(plip_online) {
        pktio_start(sana_mac);
        
        // restore flow control
        flow_control = 0;
        pktio_flow_control(0);
        
        uart_send_time_stamp_spc();
        uart_send_pstring(PSTR("eth: link up\r\n"));
        state = ETH_STATE_LINK_UP;
      }
      break;
    case ETH_STATE_LINK_UP:
      if(!plip_online) {
        pktio_stop();
  
        uart_send_time_stamp_spc();
        uart_send_pstring(PSTR("eth: link down\r\n"));
        state = ETH_STATE_LINK_DOWN;
      } 
      else {
        if(param.flow_ctl) {
          // check flow control
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
        
        // check status
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
      break;
  }
  return state;
}
    
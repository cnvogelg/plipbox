/*
 * ping_plip.c - ping plip side test mode
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

#include "ping_plip.h"
#include "pkt_buf.h"
#include "board.h"
#include "ser_parse.h"
#include "pkt_buf.h"
#include "plip.h"
#include "uart.h"
#include "uartutil.h"
#include "ip.h"
#include "stats.h"
#include "param.h"

static u16 pos;

static u08 begin_rx(plip_packet_t *pkt)
{
  pos = 0;
  return PLIP_STATUS_OK;
}

static u08 fill_rx(u08 *data)
{
  if(pos < PKT_BUF_SIZE) {
    pkt_buf[pos++] = *data;
  }
  return PLIP_STATUS_OK;
}

static u08 fill_tx(u08 *data)
{
  if(pos < PKT_BUF_SIZE) {
    *data = pkt_buf[pos++];
  }
  return PLIP_STATUS_OK;
}

static u08 end_rx(plip_packet_t *pkt)
{
  return PLIP_STATUS_OK;
}

void ping_plip_loop(void)
{
  plip_recv_init(begin_rx, fill_rx, end_rx);
  plip_send_init(fill_tx);
  ser_parse_set_data_func(0); // use serial echo

  stats_reset();
  stats_reset_timing();
  
  led_green_on(); 
  while(param.mode == PARAM_MODE_PING_PLIP) {
    ser_parse_worker();
    
    u08 status = plip_recv(&pkt);
    if(status != PLIP_STATUS_IDLE) {
      led_green_off();

      if(status == PLIP_STATUS_OK) {
        stats.pkt_rx_cnt++;
        
        // is a ping packet?
        if(ip_icmp_is_ping_request(pkt_buf)) {
          u16 pkt_size = ip_hdr_get_size(pkt_buf);
          stats.pkt_rx_bytes += pkt_size;
          
          // make reply
          ip_icmp_ping_request_to_reply(pkt_buf);

          // send reply
          pos = 0;
          status = plip_send(&pkt);
          if(status == PLIP_STATUS_CANT_SEND) {
            stats.pkt_last_tx_err = status;
            stats.pkt_tx_err++;
            uart_send('C');
          } else if(status != PLIP_STATUS_OK) {
            stats.pkt_last_tx_err = status;
            stats.pkt_tx_err++;
            uart_send('T');
          } else {
            stats.pkt_tx_cnt ++;
            stats.pkt_tx_bytes += pkt_size;
          }
          
        } else {
          // no ICMP request
          uart_send('?');
          uart_send_hex_byte_spc(pkt_buf[0]);
          uart_send_hex_byte_spc(pkt_buf[9]);
          uart_send_hex_byte_spc(pkt_buf[20]);
        }
      } else {
        stats.pkt_rx_err++;
        stats.pkt_last_rx_err = status;
        uart_send('R');
      }

      // give summary
      if(stats.pkt_rx_cnt == 256) {
        u08 err = (stats.pkt_tx_err > 0) || (stats.pkt_rx_err > 0);
        if(err) {
          led_red_on();
        } else {
          led_red_off();
        }
        
        stats_capture_timing();

        uart_send_crlf();
        stats_dump();

        stats_reset_timing();
      }

      led_green_on();
    }
  }
  led_green_off();
}


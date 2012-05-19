/*
 * main.c - main loop
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

#include "global.h"
#include "board.h"
#include "uart.h"
#include "timer.h"
#include "par_low.h"
#include "param.h"
#include "stats.h"
#include "ser_parse.h"
#include "cmd.h"
#include "slip.h"
#include "uartutil.h"
#include "stats.h"
#include "log.h"
#include "error.h"
	 
   // eth
#include "enc28j60.h"
#include "arp.h"
#include "eth.h"
#include "net.h"
#include "pkt_buf.h"
#include "uartutil.h"

#include "transfer.h"
#include "ping_plip.h"
#include "ping_slip.h"
#include "only_plip_rx.h"
#include "only_slip_rx.h"

const u08 mac[6] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };
const u08 ip[4] = { 192, 168, 2, 133 };

int main (void){
  // board init. e.g. switch off watchdog, init led
  board_init();  
  // setup timer
  timer_init();
  // setup serial
  uart_init();
  // setup par
  par_low_init();
  // param init
  param_init();
  
  // setup command handler
  ser_parse_set_cmd_func(cmd_parse);
  
  // enable uart
  uart_start_reception();

  // send welcome
#ifdef HAVE_SLIP
  // encapsulated into an invalid SLIP packet to not disturb an attached SLIP
  slip_send_end();
  uart_send_string("plip2slip: ");
  uart_send_hex_byte_crlf(param.mode);
  uart_send_hex_byte_crlf(rev);
  slip_send_end();
  
  // main loop
  while(1) {
    // reset stats & log
    stats_reset();
    log_init();
    error_init();
    
    // now enter mode loop
    switch(param.mode) {
      case PARAM_MODE_TRANSFER:
        transfer_loop();
        break;
      case PARAM_MODE_PING_PLIP:
        ping_plip_loop();
        break;
      case PARAM_MODE_PING_SLIP:
        ping_slip_loop();
        break;
      case PARAM_MODE_ONLY_PLIP_RX:
        only_plip_rx_loop();
        break;
      case PARAM_MODE_ONLY_SLIP_RX:
        only_slip_rx_loop();
        break;
    }
  }
#else
  uart_send_string("pli2eth: ");
  u08 rev = enc28j60_init(mac);
  uart_send_hex_byte_crlf(rev);

  net_init(mac, ip);

  while(1) {
    // get next packet
	  u16 len = enc28j60_packet_receive(pkt_buf, PKT_BUF_SIZE);
    if(len > 0) {
      // show length and dump eth header
      uart_send_hex_word_spc(len);
      eth_dump(pkt_buf);

      // inspect frame
      u08 *frame = pkt_buf + ETH_HDR_SIZE;
      u16 flen = len - ETH_HDR_SIZE;
      if(arp_is_ipv4(frame,flen)) {
        arp_dump(frame);
        if(arp_is_req_for_me(frame)) {
          uart_send_string("ME! ");
          eth_make_reply(pkt_buf);
          arp_make_reply(frame);
          enc28j60_packet_send(pkt_buf, len);
        }
      }
      uart_send_crlf();      
    }
  }
#endif

  return 0;
} 

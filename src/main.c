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

#include <avr/delay.h>

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
#include "icmp.h"
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
const u08 gw[4] = { 192, 168, 2, 1 };
const u08 nm[4] = { 255, 255, 255, 0 };

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
  /* ----- network stuff ----- */
  uart_send_string("plip2eth: ");
  
  /* init ethernet controller */
  u08 rev = enc28j60_init(mac);
  uart_send_hex_byte_crlf(rev);

  /* setup network addressing */
  net_init(mac, ip, gw, nm);
  
  /* wait for link up */
  for(int i = 0 ; i<10;i++) {
    if(enc28j60_is_link_up()) {
      break;
    }
    _delay_ms(250);
    uart_send('.');
  }
  uart_send_crlf();
  
  /* init ARP: send query for GW MAC */
  arp_init(pkt_buf, PKT_BUF_SIZE);

  while(1) {
    // get next packet
	  u16 len = enc28j60_packet_receive(pkt_buf, PKT_BUF_SIZE);
    if(len > 0) {

#define DUMP_ETH_HDR
#ifdef DUMP_ETH_HDR
      // show length and dump eth header
      uart_send_hex_word_spc(len);
      eth_dump(pkt_buf);
      uart_send_crlf();
#endif

      // handle ARP packets
      if(!arp_handle_packet(pkt_buf,len)) {
        // IPv4
        if(eth_is_ipv4_pkt(pkt_buf)) {
          u08 *ip_buf = pkt_buf + ETH_HDR_SIZE;
          //u16 ip_size = len - ETH_HDR_SIZE;
          // reply ping
          if(icmp_is_ping_request(ip_buf)) {
            uart_send_string("PING!");
            uart_send_crlf();
            
            icmp_ping_request_to_reply(ip_buf);
            eth_make_reply(pkt_buf);
            enc28j60_packet_send(pkt_buf, len);
          }
          
        }
      }
    }
  }
#endif

  return 0;
} 

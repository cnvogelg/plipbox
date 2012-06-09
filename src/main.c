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
#include "spi.h"
#include "enc28j60.h"
#include "uartutil.h"
#include "ping.h"

#include "net/net.h"
#include "net/udp.h"
#include "net/arp.h"
#include "net/eth.h"

#include "eth_rx.h"
#include "plip_rx.h"
#include "plip_tx.h"
#include "eth_state.h"
#include "plip_state.h"

#include "pkt_buf.h"

static void init_hw(void)
{
  // board init. e.g. switch off watchdog
  board_init();  
  // setup timer
  timer_init();
  // setup serial
  uart_init();
  // setup par
  par_low_init();
  // spi init
  spi_init();
}

static void init_eth(void)
{  
  uart_send_pstring(PSTR("eth rev: "));
  u08 rev = enc28j60_init(net_get_mac());
  uart_send_hex_byte_crlf(rev);
  // if no ethernet controller found then panic
  if(rev == 0) {
    uart_send_pstring(PSTR("NOT FOUND! PANIC!!\r\n"));
    // can't proceed :(
    while(1) {}
  }  
}

const u08 box_ip[4] = { 192,168,2,20 }; // my box

int main (void)
{
  init_hw();
  
  // send welcome
  uart_send_pstring(PSTR("\r\nWelcome to plipbox " VERSION "\r\n"));
  uart_send_pstring(PSTR("by lallafa (http://www.lallafa.de/blog)\r\n\r\n"));
  
  // param init
  param_init();  
  param_dump();
  uart_send_crlf();

  // init ethernet controller
  init_eth();
  
  eth_rx_init();
  plip_rx_init();
  plip_tx_init();
  
  // main loop
  while(1) {
    u08 eth_state = eth_state_worker();
    u08 plip_state = plip_state_worker();
    u08 eth_online = (eth_state == ETH_STATE_ONLINE);
    u08 plip_online = (plip_state == PLIP_STATE_ONLINE);
    
    eth_rx_worker(eth_state, plip_online);
    plip_rx_worker(plip_state, eth_online);
    
    // small hack to enter commands
    if(uart_read_data_available()) {
      u08 cmd = uart_read();
      switch(cmd) {
        case ' ':
        {
          const u08 ip[4] = { 173,194,35,159 }; // google.de
          u08 result = ping_eth_send_request(ip);
          uart_send_hex_byte_crlf(result);
          break;
        }
        case 'p':
        {
          u08 result = ping_eth_send_request(box_ip);
          uart_send_hex_byte_crlf(result);
          break;
        }
        case 'a':
          ping_plip_send_request(net_get_p2p_amiga());
          break;
        case 'u': // send test UDP packet
        {
          const u08 *mac = arp_find_mac(pkt_buf, box_ip, enc28j60_packet_tx);
          if(mac != 0) {
            u08 *buf = pkt_buf + ETH_HDR_SIZE;
            u08 off = udp_begin_pkt(buf, net_get_ip(), 42, box_ip, 6800);
            buf[off] = 'C';
            buf[off+1] = 'V';
            udp_finish_pkt(buf, 2);
            eth_make_to_tgt(pkt_buf, ETH_TYPE_IPV4, mac);
            u16 size = off + 2 + ETH_HDR_SIZE;
            enc28j60_packet_tx(pkt_buf, size);
            uart_send_pstring(PSTR("UDP!\r\n"));
          }
          break;
        }
      }
    }
    
  }

  return 0;
} 

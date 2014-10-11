/*
 * param.c - handle device parameters
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

#include "param.h"
#include "uartutil.h"
#include "uart.h"
#include "net/net.h"

#include <avr/eeprom.h>
#include <util/crc16.h>
#include <avr/pgmspace.h>
#include <string.h>

// current memory RAM param
param_t param;

// eeprom param
param_t eeprom_param EEMEM;
uint16_t eeprom_crc16 EEMEM;

// default 
static const param_t PROGMEM default_param = {
  .mac_addr = { 0x1a,0x11,0xaf,0xa0,0x47,0x11},

  .dump_dirs = 0,
  .dump_eth = 0,
  .dump_ip = 0,
  .dump_arp = 0,
  .dump_proto = 0,
  .dump_plip = 0,
  
  .filter_plip = 1,
  .filter_eth = 1,
  .flow_ctl = 1,
  .full_duplex = 0,
  .loop_back = 0,
  
  .log_all = 0
};

// dump all params
void param_dump(void)
{
  // mac address
  uart_send_pstring(PSTR("m: mac address   "));
  net_dump_mac(param.mac_addr);
  uart_send_crlf();

  uart_send_crlf();

  // options
  uart_send_pstring(PSTR("fd: full duplex  "));
  uart_send_hex_byte(param.full_duplex);
  uart_send_crlf();

  uart_send_pstring(PSTR("fl: loop back    "));
  uart_send_hex_byte(param.loop_back);
  uart_send_crlf();

  uart_send_pstring(PSTR("fc: ETH flow ctl "));
  uart_send_hex_byte(param.flow_ctl);
  uart_send_crlf();

  uart_send_pstring(PSTR("fe: filter ETH   "));
  uart_send_hex_byte(param.filter_eth);
  uart_send_crlf();
  
  uart_send_pstring(PSTR("fp: filter PLIP  "));
  uart_send_hex_byte(param.filter_plip);
  uart_send_crlf();
  
  // diagnosis
  uart_send_crlf();
  
  uart_send_pstring(PSTR("dd: dump dirs    "));
  uart_send_hex_byte(param.dump_dirs);
  uart_send_crlf();

  uart_send_pstring(PSTR("de: dump ETH     "));
  uart_send_hex_byte(param.dump_eth);
  uart_send_crlf();

  uart_send_pstring(PSTR("di: dump IP      "));
  uart_send_hex_byte(param.dump_ip);
  uart_send_crlf();

  uart_send_pstring(PSTR("da: dump ARP     "));
  uart_send_hex_byte(param.dump_arp);
  uart_send_crlf();
  
  uart_send_pstring(PSTR("dp: dump proto   "));
  uart_send_hex_byte(param.dump_proto);
  uart_send_crlf();

  uart_send_pstring(PSTR("dl: dump plip    "));
  uart_send_hex_byte(param.dump_plip);
  uart_send_crlf();
  
  // log
  uart_send_crlf();
  
  uart_send_pstring(PSTR("la: log all cmds "));
  uart_send_hex_byte(param.log_all);
  uart_send_crlf();
}

// build check sum for parameter block
static uint16_t calc_crc16(param_t *p)
{
  uint16_t crc16 = 0xffff;
  u08 *data = (u08 *)p;
  for(u16 i=0;i<sizeof(param_t);i++) {
    crc16 = _crc16_update(crc16,*data);
    data++;
  }
  return crc16;
}

u08 param_save(void)
{
  // check that eeprom is writable
  if(!eeprom_is_ready())
    return PARAM_EEPROM_NOT_READY;

  // write current param to eeprom
  eeprom_write_block(&param,&eeprom_param,sizeof(param_t));

  // calc current parameter crc
  uint16_t crc16 = calc_crc16(&param);
  eeprom_write_word(&eeprom_crc16,crc16);

  return PARAM_OK;
}

u08 param_load(void)
{
  // check that eeprom is readable
  if(!eeprom_is_ready())
    return PARAM_EEPROM_NOT_READY;
  
  // read param
  eeprom_read_block(&param,&eeprom_param,sizeof(param_t));
  
  // read crc16
  uint16_t crc16 = eeprom_read_word(&eeprom_crc16);
  uint16_t my_crc16 = calc_crc16(&param);
  if(crc16 != my_crc16) {
    param_reset();
    return PARAM_EEPROM_CRC_MISMATCH;
  }
  
  return PARAM_OK;
}

void param_reset(void)
{
  // restore default param
  u08 *out = (u08 *)&param;
  const u08 *in = (const u08 *)&default_param;
  for(u08 i=0;i<sizeof(param_t);i++) {
    *(out++) = pgm_read_byte_near(in++);
  }
}

void param_init(void)
{
  if(param_load()!=PARAM_OK)
    param_reset();
}

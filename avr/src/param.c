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
static param_t param;

// eeprom param
param_t eeprom_param EEMEM;
uint16_t eeprom_crc16 EEMEM;

// default 
static const param_t PROGMEM default_param = {
  .mac_addr = { 0x1a,0x11,0xaf,0xa0,0x47,0x11},
  .mode = 0,
  .flags = 0
};

static void dump_byte(PGM_P str, const u08 val)
{
  uart_send_pstring(str);
  uart_send_hex_byte(val);
  uart_send_crlf();  
}

#if 0
static void dump_word(PGM_P str, const u16 val)
{
  uart_send_pstring(str);
  uart_send_hex_word(val);
  uart_send_crlf();    
}
#endif

// dump all params
void param_dump(void)
{
  // mac address
  uart_send_pstring(PSTR("m: mac address   "));
  net_dump_mac(param.mac_addr);
  uart_send_crlf();

  // options
  dump_byte(PSTR("o: mode      "), param.mode);
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

void param_set_mode(u08 mode)
{
  param.mode = mode;
}

u08 param_get_mode(void)
{
  return param.mode;
}

void param_set_flags(u08 flags)
{
  param.flags = flags;
}

u08 param_get_flags(void)
{
  return param.flags;
}

void param_get_def_mac(mac_t mac)
{
  // restore default param
  u08 *out = mac;
  const u08 *in = default_param.mac_addr;
  for(u08 i=0;i<sizeof(mac_t);i++) {
    *(out++) = pgm_read_byte_near(in++);
  }
}

void param_get_mac(mac_t mac)
{
  for(u08 i=0;i<sizeof(mac_t);i++) {
    mac[i] = param.mac_addr[i];
  }
}

void param_set_mac(const mac_t mac)
{
  for(u08 i=0;i<sizeof(mac_t);i++) {
    param.mac_addr[i] = mac[i];
  }
}

void param_init(void)
{
  if(param_load()!=PARAM_OK)
    param_reset();
}

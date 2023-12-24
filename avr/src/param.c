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

#include "arch.h"
#include "param.h"
#include "uartutil.h"
#include "uart.h"
#include "net/net.h"
#include "proto_cmd_shared.h"
#include "param_shared.h"
#include "param_def.h"

#ifdef DEBUG_PARAM
#define DEBUG
#endif

#include "debug.h"

#include <util/crc16.h>
#include <string.h>

// eeprom param
static param_t EEPROM_ATTR eeprom_param;

static void dump_tag(u32 tag)
{
  u32 shift = 24;
  for(int i=0;i<4;i++) {
    u08 val = (tag >> shift) & 0xff;
    if(val != 0) {
      uart_send(val);
    } else {
      uart_send(' ');
    }
    shift -= 8;
  }
}

static void dump_byte_array(const u08 *data, u16 size)
{
  for(u16 i=0; i<size; i++) {
    if(i!=0) {
      uart_send(':');
    }
    uart_send_hex_byte(*data);
    data++;
  }
}

static void dump_word_array(const u16 *data, u16 size)
{
  for(u16 i=0; i<size; i++) {
    if(i!=0) {
      uart_send(':');
    }
    uart_send_hex_word(*data);
    data++;
  }
}

static void dump_long_array(const u32 *data, u16 size)
{
  for(u16 i=0; i<size; i++) {
    if(i!=0) {
      uart_send(':');
    }
    uart_send_hex_dword(*data);
    data++;
  }
}

static void dump_data(const u08 *data, u16 size, u08 type, u08 format)
{
  switch(type) {
    case PARAM_TYPE_WORD: {
      u16 *ptr = (u16 *)data;
      uart_send_hex_word(*ptr);
      break;
    }
    case PARAM_TYPE_LONG: {
      u32 *ptr = (u32 *)data;
      uart_send_hex_dword(*ptr);
      break;
    }
    case PARAM_TYPE_BYTE_ARRAY:
      dump_byte_array(data, size);
      break;
    case PARAM_TYPE_WORD_ARRAY:
      dump_word_array((const u16 *)data, size >> 1);
      break;
    case PARAM_TYPE_LONG_ARRAY:
      dump_long_array((const u32 *)data, size >> 2);
      break;
  }
}

static u16 calc_data_size(u16 size, u08 type, u08 format)
{
  switch(type) {
    case PARAM_TYPE_WORD:
      return 4;
    case PARAM_TYPE_LONG:
      return 8;
    case PARAM_TYPE_BYTE_ARRAY:
      return size * 3 - 1;
    case PARAM_TYPE_WORD_ARRAY:
      return size * 5 - 1;
    case PARAM_TYPE_LONG_ARRAY:
      return size * 9 - 1;
    default:
      return 0;
  }
}

static u16 calc_max_data_size(void)
{
  const param_def_t *def = param_defs;
  u16 max_size = 0;
  for(int i=0;i<param_defs_size;i++) {
    u16 size = read_rom_word(&def->size);
    u08 type = read_rom_char(&def->type);
    u08 format = read_rom_char(&def->format);
    u16 data_size = calc_data_size(size, type, format);
    if(data_size > max_size) {
      max_size = data_size;
    }
    def++;
  }
  return max_size;
}

// dump all params
void param_dump(void)
{
  u16 max_data_size = calc_max_data_size();

  const param_def_t *def = param_defs;
  for(int i=0;i<param_defs_size;i++) {
    // index
    uart_send('#');
    u08 index = read_rom_char(&def->index);
    uart_send_hex_byte(index);
    uart_send(' ');

    // dump tag
    u32 tag = read_rom_long(&def->tag);
    dump_tag(tag);
    uart_send(' ');

    // size
    u16 size = read_rom_word(&def->size);
    uart_send('[');
    uart_send_hex_word(size);
    uart_send(']');
    uart_send(' ');

    // value
    u08 *data = (u08 *)read_rom_ram_ptr(&def->data);
    u08 type = read_rom_char(&def->type);
    u08 format = read_rom_char(&def->format);
    dump_data(data, size, type, format);
    u16 data_size = calc_data_size(size, type, format);

    // pad value
    if(data_size < max_data_size) {
      u16 delta = max_data_size - data_size;
      for(u16 i=0;i<delta;i++) {
        uart_send(' ');
      }
    }

    // desc
    uart_send(' ');
    uart_send(' ');
    rom_pchar desc = read_rom_rom_ptr(&def->desc);
    uart_send_pstring(desc);

    uart_send_crlf();
    def++;
  }
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

  // calc check sum
  param.crc = 0;
  // calc current parameter crc
  param.crc = calc_crc16(&param);

  // write current param to eeprom
  eeprom_write_block(&param,&eeprom_param,sizeof(param_t));

  return PARAM_OK;
}

u08 param_load(void)
{
  // check that eeprom is readable
  if(!eeprom_is_ready())
    return PARAM_EEPROM_NOT_READY;
  
  // read param
  eeprom_read_block(&param,&eeprom_param,sizeof(param_t));

  // get stored checksum
  u16 got_crc = param.crc;

  // recalc
  param.crc = 0;
  uint16_t calc_crc = calc_crc16(&param);
  if(got_crc != calc_crc) {
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
  rom_copy(in, out, sizeof(param_t));
}

void param_get_def_mac(mac_t mac)
{
  // restore default param
  u08 *out = mac;
  const u08 *in = default_param.mac_addr;
  rom_copy(in, out, sizeof(mac_t));
}

void param_get_cur_mac(mac_t mac)
{
  for(u08 i=0;i<sizeof(mac_t);i++) {
    mac[i] = param.mac_addr[i];
  }
}

void param_set_cur_mac(mac_t mac)
{
  for(u08 i=0;i<sizeof(mac_t);i++) {
    param.mac_addr[i] = mac[i];
  }
}

u08 param_init(void)
{
  u08 res = param_load();
  if(res != PARAM_OK) {
    param_reset();
    param_save();
  }
  return res;
}

u08 param_get_num(void)
{
  return param_defs_size;
}

u08 param_find_tag(u32 tag)
{
  u08 id = 0;
  while(id < param_defs_size) {
    const u32 *ptr = &param_defs[id].tag;
    u32 def_tag = read_rom_long(ptr);
    if(def_tag == tag) {
      return id;
    }
    id++;
  }
  return PARAM_ID_INVALID;
}

void param_get_def(u08 index, param_def_t *ret_def)
{
  if(index >= param_defs_size) {
    return;
  }

  const param_def_t *ptr = &param_defs[index];
  rom_copy((const u08 *)ptr, (u08 *)ret_def, sizeof(param_def_t));
}

u08 *param_get_data(u08 index)
{
  if(index >= param_defs_size) {
    return NULL;
  }

  u08 *data = (u08 *)read_rom_ram_ptr(&param_defs[index].data);
  return data;
}



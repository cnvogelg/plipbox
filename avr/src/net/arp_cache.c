/*
* arp_cache.c - manage the ARP cache
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

#include "net.h"
#include "eth.h"
#include "arp_cache.h"

#include "uart.h"
#include "uartutil.h"

#define DUMP_ARP

#define ARP_STATE_IDLE    0
#define ARP_STATE_LOOKUP  1
#define ARP_STATE_WAITING 2
#define ARP_STATE_VALID   3
 
#define ARP_CACHE_INVALID 0xff
 
struct arp_cache_s {
  u08 state;
  u08 usage;
  u08 mac[6];
};

static struct arp_cache_s arp_cache[ARP_CACHE_SIZE];

// ----- helpers -----

static u08 *get_ip(u08 idx)
{
  // index 0 is gateway
  if(idx == 0) {
    return param.ip_gw_addr;
  } else {
    return param.arp_ip[idx-1];
  }
}

static u08 find_ip(const u08 *ip)
{
 for(int i=0;i<ARP_CACHE_SIZE;i++) {
   if(net_compare_ip(ip, get_ip(i))) {
     return i;
   }
 }
 return ARP_CACHE_INVALID;
}

static u08 find_low_usage(u08 state)
{
  u08 min_usage = 0xff;
  u08 min_pos = ARP_CACHE_INVALID;
  for(int i=1;i<ARP_CACHE_SIZE;i++) {
    struct arp_cache_s *ac = &arp_cache[i];
    if(ac->state == state) {
      u08 usage = ac->usage;
      if(usage < min_usage) {
        min_usage = usage;
        min_pos = i;
      }
    }
  }
  return min_pos;
}

static u08 find_of_state(u08 state)
{
  for(int i=1;i<ARP_CACHE_SIZE;i++) {
    struct arp_cache_s *ac = &arp_cache[i];
    if(ac->state == state) {
      return i;
    }
  }
  return ARP_CACHE_INVALID;
}

static u08 find_next(void)
{
  // find a free one
  u08 pos = find_of_state(ARP_STATE_IDLE);
  if(pos != ARP_CACHE_INVALID) {
    return pos;
  }
  
  // find a low usage valid one
  pos = find_low_usage(ARP_STATE_VALID);
  if(pos != ARP_CACHE_INVALID) {
    return pos;
  }
  
  // find any of type
  pos = find_of_state(ARP_STATE_VALID);
  if(pos != ARP_CACHE_INVALID) {
    return pos;
  }
  
  // find a waiting
  pos = find_of_state(ARP_STATE_WAITING);
  if(pos != ARP_CACHE_INVALID) {
    return pos;
  }

  // find a look up
  pos = find_of_state(ARP_STATE_LOOKUP);
  if(pos != ARP_CACHE_INVALID) {
    return pos;
  }
  
  // last resort
  return 1;
}

// ----- API -----

void arp_cache_init(void)
{
  // initial arp cache state
  for(u08 i=0;i<ARP_CACHE_SIZE;i++) {
    if(!net_compare_ip(net_zero_ip, get_ip(i))) {
      arp_cache[i].state = ARP_STATE_LOOKUP;
    } else {
      arp_cache[i].state = ARP_STATE_IDLE;
    }
    arp_cache[i].usage = 0;
  }
}

void arp_cache_clear(void)
{
  arp_cache[0].state = ARP_STATE_LOOKUP;
  arp_cache[0].usage = 0;
  net_copy_mac(net_zero_mac, arp_cache[0].mac);
  
  for(u08 i=1;i<ARP_CACHE_SIZE;i++) {
    arp_cache[i].state = ARP_STATE_IDLE;
    arp_cache[i].usage = 0;
    net_copy_mac(net_zero_mac, arp_cache[i].mac);
    net_copy_ip(net_zero_ip, get_ip(i));
  }
}

u08 arp_cache_handle_packet(u08 *ethbuf, u16 ethlen, net_tx_packet_func tx_func)
{
  u08 *buf = ethbuf + ETH_HDR_SIZE;
  u16 len  = ethlen - ETH_HDR_SIZE;
  
  /* return if its no IPv4 ARP packet */
  if(!arp_is_ipv4(buf, len)) {
    return 0;
  }
  
  /* extract IP + MAC of reply */
  const u08 *src_ip = buf + ARP_OFF_SRC_IP;
  const u08 *src_mac = buf + ARP_OFF_SRC_MAC;
  
  /* is someone asking for our MAC? */
  if(arp_is_req_for_me(buf)) {
    
#ifdef DUMP_DETAIL
    arp_dump(buf);
    uart_send_pstring(PSTR("ME! "));
    uart_send_crlf();
#endif
#ifdef DUMP_ARP
    uart_send_pstring(PSTR("arp: asking for me "));
    net_dump_ip(src_ip);
    uart_send_crlf();
#endif
    
    /* send a reply with my MAC+IP */
    eth_make_reply(ethbuf);
    arp_make_reply(buf);
    tx_func(ethbuf, ethlen);
    return 1;
  }
  
  /* is it a reply for a request of me? */
  else if(arp_is_reply_for_me(buf)) {

#ifdef DUMP_DETAIL
    arp_dump(buf);
    uart_send_crlf();
#endif
#ifdef DUMP_ARP
    uart_send_pstring(PSTR("arp: reply from "));
    net_dump_ip(src_ip);
    uart_send_pstring(PSTR(": MAC="));
    net_dump_mac(src_mac);
#endif
    
    /* check the cache we need to fill */
    u08 idx = find_ip(src_ip);
    if(idx == ARP_CACHE_INVALID) {
#ifdef DUMP_ARP
      uart_send_pstring(PSTR(" -> not in cache?!\r\n"));
#endif
    }
    else {
#ifdef DUMP_ARP
      uart_send_pstring(PSTR(" -> cache #"));
      uart_send_hex_byte_crlf(idx);
#endif
      /* store in cache */
      struct arp_cache_s *ac = &arp_cache[idx];
      net_copy_mac(src_mac, ac->mac);
      if(ac->usage < 0xff) {
        ac->usage ++;
      }
      ac->state = ARP_STATE_VALID;
    }
    return 1;
  }
  else {
#ifdef DUMP_ARP
    //uart_send_pstring(PSTR("arp: ??\r\n"));
#endif
    return 0;
  }
}

void arp_cache_worker(u08 *buf, net_tx_packet_func tx_func)
{
  // look through cache and see if one entry needs a lookup
  for(u08 i=0;i<ARP_CACHE_SIZE;i++) {
    struct arp_cache_s *ac = &arp_cache[i];
    if(ac->state == ARP_STATE_LOOKUP) {
#ifdef DUMP_ARP
      uart_send_pstring(PSTR("arp: cache do req #"));
      uart_send_hex_byte_crlf(i);
#endif
      // send a request
      arp_send_request(buf, get_ip(i), tx_func);     
      ac->state = ARP_STATE_WAITING;
    }
  }
}

const u08 *arp_cache_find_mac(const u08 *ip)
{
  // if IP addr is not on my subnet then resolve IP addr of gateway
  if(!net_is_my_subnet(ip)) {
    ip = net_get_gateway();
  }
  
  // look through cache 
  for(u08 i=0;i<ARP_CACHE_SIZE;i++) {
    struct arp_cache_s *ac = &arp_cache[i];
    if(ac->state != ARP_STATE_IDLE) {
      const u08 *ac_ip = get_ip(i);
      if(net_compare_ip(ac_ip, ip)) {
        // increase usage
        if(ac->usage < 0xff) {
          ac->usage++;
        }
        // found IP! -> if mac is already valid then return it
        if(ac->state == ARP_STATE_VALID) {
          return ac->mac;
        }
        // not ready yet -> have to wait
        else {
          return 0; 
        }
      }
    }
  }
  
  // nothing found in cache :( -> need to find a place in the cache
  u08 idx = find_next();
#ifdef DUMP_ARP
  uart_send_pstring(PSTR("arp: cache overwrite #"));
  uart_send_hex_byte_crlf(idx);
#endif
  struct arp_cache_s *nac = &arp_cache[idx];
  nac->state = ARP_STATE_LOOKUP;
  nac->usage = 1;
  net_copy_ip(ip, get_ip(idx));
  return 0;
}

void arp_cache_dump(void)
{
  uart_send_pstring(PSTR("ARP cache\r\n"));
  for(u08 i=0;i<ARP_CACHE_SIZE;i++) {
    struct arp_cache_s *ac = &arp_cache[i];
    net_dump_ip(get_ip(i));
    uart_send(' ');
    net_dump_mac(ac->mac);
    uart_send(' ');
    uart_send_hex_byte_spc(ac->state);
    uart_send_hex_byte_crlf(ac->usage);
  }
}


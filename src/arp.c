/*
 * arp.c - handle ARP protocol
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

#include "arp.h"
#include "net.h"
#include "eth.h"
#include "uartutil.h"
#include "enc28j60.h"

#define ARP_OFF_HW_TYPE   0
#define ARP_OFF_PROT_TYPE 2
#define ARP_OFF_HW_SIZE   4
#define ARP_OFF_PROT_SIZE 5
#define ARP_OFF_OP        6
#define ARP_OFF_SRC_MAC   8
#define ARP_OFF_SRC_IP    14
#define ARP_OFF_TGT_MAC   18
#define ARP_OFF_TGT_IP    24

#define ARP_SIZE 28

#define ARP_REQUEST   1
#define ARP_REPLY     2

   /* debug */
#define DUMP_ARP

#define ARP_CACHE_SIZE  4
 
struct arp_cache_s {
  u08 ip[4];
  u08 mac[6];
};

   /* ARP cache */
static u08 gw_mac[6];
static u08 has_gw_mac;

static struct arp_cache_s arp_cache[ARP_CACHE_SIZE];
static u08 arp_cache_total;
static u08 arp_cache_pos;

const u08 *arp_get_gw_mac(void)
{
  if(!has_gw_mac) {
    return 0;
  }
  return gw_mac;
}

u08 arp_is_ipv4(const u08 *buf, u16 len)
{
  if(len < ARP_SIZE) {
    return 0;
  }
  
  u16 hw_type = net_get_word(buf + ARP_OFF_HW_TYPE);
  u16 pt_type = net_get_word(buf + ARP_OFF_PROT_TYPE);
  u08 hw_size = buf[ARP_OFF_HW_SIZE];
  u08 pt_size = buf[ARP_OFF_PROT_SIZE];
  
  return (hw_type == 1) && (pt_type == 0x800) && (hw_size == 6) && (pt_size == 4);
}

u16 arp_get_op(const u08 *buf)
{
  return net_get_word(buf + ARP_OFF_OP);
}

u08 arp_is_req_for_me(const u08 *buf)
{
  return (arp_get_op(buf) == ARP_REQUEST) && net_compare_my_ip(buf + ARP_OFF_TGT_IP);
}

u08 arp_is_reply_for_me(const u08 *buf)
{
  return (arp_get_op(buf) == ARP_REPLY) && net_compare_my_ip(buf + ARP_OFF_TGT_IP);
}

void arp_make_reply(u08 *buf)
{
  net_put_word(buf + ARP_OFF_OP, ARP_REPLY);
  // copy old src to new tgt
  net_copy_mac(buf + ARP_OFF_SRC_MAC, buf + ARP_OFF_TGT_MAC);
  net_copy_ip(buf + ARP_OFF_SRC_IP, buf + ARP_OFF_TGT_IP);
  // set my own mac/ip as src
  net_copy_my_mac(buf + ARP_OFF_SRC_MAC);
  net_copy_my_ip(buf + ARP_OFF_SRC_IP);
}

u16 arp_make_request(u08 *buf, const u08 ip[4])
{
  // IPv4 ARP
  net_put_word(buf + ARP_OFF_HW_TYPE, 1);
  net_put_word(buf + ARP_OFF_PROT_TYPE, 0x800);
  buf[ARP_OFF_HW_SIZE] = 6;
  buf[ARP_OFF_PROT_SIZE] = 4;
  // request
  net_put_word(buf + ARP_OFF_OP, ARP_REQUEST);
  // me being the source
  net_copy_my_mac(buf + ARP_OFF_SRC_MAC);
  net_copy_my_ip(buf + ARP_OFF_SRC_IP);
  // tgt being the request host
  net_copy_zero_mac(buf + ARP_OFF_TGT_MAC);
  net_copy_ip(ip, buf + ARP_OFF_TGT_IP);
  return ARP_SIZE;
}

void arp_dump(const u08 *buf)
{
  uart_send_string("ARP:src_mac=");
  net_dump_mac(buf + ARP_OFF_SRC_MAC);
  uart_send_string(",src_ip=");
  net_dump_ip(buf + ARP_OFF_SRC_IP);
  uart_send_string(",tgt_mac=");
  net_dump_mac(buf + ARP_OFF_TGT_MAC);
  uart_send_string(",tgt_ip=");
  net_dump_ip(buf + ARP_OFF_TGT_IP);
  uart_send_string(",op=");
  u16 op = arp_get_op(buf);
  uart_send_hex_word_spc(op);
}

void arp_init(u08 *ethbuf, u16 maxlen)
{  
  // send arp request for gateway
  arp_send_request(ethbuf, net_get_gateway());
}

void arp_send_request(u08 *ethbuf, const u08 *ip)
{
  // build a request for gw ip
  eth_make_to_any(ethbuf, ETH_TYPE_ARP);
  u16 len = arp_make_request(ethbuf + ETH_HDR_SIZE, ip);
  len += ETH_HDR_SIZE;

#ifdef DUMP_ARP
  uart_send_string("arp:req ");
  net_dump_ip(ip);
  uart_send_crlf();
#endif
#ifdef DUMP_DETAIL
  uart_send_hex_word_spc(len);
  eth_dump(ethbuf);
  uart_send_crlf();
  arp_dump(ethbuf + ETH_HDR_SIZE);
  uart_send_crlf();
#endif
  
  enc28j60_packet_tx(ethbuf, len);  
}

u08 arp_handle_packet(u08 *ethbuf, u16 ethlen)
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
    uart_send_string("ME! ");
    uart_send_crlf();
#endif
#ifdef DUMP_ARP
    uart_send_string("arp:me: ");
    net_dump_ip(src_ip);
    uart_send_crlf();
#endif
    
    /* send a reply with my MAC+IP */
    eth_make_reply(ethbuf);
    arp_make_reply(buf);
    enc28j60_packet_tx(ethbuf, ethlen);
    return 1;
  }
  
  /* is it a reply for a request of me? */
  else if(arp_is_reply_for_me(buf)) {

#ifdef DUMP_DETAIL
    arp_dump(buf);
    uart_send_crlf();
#endif
#ifdef DUMP_ARP
    uart_send_string("arp:reply ");
    net_dump_ip(src_ip);
    uart_send_spc();
    net_dump_mac(src_mac);
#endif
    
    /* we got a reply for the gateway MAC -> keep in cache */    
    if(net_compare_ip(src_ip, net_get_gateway())) {
#ifdef DUMP_ARP
      uart_send_string(" -> GW");
      uart_send_crlf();
#endif
      net_copy_mac(src_mac, gw_mac);
      has_gw_mac = 1;
    }
    /* non-GW reply -> push into cache */
    else {
      u08 pos = arp_cache_find_ip(src_ip);
      if(pos == ARP_CACHE_INVALID) {
        /* add a new entry */
        pos = arp_cache_add(src_ip, src_mac);
      } else {
        /* update mac of entry */
        arp_cache_update(pos, src_mac);
      }
#ifdef DUMP_ARP
      uart_send_string(" -> cache ");
      uart_send_hex_byte_crlf(pos);
      uart_send_crlf();
#endif
    }
    return 1;
  }
  else {
    return 0;
  }
}

const u08 *arp_find_mac(u08 *buf, const u08 *ip)
{
  /* is GW itself? */
  if(net_compare_ip(ip, net_get_gateway())) {
    return arp_get_gw_mac();
  }
  
  /* is IP on my subnet? */
  u08 is_my_net = net_is_my_subnet(ip);
  const u08 *mac = 0;
  if(is_my_net) {
    /* yes its my subnet -> ask cache */
    u08 cache_pos = arp_cache_find_ip(ip);
    if(cache_pos != ARP_CACHE_INVALID) {
      mac = arp_cache_get_mac(cache_pos);
    } else {
      /* not found -> trigger an arp request */
      arp_send_request(buf, ip);     
    }
  } else {
    /* not my subnet -> use GW
       if GW mac is available then use it */
    mac = arp_get_gw_mac();
  }
  return mac;
}

/* ----- arp_cache ----- */

void arp_cache_init(void)
{
  // clear arp cache
  for(int i=0;i<ARP_CACHE_SIZE;i++) {
    net_copy_zero_ip(arp_cache[i].ip);
    net_copy_zero_mac(arp_cache[i].mac);
  }
  arp_cache_total = 0;
  arp_cache_pos = 0;
}

u08 *arp_cache_get_mac(u08 index)
{
  return arp_cache[index].mac;
}

u08 arp_cache_find_ip(const u08 *ip)
{
  for(int i=0;i<ARP_CACHE_SIZE;i++) {
    if(net_compare_ip(ip, arp_cache[i].ip)) {
      return i;
    }
  }
  return ARP_CACHE_INVALID;
}

u08 arp_cache_add(const u08 *ip, const u08 *mac)
{
  if(arp_cache_total < ARP_CACHE_SIZE) {
    arp_cache_total ++;
  }

  u08 i = arp_cache_pos;
  net_copy_ip(ip, arp_cache[i].ip);
  net_copy_mac(mac, arp_cache[i].mac);

  arp_cache_pos ++;
  if(arp_cache_pos == ARP_CACHE_SIZE) {
    arp_cache_pos = 0;
  }
  return i;
}

void arp_cache_update(u08 index, const u08 *mac)
{
  net_copy_mac(mac, arp_cache[index].mac);
}

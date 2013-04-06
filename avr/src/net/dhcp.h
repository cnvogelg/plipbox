/*
 * dhcp.h - handle DHCP messages
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

#ifndef DHCP_H
#define DHCP_H

#include "bootp.h"
  
#define DHCP_TYPE_DISCOVER    1
#define DHCP_TYPE_OFFER       2
#define DHCP_TYPE_REQUEST     3
#define DHCP_TYPE_DECLINE     4
#define DHCP_TYPE_ACK         5
#define DHCP_TYPE_NAK         6
#define DHCP_TYPE_RELEASE     7  
   
typedef void (*dhcp_handle_option_func)(u08 option, u08 size, const u08 *data);

extern u16 dhcp_begin_pkt(u08 *ip_buf, u08 cmd);
extern u16 dhcp_finish_pkt(u08 *ip_buf, u16 size);
extern void dhcp_make_request_from_offer_pkt(u08 *ip_buf);
extern u16 dhcp_make_renew_pkt(u08 *ip_buf, const u08 *srv_ip, const u08 *srv_id);

extern u16 dhcp_begin_eth_pkt_multicast(u08 *eth_buf, u08 cmd);
extern u16 dhcp_begin_eth_pkt_unicast(u08 *eth_buf, u08 cmd, const u08 *mac);
extern u16 dhcp_finish_eth_pkt(u08 *eth_buf, u16 size);
extern void dhcp_make_request_from_offer_eth_pkt(u08 *ip_buf);
extern u16 dhcp_make_renew_eth_pkt(u08 *eth_buf, const u08 *srv_mac, const u08 *srv_ip, const u08 *srv_id);
extern u16 dhcp_make_discover_eth_pkt(u08 *buf);

extern u08 *dhcp_add_type(u08 *opt_buf, u08 type);
extern u08 *dhcp_add_ip(u08 *opt_buf, u08 type, const u08 *ip);
extern u08 *dhcp_add_end(u08 *opt_buf);

extern u08 dhcp_is_dhcp_pkt(u08 *ip_buf);

inline const u08 *dhcp_get_options(const u08 *udp_buf) { return udp_buf + BOOTP_OFF_VEND + 4; }
inline u32 dhcp_get_server_id(const u08 *udp_buf) { return bootp_get_xid(udp_buf); }

extern void dhcp_parse_options(const u08 *opt_buf, dhcp_handle_option_func func);
extern void dhcp_dump_options(u08 option, u08 size, const u08 *data);
extern u08 dhcp_get_message_type(const u08 *opt_buf);
extern u08 dhcp_get_option_type(const u08 *opt_buf, u08 msg_type, const u08 **ptr);
extern u08 dhcp_set_message_type(u08 *opt_buf, u08 msg_type);

#endif

/*
 * dhcp.h - handle DHCP messages
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
   
extern u16 dhcp_begin_pkt(u08 *buf, u08 cmd);
extern u16 dhcp_finish_pkt(u08 *buf);

extern u16 dhcp_begin_eth_pkt(u08 *buf, u08 cmd);
extern u16 dhcp_finish_eth_pkt(u08 *buf);

extern u08 *dhcp_add_type(u08 *buf, u08 type);
extern u08 *dhcp_add_end(u08 *buf);

#endif

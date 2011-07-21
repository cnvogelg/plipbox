#ifndef IP_H
#define IP_H

#include "global.h"

// ----- IP Header -----
extern u08 ip_hdr_check(const u08 *buf);
extern void ip_hdr_calc_check(u08 *buf);

// ----- ICMP Ping -----
extern u08 ip_icmp_is_ping_request(const u08 *buf);
extern u08 ip_icmp_check(const u08 *buf);
extern void ip_icmp_calc_check(u08 *buf);
extern void ip_icmp_ping_request_to_reply(u08 *buf);

#endif
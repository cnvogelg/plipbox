#ifndef PKT_BUF_H
#define PKT_BUF_H

#include "global.h"
#include "plip.h"

#define PKT_BUF_SIZE    256

extern u08 pkt_buf[PKT_BUF_SIZE];
extern plip_packet_t pkt;

#endif

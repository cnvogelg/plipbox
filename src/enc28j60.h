// Microchip ENC28J60 Ethernet Interface Driver
// Author: Pascal Stang 
// Modified by: Guido Socher
// Modified by: Christian Vogelgsang (pure C code)
// Copyright: GPL V2
// 
// This driver provides initialization and transmit/receive
// functions for the Microchip ENC28J60 10Mb Ethernet Controller and PHY.
// This chip is novel in that it is a full MAC+PHY interface all in a 28-pin
// chip, using an SPI interface to the host processor.
//
// 2010-05-20 <jc@wippler.nl>

#ifndef ENC28J60_H
#define ENC28J60_H

#include <stdint.h>
#include "global.h"

extern uint8_t enc28j60_init(const uint8_t* macaddr);
extern uint8_t enc28j60_is_link_up ( void );
  
extern void enc28j60_packet_send (uint8_t *buffer, uint16_t len);
extern uint16_t enc28j60_packet_receive (uint8_t *buffer, uint16_t buffer_size);

extern void enc28j60_enable_broadcast( void );
extern void enc28j60_disable_broadcast( void );

extern void enc28j60_power_down( void );
extern void enc28j60_power_up( void );

extern uint8_t enc28j60_do_BIST ( void );

#endif

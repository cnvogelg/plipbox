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
#include "spi.h"

extern uint8_t enc28j60_init(const uint8_t* macaddr);
extern uint8_t enc28j60_is_link_up ( void );
  
extern void enc28j60_enable_broadcast( void );
extern void enc28j60_disable_broadcast( void );

extern void enc28j60_power_down( void );
extern void enc28j60_power_up( void );

extern uint8_t enc28j60_do_BIST ( void );

/* splitted send/receive */
extern u16  enc28j60_packet_rx(u08 *data, u16 max_size);
extern u08  enc28j60_packet_rx_num_waiting(void);
extern u16  enc28j60_packet_rx_begin(void);
extern void enc28j60_packet_rx_byte_begin(void);
inline u08  enc28j60_packet_rx_byte(void) { return spi_in(); }
extern void enc28j60_packet_rx_byte_end(void);
extern void enc28j60_packet_rx_blk(u08 *data, u16 size);
extern void enc28j60_packet_rx_end(void);

extern void enc28j60_packet_tx(const u08 *data, u16 size);
extern void enc28j60_packet_tx_prepare(void);
extern void enc28j60_packet_tx_begin_range(u16 offset);
inline void enc28j60_packet_tx_byte(u08 data) { spi_out(data); }
extern void enc28j60_packet_tx_blk(const u08 *data, u16 size);
extern void enc28j60_packet_tx_end_range(void);
extern void enc28j60_packet_tx_send(u16 len);

#endif

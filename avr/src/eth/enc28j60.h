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

extern uint8_t enc28j60_init( u08 full_duplex );

extern void enc28j60_start(const uint8_t* macaddr);
extern void enc28j60_stop( void );

extern uint8_t enc28j60_is_link_up ( void );
  
extern void enc28j60_enable_broadcast( void );
extern void enc28j60_disable_broadcast( void );

extern void enc28j60_power_down( void );
extern void enc28j60_power_up( void );

extern uint8_t enc28j60_do_BIST ( void );

extern void enc28j60_flow_control( u08 on );

#define ENC28J60_TX_ERR  2
#define ENC28J60_RX_ERR  1

extern u08 enc28j60_get_status( void );

/* splitted send/receive */
extern u16  enc28j60_packet_rx(u08 *data, u16 max_size);
extern u08  enc28j60_packet_rx_num_waiting(void);
extern u16  enc28j60_packet_rx_begin(void);
extern void enc28j60_packet_rx_data_begin(void);
inline u08  enc28j60_packet_rx_byte(void) { return spi_in(); }
extern void enc28j60_packet_rx_data_end(void);
extern void enc28j60_packet_rx_blk(u08 *data, u16 size);
extern void enc28j60_packet_rx_end(void);

extern void enc28j60_packet_tx(const u08 *data, u16 size);
extern void enc28j60_packet_tx_begin(void);
inline void enc28j60_packet_tx_byte(u08 data) { spi_out(data); }
extern void enc28j60_packet_tx_blk(const u08 *data, u16 size);
extern void enc28j60_packet_tx_end(void);
extern void enc28j60_packet_tx_send(u16 len);

#endif

/*
  mode_cmd.c - implement the proto_cmd_api for module operation
*/

#include "types.h"

#ifdef DEBUG_MODE
#define DEBUG
#endif

#include "debug.h"
#include "mode.h"
#include "mode_mod.h"
#include "uart.h"
#include "uartutil.h"
#include "proto_cmd.h"

u16 proto_cmd_api_get_version(void)
{
  return VERSION_MAJ << 8 | VERSION_MIN;
}

void proto_cmd_api_attach(void)
{
  uart_send_time_stamp_spc();
  uart_send_pstring(PSTR("proto: attach"));
  uart_send_crlf();

  mode_attach();
}

void proto_cmd_api_detach(void)
{
  uart_send_time_stamp_spc();
  uart_send_pstring(PSTR("proto: detach"));
  uart_send_crlf();

  mode_detach();
}

// async poll of status
u16 proto_cmd_api_get_status(void)
{
  return mode_get_proto_status();
}

// ----- TX -----

u08 *proto_cmd_api_tx_begin(u16 size)
{
  return mode_tx_begin(size);
}

u08 proto_cmd_api_tx_end(u16 size)
{
  return mode_tx_end(size);
}

// ----- rx packet from pio and send to parallel port -----

u16 proto_cmd_api_rx_size(void)
{
  return mode_rx_size();
}

u08 *proto_cmd_api_rx_begin(u16 size)
{
  return mode_rx_begin(size);
}

u08 proto_cmd_api_rx_end(u16 size)
{
  return mode_rx_end(size);
}




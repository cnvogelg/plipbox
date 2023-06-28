#ifndef DEBUG_H
#define DEBUG_H

#ifdef DEBUG
// enable debug output macros
#include "uartutil.h"
#include "uart.h"

// debug char
#define DC(x)  uart_send(x)
// debug string
#define DS(x)  uart_send_pstring(PSTR(x))
// debug string
#define DSB(x) uart_send_string((const char *)x)
// debug byte
#define DB(x)  uart_send_hex_byte(x)
// debug word
#define DW(x)  uart_send_hex_word(x)
// debug long
#define DL(x)  uart_send_hex_long(x)
// mac
#define DM(x)  uart_send_hex_mac(x)

// pointer
#if CONFIG_PTR_BITS == 16
#define DP(x)  uart_send_hex_word((u16)x)
#elif CONFIG_PTR_BITS == 32
#define DP(x)  uart_send_hex_long((u32)x)
#else
#error invalid CONFIG_PTR_BITS
#endif

// debug newline
#define DNL    uart_send_crlf()
// debug space
#define DSPC   uart_send(' ')

#else
// debug output is disabled

#define DC(x)
#define DS(x)
#define DSB(x)
#define DB(x)
#define DW(x)
#define DL(x)
#define DM(x)
#define DP(x)
#define DNL
#define DSPC

#endif

#endif

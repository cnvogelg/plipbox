#include <string.h>

#include "types.h"

#ifdef DEBUG_BUF
#define DEBUG
#endif

#include "debug.h"
#include "rx_buf.h"
#include "pkt_buf.h"
#include "uartutil.h"
#include "wire.h"

const u16 buffer_size = CONFIG_RX_BUF_SIZE_KB * 1024;

// in the buffer store packets as:
// u16 size + u08 data[size]+ u16 size2 + u08 data[size2] +++
// it wraps around at buffer size
static u08 buffer[CONFIG_RX_BUF_SIZE_KB * 1024];

static u16 pos_put;
static u16 pos_get;
static u16 used;
static u16 num_pkt;

static void store_size(u16 offset, u16 size)
{
  wire_h2w_u16(size, &buffer[offset]);
}

static u16 retrieve_size(u16 offset)
{
  u16 res = 0;
  wire_w2h_u16(&buffer[offset], &res);
  return res;
}

void rx_buf_init(void)
{
  pos_put = 0;
  pos_get = 0;
  used = 0;
  num_pkt = 0;
}

u16 rx_buf_get_size(void)
{
  return buffer_size;;
}

u16 rx_buf_get_used(void)
{
  return used;
}

u16 rx_buf_get_free(void)
{
  return buffer_size - used;
}

u16 rx_buf_get_num_pkt(void)
{
  return num_pkt;
}

void rx_buf_dump(void)
{
  uart_send_pstring(PSTR("rx_buf:size="));
  uart_send_hex_word(buffer_size);
  uart_send_pstring(PSTR(",used="));
  uart_send_hex_word(used);
  uart_send_pstring(PSTR(",num_pkt="));
  uart_send_hex_word(num_pkt);
}

u08 rx_buf_put(const u08 *data, u16 size)
{
  // padd to even size (align size fields on even offset)
  u16 even_size = (size + 1) & ~1;

  // check if new data fits in buffer
  u16 needed = used + even_size + 2; // +2 for size field
  if(needed > buffer_size) {
    return 0;
  }

  // update used
  used += even_size + 2;
  num_pkt++;

  // store size
  store_size(pos_put, size);

  // advance pointer
  pos_put += 2;
  if(pos_put > buffer_size) {
    pos_put = 0;
  }

  // copy data
  u16 data_off = pos_put + size;
  if(data_off > buffer_size) {
    // data wraps around
    u16 size1 = buffer_size - pos_put;
    u16 size2 = size - size1;
    memcpy(&buffer[pos_put], data, size1);
    memcpy(&buffer[0], data + size1, size2);
    pos_put = (pos_put + size2 + 1) & ~1;
  } else {
    // no wrap around
    // copy block directly
    memcpy(&buffer[pos_put], data, size);

    // advance pointer
    pos_put += even_size;
    if(pos_put > buffer_size) {
      pos_put -= buffer_size;
    }
  }
  return 1;
}

u16 rx_buf_get(u08 *data)
{
  // buffer is empty
  if(used == 0) {
    return 0;
  }

  // get next data size
  u16 size = retrieve_size(pos_get);
  u16 even_size = (size + 1) & ~1;

  used -= even_size + 2;
  num_pkt--;

  // advance pointer
  pos_get += 2;
  if(pos_get > buffer_size) {
    pos_get = 0;
  }

  // copy data
  u16 data_off = pos_get + size;
  if(data_off > buffer_size) {
    // data wraps around
    u16 size1 = buffer_size - pos_get;
    u16 size2 = size - size1;
    memcpy(data, &buffer[pos_get], size1);
    memcpy(data + size1, &buffer[0], size2);
    pos_get = (pos_get + size2 + 1) & ~1;
  } else {
    // no wrap around
    // copy block directly
    memcpy(data, &buffer[pos_get], size);

    // advance pointer
    pos_get += even_size;
    if(pos_get > buffer_size) {
      pos_get -= buffer_size;
    }
  }
}

u16 rx_buf_peek_buf_size(void)
{
  if(used == 0) {
    return 0;
  }
  return retrieve_size(pos_get);
}

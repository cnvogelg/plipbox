#include "types.h"

#ifdef DEBUG_BUF
#define DEBUG
#endif

#include "debug.h"
#include "rx_buf.h"
#include "pkt_buf.h"

struct buffer {
  u08 data[PKT_BUF_SIZE];
  u08 size;
};

static struct buffer buffers[CONFIG_RX_BUF_NUM];
static u08 pos_add;
static u08 pos_get;
static u08 size;

void rx_buf_init(void)
{
  pos_add = 0;
  pos_get = 0;
  size = 0;
}

u08 rx_buf_capacity(void)
{
  return CONFIG_RX_BUF_NUM;
}

u08 rx_buf_size(void)
{
  return size;
}

u08 rx_buf_free(void)
{
  return CONFIG_RX_BUF_NUM - size;
}

u08 *rx_buf_add(u16 buf_size)
{
  if((size == CONFIG_RX_BUF_NUM)) {
    return NULL;
  }
  if((buf_size > PKT_BUF_SIZE)) {
    return NULL;
  }

  u08 old_pos = pos_add;
  pos_add++;
  if(pos_add == CONFIG_RX_BUF_NUM) {
    pos_add = 0;
  }
  size++;

  buffers[old_pos].size = buf_size;
  return buffers[old_pos].data;
}

u08 *rx_buf_get(u16 *buf_size)
{
  if(size == 0) {
    return NULL;
  }

  u08 old_pos = pos_get;
  pos_get++;
  if(pos_get == CONFIG_RX_BUF_NUM) {
    pos_get = 0;
  }
  size--;

  *buf_size = buffers[old_pos].size;
  return buffers[old_pos].data;
}

u16 rx_buf_peek_buf_size(void)
{
  if(size == 0) {
    return 0;
  }
  return buffers[pos_get].size;
}

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
static u08 pos_put;
static u08 pos_get;
static u08 size;

void rx_buf_init(void)
{
  pos_put = 0;
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

u08 *rx_buf_put_begin(u16 buf_size)
{
  if((size == CONFIG_RX_BUF_NUM)) {
    return NULL;
  }
  if((buf_size > PKT_BUF_SIZE)) {
    return NULL;
  }

  buffers[pos_put].size = buf_size;
  return buffers[pos_put].data;
}

void rx_buf_put_end(void)
{
  pos_put++;
  if(pos_put == CONFIG_RX_BUF_NUM) {
    pos_put = 0;
  }
  size++;
}

u08 *rx_buf_get_begin(u16 *buf_size)
{
  if(size == 0) {
    return NULL;
  }

  *buf_size = buffers[pos_get].size;
  return buffers[pos_get].data;
}

void rx_buf_get_end(void)
{
  pos_get++;
  if(pos_get == CONFIG_RX_BUF_NUM) {
    pos_get = 0;
  }
  size--;
}

u16 rx_buf_peek_buf_size(void)
{
  if(size == 0) {
    return 0;
  }
  return buffers[pos_get].size;
}

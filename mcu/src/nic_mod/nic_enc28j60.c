#include "types.h"

#ifdef DEBUG_ENC28J60
#define DEBUG
#endif

#include "debug.h"
#include "nic.h"
#include "nic_mod.h"
#include "nic_enc28j60.h"
#include "enc28j60.h"
#include "pkt_buf.h"

#define MODE_NORMAL         0
#define MODE_LOOP_BUF       1

static u08 mode;
static u08 enc_flags;

static void map_caps(u16 caps)
{
  mode = MODE_NORMAL;
  if((caps & NIC_CAP_LOOP_BACK)) {
    mode = MODE_LOOP_BUF;
  }

  enc_flags = 0;
  if((caps & NIC_CAP_BROADCAST)) {
    enc_flags |= ENC28J60_FLAG_RX_BROADCAST;
  }
  if((caps & NIC_CAP_FULL_DUPLEX)) {
    enc_flags |= ENC28J60_FLAG_FULL_DUPLEX;
  }
  if((caps & NIC_CAP_FLOW_CONTROL)) {
    enc_flags |= ENC28J60_FLAG_FLOW_CONTROL;
  }
}

static u08 attach(u16 *caps, u08 port, mac_t mac)
{
  // check port
  if(port > enc28j60_num_ports()) {
    return NIC_ERROR_INVALID_PORT;
  }

  // try to reset enc and identify
  u08 res = enc28j60_reset_and_find(port);
  if(res == ENC28J60_ERROR_NOT_FOUND) {
    return NIC_ERROR_DEVICE_NOT_FOUND;
  }

  map_caps(*caps);
  enc28j60_setup_buffers();

  if(mode == MODE_LOOP_BUF) {
    pkt_size = 0;
  }
  else {
    // setup MAC
    DS(("enc28j60: attach: ")); DB(enc_flags); DNL;
    enc28j60_setup_mac_phy(mac, enc_flags);
    enc28j60_enable_rx();
  }

  return NIC_OK;
}

static void detach(void)
{
}

static void ping(void)
{
  DS(("enc28j60: ping: num_pkt="));
  DB(enc28j60_rx_num_pending());
  DNL;
}

static u08 rx_num_pending(void)
{
  if(mode == MODE_LOOP_BUF) {
    return (pkt_size > 0) ? 1 : 0;
  } else {
    return enc28j60_rx_num_pending();
  }
}

static u08 rx_size(u16 *got_size)
{
  if(mode == MODE_LOOP_BUF) {
    *got_size = pkt_size;
  } else {
    u16 size = 0;
    u08 ok = enc28j60_rx_size(&size);
    if(ok != ENC28J60_OK) {
      return NIC_ERROR_RX;
    }
  }
  return NIC_OK;
}

static u08 rx_data(u08 *buf, u16 size)
{
  if(mode == MODE_LOOP_BUF) {
    enc28j60_rx_begin_loop_back();
    enc28j60_rx_data(buf, size);
    enc28j60_rx_end_loop_back();
    pkt_size = 0;
  } else {
    enc28j60_rx_begin();
    enc28j60_rx_data(buf, size);
    enc28j60_rx_end();
  }
  return NIC_OK;
}

static u08 tx_data(const u08 *buf, u16 size)
{
  if(mode == MODE_LOOP_BUF) {
    enc28j60_tx_begin_loop_back();
    enc28j60_tx_data(buf, size);
    enc28j60_tx_end_loop_back();
    pkt_size = size;
  } else {
    enc28j60_tx_begin();
    enc28j60_tx_data(buf, size);
    enc28j60_tx_end(size);
  }
  return NIC_OK;
}

static void rx_direct_begin(u16 size)
{
  if(mode == MODE_LOOP_BUF) {
    enc28j60_rx_begin_loop_back();
    pkt_size = 0;
  } else {
    enc28j60_rx_begin();
  }
}

static u08 rx_direct_end(u16 size)
{
  if(mode == MODE_LOOP_BUF) {
    enc28j60_rx_end_loop_back();
  } else {
    enc28j60_rx_end();
  }
  return NIC_OK;
}

static void tx_direct_begin(u16 size)
{
  if(mode == MODE_LOOP_BUF) {
    enc28j60_tx_begin_loop_back();
    pkt_size = size;
  } else {
    enc28j60_tx_begin();
  }
}

static u08 tx_direct_end(u16 size)
{
  if(mode == MODE_LOOP_BUF) {
    enc28j60_tx_end_loop_back();
  } else {
    enc28j60_tx_end(size);
  }
  return NIC_OK;
}

static u08 ioctl(u08 cmd, u08 *value)
{
  switch(cmd) {
  case NIC_IOCTL_GET_HW_VERSION:
    *value = enc28j60_hw_revision();
    return NIC_OK;
  case NIC_IOCTL_GET_LINK_STATUS:
    *value = enc28j60_link_up();
    return NIC_OK;
  default:
    return NIC_ERROR_IOCTL_NOT_FOUND;
  }
}

// ----- NIC module -----
static const char ROM_ATTR name[] = "enc28j60";
const nic_mod_t ROM_ATTR nic_mod_enc28j60 = {
  .name = name,
  .caps= NIC_CAP_LOOP_BACK | NIC_CAP_DIRECT_IO |
         NIC_CAP_BROADCAST | NIC_CAP_FULL_DUPLEX | NIC_CAP_FLOW_CONTROL,

  .attach = attach,
  .detach = detach,

  .ping = ping,

  .rx_num_pending = rx_num_pending,
  .rx_size = rx_size,
  .rx_data = rx_data,
  .tx_data = tx_data,

  .rx_direct_begin = rx_direct_begin,
  .rx_direct_end = rx_direct_end,
  .tx_direct_begin = tx_direct_begin,
  .tx_direct_end = tx_direct_end,

  .ioctl = ioctl
};

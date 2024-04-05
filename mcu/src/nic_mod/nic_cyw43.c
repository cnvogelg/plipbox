
//#include <cyw43.h>
#include <pico/cyw43_arch.h>

#include "types.h"

#ifdef DEBUG_CYW43
#define DEBUG
#endif

#include "debug.h"
#include "uartutil.h"
#include "nic.h"
#include "nic_mod.h"
#include "nic_wifi.h"
#include "nic_wifi_mod.h"
#include "nic_cyw43.h"
#include "pkt_buf.h"
#include "rx_buf.h"
#include "param.h"
#include "net.h"

static u08 link_up;

// --- driver callbacks ---

void cyw43_cb_process_ethernet(void *cb_data, int itf, size_t len, const uint8_t *buf)
{
  DT; DS(("cyw43: rx:")); DW(len);
  u08 added = rx_buf_put(buf, len);
  if(added) {
    DS(" add");
  } else {
    DS(" drop - no buf!");
  }
  DNL;
}

void cyw43_cb_tcpip_set_link_down(cyw43_t *self, int itf)
{
  DT; DS(("cyw43: link down!\n"));
  link_up = 0;
}

void cyw43_cb_tcpip_set_link_up(cyw43_t *self, int itf)
{
  DT; DS(("cyw43: link up!\n"));
  link_up = 1;
}

// dummy func to please cyw43 driver
struct pbuf;
uint16_t pbuf_copy_partial(const struct pbuf *p, void *dataptr, uint16_t len, uint16_t offset)
{
  return 0;
}

// wifi scan callback
static int wifi_scan_result(void *env, const cyw43_ev_scan_result_t *result)
{
  DT; DS(("cyw43: wifi scan:")); DNL;
  nic_wifi_mod_scan_result_t result_cb = (nic_wifi_mod_scan_result_t)env;
  nic_wifi_scan_result_t nic_result;

  // transfer result
  memcpy(nic_result.ssid, result->ssid, result->ssid_len);
  nic_result.ssid[result->ssid_len] = '\0';
  nic_result.ssid_len = result->ssid_len;
  nic_result.channel = result->channel;
  nic_result.rssi = result->rssi;

  // convert auth mode
  nic_result.auth_mode = NIC_WIFI_AUTH_NONE;
  u08 auth_mode = result->auth_mode;
  if(auth_mode != 0) {
    if(auth_mode & 1) {
      nic_result.auth_mode |= NIC_WIFI_AUTH_WEP;
    }
    if(auth_mode & 2) {
      nic_result.auth_mode |= NIC_WIFI_AUTH_WPA;
    }
    if(auth_mode & 4) {
      nic_result.auth_mode |= NIC_WIFI_AUTH_WPA2;
    }
  }

  // pass result back
  result_cb(&nic_result);
}

// --- nic API ---

static u08 attach(u16 caps, u08 port, mac_t mac)
{
  // setup cyw43
  DT; DS(("cyw43: init:mac=")); DM(mac);
  if(cyw43_arch_init()) {
    DS(("FAILED!\n"));
    return NIC_ERROR_DEVICE_NOT_FOUND;
  }

  // set mac
  memcpy(cyw43_state.mac, mac, sizeof(cyw43_state.mac));

  // enable station mode
  cyw43_arch_enable_sta_mode();

  // read back mac
  mac_t amac;
  cyw43_wifi_get_mac(&cyw43_state, CYW43_ITF_STA, amac);
  DS((",got=")); DM(amac); DNL;
  if(!net_compare_mac(mac, amac)) {
    return NIC_ERROR_DEVICE_ERROR;
  }

  // start connection
  const u08 *ssid = param_get_wifi_ssid();
  const u08 *pass = param_get_wifi_pass();
  DT; DS(("cyw43: connect:ssid="));
  DS((ssid)); DS((",pass=")); DS((pass)); DC(':');
  int res = cyw43_arch_wifi_connect_async(ssid, pass, CYW43_AUTH_WPA2_MIXED_PSK);
  if(res != PICO_OK) {
    DS(("FAILED!\n"));
    return NIC_ERROR_CONNECT_FAILED;
  }
  DS(("ok\n"));

  link_up = 0;
  rx_buf_init();

  return NIC_OK;
}

static void detach(void)
{
  DT; DS(("cyw43: exit:"));
  cyw43_wifi_leave(&cyw43_state, CYW43_ITF_STA);

  cyw43_arch_disable_sta_mode();

  cyw43_arch_deinit();
  DS(("done\n"));
}

static void ping(void)
{
}

static void status(void)
{
  uart_send_time_stamp_spc();
  uart_send_pstring(PSTR("cyw43 status:"));
  rx_buf_dump();
  uart_send_crlf();
}

static u08 rx_num_pending(void)
{
#if PICO_CYW43_ARCH_POLL
  cyw43_arch_poll();
#endif

  return rx_buf_get_num_pkt();
}

static u08 rx_size(u16 *got_size)
{
  *got_size = rx_buf_peek_buf_size();
  return NIC_OK;
}

static u08 *rx_begin(u16 size)
{
  rx_buf_get(pkt_buf);
  return pkt_buf;
}

static u08 rx_end(u16 size)
{
  return NIC_OK;
}

static u08 *tx_begin(u16 size)
{
  return pkt_buf;
}

static u08 tx_end(u16 size)
{
  DS(("cyw43: tx:")); DW(size);
  int ret = cyw43_send_ethernet(&cyw43_state, CYW43_ITF_STA, size, pkt_buf, 0);
  if(ret != PICO_OK) {
    DS((":ERROR!\n"));
    return NIC_ERROR_TX;
  }
  DS((":ok\n"));

  return NIC_OK;
}

static u08 ioctl(u08 cmd, u08 *value)
{
  switch(cmd) {
  case NIC_IOCTL_GET_LINK_STATUS:
    *value = link_up;
    return NIC_OK;

  case NIC_WIFI_IOCTL_GET_EXT_LINK_STATUS:
    {
      int state = cyw43_wifi_link_status(&cyw43_state, CYW43_ITF_STA);
      switch(state) {
      case CYW43_LINK_JOIN:
        *value = NIC_WIFI_LINK_UP;
        break;
      case CYW43_LINK_FAIL:
        *value = NIC_WIFI_LINK_FAIL;
        break;
      case CYW43_LINK_NONET:
        *value = NIC_WIFI_LINK_NO_NET;
        break;
      case CYW43_LINK_BADAUTH:
        *value = NIC_WIFI_LINK_BAD_AUTH;
        break;
      case CYW43_LINK_DOWN:
        *value = NIC_WIFI_LINK_DOWN;
        break;
      default:
        *value = NIC_WIFI_LINK_UNKNOWN;
        break;
      }
      return NIC_OK;
    }

  case NIC_WIFI_IOCTL_GET_RSSI:
    {
      s16 *rssi = (s16 *)value;
      int32_t cy_rssi;
      int res = cyw43_wifi_get_rssi(&cyw43_state, &cy_rssi);
      if(res != 0) {
        return NIC_ERROR_DEVICE_ERROR;
      }
      return NIC_OK;
    }

  case NIC_WIFI_IOCTL_GET_BSSID:
    {
      int res = cyw43_wifi_get_bssid(&cyw43_state, value);
      if(res != 0) {
        return NIC_ERROR_DEVICE_ERROR;
      }
      return NIC_OK;
    }

  default:
    return NIC_ERROR_IOCTL_NOT_FOUND;
  }
}

// ----- WIFI API -----

u08 wifi_scan_start(nic_wifi_mod_scan_result_t result_cb)
{
  // can't start new scan since a scan is still busy
  if(cyw43_wifi_scan_active(&cyw43_state)) {
    return NIC_ERROR_WIFI_SCAN_BUSY;
  }

  // start scan
  cyw43_wifi_scan_options_t scan_options = { 0 };
  DS(("cyw46: scan:"));
  int res= cyw43_wifi_scan(&cyw43_state, &scan_options, result_cb, wifi_scan_result);
  if(res != PICO_OK) {
    DS(("ERROR\n"));
    return NIC_ERROR_DEVICE_ERROR;
  }
  DS(("ok\n"));

  return NIC_OK;
}

u08 wifi_scan_busy(void)
{
  return cyw43_wifi_scan_active(&cyw43_state);
}

// ----- Wifi Extension -----
static const nic_wifi_mod_t ROM_ATTR nic_wifi_mod_cyw43 = {
  .scan_start = wifi_scan_start,
  .scan_busy = wifi_scan_busy,
};

// ----- NIC module -----
static const char ROM_ATTR name[] = "cyw43";
const nic_mod_t ROM_ATTR nic_mod_cyw43 = {
  .name = name,
  .caps = NIC_CAP_LINK_STATUS,
  .tag = NIC_TAG_CYW,

  .attach = attach,
  .detach = detach,

  .ping = ping,
  .status = status,

  .rx_num_pending = rx_num_pending,
  .rx_size = rx_size,
  .rx_begin = rx_begin,
  .rx_end = rx_end,

  .tx_begin = tx_begin,
  .tx_end = tx_end,

  .ioctl = ioctl,
  .wifi_ext = &nic_wifi_mod_cyw43
};

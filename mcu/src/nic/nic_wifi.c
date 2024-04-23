#include <string.h>

#include "arch.h"
#include "types.h"

#ifdef DEBUG_NIC
#define DEBUG
#endif

#include "debug.h"
#include "uartutil.h"
#include "nic_wifi.h"
#include "nic_wifi_mod.h"
#include "nic_mod.h"

static u08 num_scan_results;
static u08 scan_active;
static nic_wifi_scan_result_t scan_results[CONFIG_WIFI_SCAN_MAX_RESULT];

static void wifi_scan_cb(const nic_wifi_scan_result_t *result)
{
  uart_send_time_stamp_spc();
  uart_send_pstring(PSTR("wifi_scan: ssid="));
  uart_send_string(result->ssid);
  uart_send_pstring(PSTR(" auth_mode="));
  uart_send_hex_byte(result->auth_mode);
  uart_send_pstring(PSTR(" channel="));
  uart_send_hex_word(result->channel);
  uart_send_pstring(PSTR(" rssi="));
  uart_send_hex_word(result->rssi);
  uart_send_pstring(PSTR(" index="));

  if(num_scan_results < CONFIG_WIFI_SCAN_MAX_RESULT) {
    uart_send_hex_byte(num_scan_results);
    memcpy(&scan_results[num_scan_results], result, sizeof(nic_wifi_scan_result_t));
    num_scan_results++;
  } else {
    uart_send_pstring(PSTR("NONE!"));
  }
  uart_send_crlf();
}

void nic_wifi_init(void)
{
  nic_wifi_mod_set_current();
  num_scan_results = 0;
  scan_active = 0;
}

u08 nic_wifi_is_available(void)
{
  return nic_wifi_mod_is_available();
}

u08 nic_wifi_scan_start(void)
{
  if(scan_active) {
    return NIC_ERROR_WIFI_SCAN_BUSY;
  }

  num_scan_results = 0;

  uart_send_time_stamp_spc();
  uart_send_pstring(PSTR("wifi_scan: res="));
  u08 res = nic_wifi_mod_scan_start(wifi_scan_cb);
  uart_send_hex_byte(res);
  uart_send_crlf();
  if(res == NIC_OK) {
    scan_active = 1;
  }
  return res;
}

u08 nic_wifi_scan_busy(void)
{
  u08 busy = nic_wifi_mod_scan_busy();
  if(!busy) {
    uart_send_time_stamp_spc();
    uart_send_pstring(PSTR("wifi_scan: done. found="));
    uart_send_hex_byte(num_scan_results);
    uart_send_crlf();
    scan_active = 0;
  }
  return busy;
}

u08 nic_wifi_scan_num_result(void)
{
  return num_scan_results;
}

const nic_wifi_scan_result_t *nic_wifi_scan_get_result(u08 index)
{
  if(index >= num_scan_results) {
    return NULL;
  }
  return &scan_results[index];
}

u08 nic_wifi_get_rssi(s16 *rssi)
{
  return nic_mod_ioctl(NIC_WIFI_IOCTL_GET_RSSI, (u08 *)rssi);
}

u08 nic_wifi_get_bssid(mac_t bssid)
{
  return nic_mod_ioctl(NIC_WIFI_IOCTL_GET_BSSID, (u08 *)bssid);
}

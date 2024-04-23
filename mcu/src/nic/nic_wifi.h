#ifndef NIC_WIFI_H
#define NIC_WIFI_H

#include "types.h"
#include "nic_shared.h"

#ifndef CONFIG_WIFI_SCAN_MAX_RESULT
#define CONFIG_WIFI_SCAN_MAX_RESULT 32
#endif

/* wifi ioctl */
#define NIC_WIFI_IOCTL_GET_RSSI             100 /* s16 rssid */
#define NIC_WIFI_IOCTL_GET_BSSID            101 /* mac_t bssid */

/* scan result */
typedef struct nic_wifi_scan_result {
  u08 ssid[NIC_WIFI_SSID_SIZE+1]; /* null terminated */
  u08 ssid_len;
  u08 auth_mode;
  u16 channel;
  s16 rssi;
} nic_wifi_scan_result_t;

extern void nic_wifi_init(void);
extern u08 nic_wifi_is_available(void);
extern u08 nic_wifi_scan_start(void);
extern u08 nic_wifi_scan_busy(void);

extern u08 nic_wifi_scan_num_result(void);
extern const nic_wifi_scan_result_t *nic_wifi_scan_get_result(u08 index);

extern u08 nic_wifi_get_rssi(s16 *rssi);
extern u08 nic_wifi_get_bssid(mac_t bssid);

#endif

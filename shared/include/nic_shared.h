#ifndef NIC_SHARED_H
#define NIC_SHARED_H

/* capabilities bit field (requested in NCAP) */
#define NIC_CAP_DIRECT_IO       1
#define NIC_CAP_LOOP_BACK       2
#define NIC_CAP_LINK_STATUS     4

#define NIC_CAP_BROADCAST       0x10
#define NIC_CAP_FULL_DUPLEX     0x20
#define NIC_CAP_FLOW_CONTROL    0x40

/* result values */
#define NIC_OK                        0
#define NIC_ERROR_INVALID_PORT        1
#define NIC_ERROR_DEVICE_NOT_FOUND    2
#define NIC_ERROR_IOCTL_NOT_FOUND     3
#define NIC_ERROR_RX                  4
#define NIC_ERROR_TX                  5
#define NIC_ERROR_ALREADY_ATTACHED    6
#define NIC_ERROR_NOT_ATTACHED        7
#define NIC_ERROR_CONNECT_FAILED      8
#define NIC_ERROR_DEVICE_ERROR        9

/* wifi results */
#define NIC_ERROR_WIFI_NOT_SUPPORTED  100
#define NIC_ERROR_WIFI_SCAN_BUSY      101

/* wifi parameter sizes */
#define NIC_WIFI_SSID_SIZE      32
#define NIC_WIFI_PASS_SIZE      32

/* auth mask in scan result */
#define NIC_WIFI_AUTH_NONE      0
#define NIC_WIFI_AUTH_WEP       1
#define NIC_WIFI_AUTH_WPA       2
#define NIC_WIFI_AUTH_WPA2      4
#define NIC_WIFI_AUTH_UNKNOWN   99

/* wifi ext link status */
#define NIC_WIFI_LINK_DOWN      0
#define NIC_WIFI_LINK_UP        1
#define NIC_WIFI_LINK_NO_NET    2
#define NIC_WIFI_LINK_BAD_AUTH  3
#define NIC_WIFI_LINK_FAIL      4
#define NIC_WIFI_LINK_UNKNOWN   99

#endif

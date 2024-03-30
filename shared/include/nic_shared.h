#ifndef NIC_SHARED_H
#define NIC_SHARED_H

#include "tag.h"

/* options */
#define NIC_OPT_FAST_IO         1 /* prefer direct if available */
#define NIC_OPT_LOOP_BACK       2
#define NIC_OPT_FULL_DUPLEX     4

/* capabilities bit field */
#define NIC_CAP_BUFFER_IO       1
#define NIC_CAP_DIRECT_IO       2
#define NIC_CAP_LOOP_BACK       4
#define NIC_CAP_LINK_STATUS     8

#define NIC_CAP_RX_BROADCAST    0x10
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

// nic_def
// +00 u08 index
// +01 u32 tag
// +05 u16 caps
// =07
#define NIC_DEF_SIZE      7

// if an id was not found
#define NIC_ID_INVALID    0xff

// tags
#define NIC_TAG_LOOP            MAKE_TAG('L','O','O','P')
#define NIC_TAG_ENC             MAKE_TAG('E','N','C', 0)
#define NIC_TAG_CYW             MAKE_TAG('C','Y','W', 0)

#endif

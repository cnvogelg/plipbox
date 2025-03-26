#ifndef NIC_SHARED_H
#define NIC_SHARED_H

#include "tag.h"

/* options */
#define NIC_OPT_DIRECT_IO       1 /* prefer direct if available */
#define NIC_OPT_LOOP_BACK       2
#define NIC_OPT_FULL_DUPLEX     4

/* capabilities bit field */
#define NIC_CAP_DIRECT_IO       1
#define NIC_CAP_LOOP_BACK       2
#define NIC_CAP_LINK_STATUS     4

#define NIC_CAP_RX_BROADCAST    0x10
#define NIC_CAP_FULL_DUPLEX     0x20
#define NIC_CAP_FLOW_CONTROL    0x40

/* --- nic status --- */
#define NIC_STATUS_OK                        0
#define NIC_STATUS_UNKNOWN                   0
#define NIC_STATUS_ATTACHED                  1
#define NIC_STATUS_DETACHED                  2
#define NIC_STATUS_ERROR_INVALID_PORT        10
#define NIC_STATUS_ERROR_DEVICE_NOT_FOUND    11
#define NIC_STATUS_ERROR_IOCTL_NOT_FOUND     12
#define NIC_STATUS_ERROR_RX                  13
#define NIC_STATUS_ERROR_TX                  14
#define NIC_STATUS_ERROR_ALREADY_ATTACHED    15
#define NIC_STATUS_ERROR_NOT_ATTACHED        16
#define NIC_STATUS_ERROR_CONNECT_FAILED      17
#define NIC_STATUS_ERROR_DEVICE_ERROR        18
/* wifi nic extended status */
#define NIC_STATUS_ERROR_WIFI_NOT_SUPPORTED  100
#define NIC_STATUS_ERROR_WIFI_SCAN_BUSY      101

/* --- link status --- */
#define NIC_LINK_STATUS_UNKNOWN       0x00
#define NIC_LINK_STATUS_UP            0x01
#define NIC_LINK_STATUS_DOWN          0x02
#define NIC_LINK_STATUS_FAILED        0x03
#define NIC_LINK_STATUS_NO_NET        0x04
#define NIC_LINK_STATUS_BAD_AUTH      0x05

/* wifi parameter sizes */
#define NIC_WIFI_SSID_SIZE      32
#define NIC_WIFI_PASS_SIZE      32

/* auth mask in scan result */
#define NIC_WIFI_AUTH_NONE      0
#define NIC_WIFI_AUTH_WEP       1
#define NIC_WIFI_AUTH_WPA       2
#define NIC_WIFI_AUTH_WPA2      4
#define NIC_WIFI_AUTH_UNKNOWN   99

/* ----- nic_def ----- */
// +00 u32 tag
// +04 u16 caps
// =06
#define NIC_DEF_SIZE      6

// if an id was not found
#define NIC_ID_INVALID    0xff

// tags
#define NIC_TAG_LOOP            MAKE_TAG('L','O','O','P')
#define NIC_TAG_ENC             MAKE_TAG('E','N','C', 0)
#define NIC_TAG_CYW             MAKE_TAG('C','Y','W', 0)

#endif

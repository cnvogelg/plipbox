#include "types.h"
#include "arch.h"
#include "param_def.h"
#include "param_shared.h"
#include "param.h"

// runtime parameter
param_t param;

// default value
const param_t ROM_ATTR default_param = {
  .mac_addr = { 0x1a,0x11,0xaf,0xa0,0x47,0x11},
  .mode = 0,
  .nic = 0,
  .nic_caps = 0,
  .nic_port = 0,
  .ip_addr = { 10, 0, 0, 2 },
  .net_mask = { 255, 255, 255, 0},
#ifdef HAVE_WIFI
  .wifi_ssid = "ssid",
  .wifi_pass = "pass",
#endif
};

// descriptions
static const char ROM_ATTR desc_mac[] = "mac adress";
static const char ROM_ATTR desc_mode[] = "operation mode";
static const char ROM_ATTR desc_nic[] = "NIC device";
static const char ROM_ATTR desc_nic_caps[] = "NIC flags";
static const char ROM_ATTR desc_nic_port[] = "NIC port";
static const char ROM_ATTR desc_ip_addr[] = "test IP addr";
static const char ROM_ATTR desc_net_mask[] = "test net mask";
#ifdef HAVE_WIFI
static const char ROM_ATTR desc_wifi_ssid[] = "Wifi SSID";
static const char ROM_ATTR desc_wifi_pass[] = "Wifi password";
#endif

// parameter description
const param_def_t ROM_ATTR param_defs[] = {
  {
    .index = 0,
    .tag = PARAM_TAG_MAC_ADDR,
    .type = PARAM_TYPE_BYTE_ARRAY,
    .format = PARAM_FORMAT_HEX,
    .size = MAC_SIZE,
    .data = (u08 *)&param.mac_addr,
    .desc = desc_mac
  },
  {
    .index = 1,
    .tag = PARAM_TAG_MODE,
    .type = PARAM_TYPE_WORD,
    .format = PARAM_FORMAT_DEC,
    .size = PARAM_SIZE_WORD,
    .data = (u08 *)&param.mode,
    .desc = desc_mode
  },
  {
    .index = 2,
    .tag = PARAM_TAG_NIC,
    .type = PARAM_TYPE_WORD,
    .format = PARAM_FORMAT_DEC,
    .size = PARAM_SIZE_WORD,
    .data = (u08 *)&param.nic,
    .desc = desc_nic
  },
  {
    .index = 3,
    .tag = PARAM_TAG_NCAP,
    .type = PARAM_TYPE_WORD,
    .format = PARAM_FORMAT_BIN,
    .size = PARAM_SIZE_WORD,
    .data = (u08 *)&param.nic_caps,
    .desc = desc_nic_caps
  },
  {
    .index = 4,
    .tag = PARAM_TAG_NPRT,
    .type = PARAM_TYPE_WORD,
    .format = PARAM_FORMAT_BIN,
    .size = PARAM_SIZE_WORD,
    .data = (u08 *)&param.nic_port,
    .desc = desc_nic_port
  },
  {
    .index = 5,
    .tag = PARAM_TAG_IP,
    .type = PARAM_TYPE_BYTE_ARRAY,
    .format = PARAM_FORMAT_DEC,
    .size = IP_ADDR_SIZE,
    .data = (u08 *)&param.ip_addr,
    .desc = desc_ip_addr
  },
  {
    .index = 6,
    .tag = PARAM_TAG_NMSK,
    .type = PARAM_TYPE_BYTE_ARRAY,
    .format = PARAM_FORMAT_DEC,
    .size = IP_ADDR_SIZE,
    .data = (u08 *)&param.net_mask,
    .desc = desc_net_mask
  },
#ifdef HAVE_WIFI
  {
    .index = 7,
    .tag = PARAM_TAG_WIFI_SSID,
    .type = PARAM_TYPE_BYTE_ARRAY,
    .format = PARAM_FORMAT_STR,
    .size = NIC_WIFI_SSID_SIZE,
    .data = (u08 *)&param.wifi_ssid,
    .desc = desc_wifi_ssid
  },
  {
    .index = 8,
    .tag = PARAM_TAG_WIFI_PASS,
    .type = PARAM_TYPE_BYTE_ARRAY,
    .format = PARAM_FORMAT_STR,
    .size = NIC_WIFI_PASS_SIZE,
    .data = (u08 *)&param.wifi_pass,
    .desc = desc_wifi_pass
  }
#endif
};

const size_t param_defs_size = sizeof(param_defs) / sizeof(param_def_t);

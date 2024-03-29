#ifndef NIC_WIFI_MOD_H
#define NIC_WIFI_MOD_H

#include "arch.h"
#include "types.h"
#include "nic_wifi.h"
#include "nic_wifi_mod.h"

typedef void (*nic_wifi_mod_scan_result_t)(const nic_wifi_scan_result_t *result);

/* function pointers */
typedef u08 (*nic_wifi_mod_scan_start_t)(nic_wifi_mod_scan_result_t result_cb);
typedef u08 (*nic_wifi_mod_scan_busy_t)(void);

typedef struct nic_wifi_mod {
  nic_wifi_mod_scan_start_t  scan_start;
  nic_wifi_mod_scan_busy_t   scan_busy;
} nic_wifi_mod_t;

typedef const nic_wifi_mod_t *nic_wifi_mod_ptr_t;

extern nic_wifi_mod_ptr_t nic_wifi_mod_ptr;

extern void nic_wifi_mod_set_current(void);
static inline u08 nic_wifi_mod_is_available(void)
{
  return nic_wifi_mod_ptr != NULL;
}

static inline u08 nic_wifi_mod_scan_start(nic_wifi_mod_scan_result_t result_cb)
{
  nic_wifi_mod_ptr_t pd = nic_wifi_mod_ptr;
  nic_wifi_mod_scan_start_t scan_start = (nic_wifi_mod_scan_start_t)read_rom_rom_ptr(&pd->scan_start);
  return scan_start(result_cb);
}

static inline u08 nic_wifi_mod_scan_busy(void)
{
  nic_wifi_mod_ptr_t pd = nic_wifi_mod_ptr;
  nic_wifi_mod_scan_busy_t scan_busy = (nic_wifi_mod_scan_busy_t)read_rom_rom_ptr(&pd->scan_busy);
  return scan_busy();
}

#endif

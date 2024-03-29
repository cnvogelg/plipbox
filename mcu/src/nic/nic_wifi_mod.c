#include "types.h"

#ifdef DEBUG_NIC
#define DEBUG
#endif

#include "debug.h"
#include "nic_wifi_mod.h"
#include "nic_mod.h"

// current pointer
nic_wifi_mod_ptr_t nic_wifi_mod_ptr;

void nic_wifi_mod_set_current(void)
{
  nic_wifi_mod_ptr = nic_mod_wifi_ext();
}

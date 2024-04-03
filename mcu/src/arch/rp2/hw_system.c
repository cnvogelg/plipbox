#include <string.h>

#include "hardware/watchdog.h"
#include "pico/unique_id.h"

#include "types.h"
#include "hw_system.h"

void hw_system_init(void)
{
}

void hw_system_reset(void)
{
  watchdog_enable(1, true);
  // wait for my death
  while(1) {}
}

void hw_system_otp_mac(mac_t mac)
{
  pico_unique_board_id_t board_id;

  pico_get_unique_board_id(&board_id);
  memcpy(mac, &board_id.id[2], 6);
  mac[0] &= (uint8_t)~0x1; // unicast
  mac[0] |= 0x2; // locally administered
}

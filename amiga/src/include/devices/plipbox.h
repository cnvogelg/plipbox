/* SANA2 Extensions of the plipbox.device */

#include <devices/sana2.h>

#define S2PB_GET_VERSION  (S2_END+0)
#define S2PB_SET_MAC      (S2_END+1)
#define S2PB_SET_MODE     (S2_END+2)
#define S2PB_GET_MODE     (S2_END+3)
#define S2PB_SET_FLAGS    (S2_END+4)
#define S2PB_GET_FLAGS    (S2_END+5)
#define S2PB_RESET_PREFS  (S2_END+6)
#define S2PB_LOAD_PREFS   (S2_END+7)
#define S2PB_SAVE_PREFS   (S2_END+8)

/*
  S2PB_GET_VERSION

    out:
      ios2_Req.io_Error
      ios2_WireError: on success its the version

    Check if a plipbox device is here and return its version

    Version LONG contains both device and firmware version

                     Bits
    VER_DEV_MAJOR    31..24
    VER_DEV_MINOR    23..16
    VER_FW_MAJOR     15..8
    VER_FW_MINOR      7..0

  S2PB_SET_MAC

    in:
      ios_SrcAddr: new mac
    out:
      ios2_Req.io_Error
      ios2_WireError

    Set a new current MAC address in plipbox.

  S2PB_SET_MODE/FLAGS

    in:
      ios_WireError: new mode
    out:
      ios2_Req.io_Error
      ios2_WireError

    Set operation mode of plipbox.

  S2PB_GET_MODE/FLAGS

    in:
      -
    out:
      ios2_WireError: current mode
      ios2_Req.io_Error

    Get operation mode of plipbox.

  S2PB_RESET_PREFS
  S2PB_LOAD_PREFS
  S2PB_SAVE_PREFS

    in:
      -
    out:
      ios2_WireError: result code
*/

// mode
#define S2PB_MODE_BRIDGE          0x00
#define S2PB_MODE_LOOPBACK_BUF    0x01
#define S2PB_MODE_LOOPBACK_DEV    0x02
#define S2PB_MODE_MASK            0x03
// mask for transfer type
#define S2PB_MODE_BUF_TRANSFER    0x00
#define S2PB_MODE_SPI_TRANSFER    0x80
#define S2PB_MODE_TRANSFER_MASK   0x80

/* SANA2 Extensions of the plipbox.device */

#include <devices/sana2.h>

#define S2PB_SET_MAC      (S2_END+0)
#define S2PB_SET_MODE     (S2_END+1)
#define S2PB_GET_MODE     (S2_END+2)

/*
  S2PB_SET_MAC

    in:
      ios_SrcAddr: new mac
    out:
      ios2_Req.io_Error
      ios2_WireError

    Set a new current MAC address in plipbox.

  S2PB_SET_MODE

    in:
      ios_WireError: new mode
    out:
      ios2_Req.io_Error
      ios2_WireError

    Set operation mode of plipbox.

  S2PB_GET_MODE

    in:
      -
    out:
      ios_WireError: current mode
      ios2_Req.io_Error
      ios2_WireError

    Get operation mode of plipbox.
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

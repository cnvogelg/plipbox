/* SANA2 Extensions of the plipbox.device */

#ifndef DEVICES_PLIPBOX_H
#define DEVICES_PLIPBOX_H

#include <devices/sana2.h>

// plipbox extra open flags
#define S2PB_OPB_REQ_TIMING (8) /* exact request timing */
#define S2PB_OPF_REQ_TIMING (1<<S2PB_OPB_REQ_TIMING)

// structure for ios2_StatData when REQ_TIMING is enabled
struct s2pb_req_timing {
  S2QUAD  start_req;    /* start time of request in E clock */
  S2QUAD  start_op;     /* start of op */
  S2QUAD  end_op;
  S2QUAD  end_req;
};
typedef struct s2pb_req_timing s2pb_req_timing_t;

// structure for custom S2PB_REQUEST
struct s2pb_request
{
  UBYTE command;   // in
  UBYTE in_extra;  // in
  UBYTE status;    // out
  UBYTE out_extra;  // out
  UWORD in_size;   // in
  UWORD out_size;  // in: max_size, out: real_size
  APTR  in_data;
  APTR  out_data;
};
typedef struct s2pb_request s2pb_request_t;


#define S2PB_BASE 0x3000

// plipbox extra commands
#define S2PB_GET_VERSION    (S2PB_BASE + 0)
#define S2PB_DO_REQUEST     (S2PB_BASE + 1)

/*
  S2PB_GET_VERSION

    out:
      ios2_Req.io_Error: S2ERR_NOERR
      ios2_WireError: on success its the version

    Check if a plipbox device is here and return its version

    Version LONG contains both device and firmware version

                     Bits
    VER_DEV_MAJOR    31..24
    VER_DEV_MINOR    23..16
    VER_FW_MAJOR     15..8
    VER_FW_MINOR      7..0


  S2PB_DO_REQUEST

    in:
      ios2_DataLength: sizeof(s2pb_request)
      ios2_Data: pointer to s2pb_request
    out:
      ios2_WireError: number of parameters on device
       or
      ios2_Req.io_Error: S2ERR_NOERR, S2ERR_BAD_STATE
      ios2_WireError
      ios2_Data: out fields in s2pb_request

    Execute a custom request on the plipbox device.
*/

#endif

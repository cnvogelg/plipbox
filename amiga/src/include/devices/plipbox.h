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


#define S2PB_BASE 0x3000

// plipbox extra commands
#define S2PB_GET_VERSION (S2PB_BASE + 0)
#define S2PB_PARAM_GET_NUM (S2PB_BASE + 1)
#define S2PB_PARAM_FIND_TAG (S2PB_BASE + 2)
#define S2PB_PARAM_GET_DEF (S2PB_BASE + 3)
#define S2PB_PARAM_GET_VAL (S2PB_BASE + 4)
#define S2PB_PARAM_SET_VAL (S2PB_BASE + 5)
#define S2PB_PREFS_RESET (S2PB_BASE + 6)
#define S2PB_PREFS_LOAD (S2PB_BASE + 7)
#define S2PB_PREFS_SAVE (S2PB_BASE + 8)

#define S2PB_NO_INDEX 0xff

struct s2pb_param_def
{
  UBYTE index;
  UBYTE type;
  UBYTE format;
  UBYTE reserved;
  UWORD size;
  ULONG tag;
};
typedef struct s2pb_param_def s2pb_param_def_t;

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

  ----- parameter handling -----

  S2PB_PARAM_GET_NUM

    in:
      -
    out:
      ios2_WireError: number of parameters on device
       or
      ios2_Req.io_Error: S2ERR_NOERR, S2ERR_BAD_STATE
      ios2_WireError

    get the total number of configurable parameters

  S2PB_PARAM_FIND_TAG

    in:
      ios2_WireError: tag
    out:
      ios2.WireError: parameter index or S2PB_NO_INDEX
       or
      ios2_Req.io_Error: S2ERR_NOERR, S2ERR_BAD_STATE, S2ERR_SOFTWARE
      ios2_WireError: param error (S2ERR_SOFTWARE)

  S2PB_PARAM_GET_DEF

    in:
      ios2_WireError: parameter index (0 .. PARAM_GET_NUM-1)
      ios2_DataLength: sizeof(s2pb_param_def_t)
      ios2_Data: pointer to s2pb_param_def_t
    out:
      ios2_Req.io_Error: S2ERR_NOERR, S2ERR_BAD_STATE, S2ERR_SOFTWARE
      ios2_WireError: param error (S2ERR_SOFTWARE)

    Get a parameter description from the device

  S2PB_PARAM_GET_VAL

    in:
      ios2_WireError: parameter index (0 .. PARAM_GET_NUM-1)
      ios2_DataLength: sizeof parameter (from param_def_t.size)
      ios2_Data: pointer to parameter data (io_Length)
    out:
      ios2_Req.io_Error
      ios2_WireError:

    Get a parameter value

  S2PB_PARAM_SET_VAL

    in:
      ios2_WireError: parameter index (0 .. PARAM_GET_NUM-1)
      ios2_DataLength: sizeof parameter (from param_def_t.size)
      ios2_Data: pointer to parameter data (io_Length)
    out:
      ios2_Req.io_Error
      ios2_WireError:

    Set a parameter value

  ----- prefs -----

  S2PB_RESET_PREFS
  S2PB_LOAD_PREFS
  S2PB_SAVE_PREFS

    in:
      -
    out:
      ios2_Req.io_Error
      ios2_WireError: result code
*/

struct plipbox_param_def
{
  UBYTE index;
  UBYTE type;
  UBYTE format;
  UBYTE reserved;
  UWORD size;
  ULONG tag;
};
typedef struct plipbox_param_def plipbox_param_def_t;

// param type
#define S2PB_PARAM_TYPE_WORD 1
#define S2PB_PARAM_TYPE_LONG 2
#define S2PB_PARAM_TYPE_BYTE_ARRAY 3
#define S2PB_PARAM_TYPE_WORD_ARRAY 4
#define S2PB_PARAM_TYPE_LONG_ARRAY 5

// param format flags
#define S2PB_PARAM_FORMAT_HEX 1 // default value base is 16 instead of 10
#define S2PB_PARAM_FORMAT_BIN 2 // default value base is 2 instead of 10
#define S2PB_PARAM_FORMAT_STR 4 // data is given as null terminated string

#endif

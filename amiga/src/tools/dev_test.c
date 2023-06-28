/*
 * dev_test.c: test tool for device level testing of plipbox.device
 *
 * Written by
 *  Christian Vogelgsang <chris@vogelgsang.org>
 *
 * This file is part of plipbox.
 * See README for copyright notice.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307  USA.
 *
 */

#include <exec/types.h>
#include <exec/errors.h>
#include <exec/memory.h>
#include <exec/devices.h>
#include <dos/dos.h>
#include <dos/rdargs.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/alib_protos.h>

#include <stdlib.h>
#include <string.h>

#include <proto/exec.h>
#include <proto/dos.h>

#include <devices/sana2.h>
#include <devices/plipbox.h>

#include "compiler.h"
#include "atimer.h"

typedef UBYTE mac_t[6];
#define MAC_SIZE 6

/* SAS stuff */
extern struct ExecBase *SysBase;
#ifdef __SASC
extern struct DosLibrary *DOSBase;
#else
struct DosLibrary *DOSBase;
#endif

/* ---------- globals ---------------------------------------- */

static struct MsgPort *write_port = NULL;
static struct IOSana2Req *write_req = NULL;
static struct MsgPort *read_port = NULL;
static struct IOSana2Req *read_req = NULL;
static atimer_handle_t *atimer;

static struct Device *sana_dev = NULL;
static UBYTE *write_buf = NULL;
static UBYTE *read_buf = NULL;
static ULONG pkt_buf_size;
static UWORD frame_len = 0;
static ULONG data_offset = 0;

/* arg parsing */
static char *args_template =
  "-D=DEVICE/K,-U=UNIT/N/K,-M=MTU/N/K,-V=VERBOSE/S,-R=REPLY/S,-D=DELAY/N/K,-M=MODE/N/K,-F=FRAMELEN/N/K";
enum args_offset {
  DEVICE_ARG,
  UNIT_ARG,
  MTU_ARG,
  VERBOSE_ARG,
  REPLY_ARG,
  DELAY_ARG,
  MODE_ARG,
  FRAME_LEN_ARG,
  NUM_ARGS
};
static struct RDArgs *args_rd = NULL;
static LONG args_array[NUM_ARGS];

/* ---------- helpers ----------------------------------------- */

/* copy helper for SANA-II device */
static ASM SAVEDS int MemCopy(REG(a0,UBYTE *to),
         REG(a1,UBYTE *from),
         REG(d0,LONG len))
{
  CopyMem(from, to, len);
  return 1;
}

/* open sana device */
static BOOL open_device(char *name, ULONG unit, ULONG flags)
{
  static ULONG sana_tags[] = {
    S2_CopyToBuff, (ULONG)MemCopy,
    S2_CopyFromBuff, (ULONG)MemCopy,
    TAG_DONE, 0
  };

  /* create write port */
  write_port = CreateMsgPort();
  if(write_port == NULL) {
    PutStr((STRPTR)"Error creating write port!\n");
    return FALSE;
  }

  /* create read port */
  read_port = CreateMsgPort();
  if(read_port == NULL) {
    PutStr((STRPTR)"Error creating read port!\n");
  }

  /* create IO request */
  write_req = (struct IOSana2Req *)CreateIORequest(write_port, sizeof(struct IOSana2Req));
  if(write_req == NULL) {
    PutStr((STRPTR)"Error creatio IO write request!\n");
    return FALSE;
  }

  /* create IO request */
  read_req = (struct IOSana2Req *)CreateIORequest(read_port, sizeof(struct IOSana2Req));
  if(read_req == NULL) {
    PutStr((STRPTR)"Error creatio IO read request!\n");
    return FALSE;
  }

  /* store copy buffer pointers */
  write_req->ios2_BufferManagement = sana_tags;

  /* open device */
  if(OpenDevice((STRPTR)name, unit, (struct IORequest *)write_req, flags) != 0) {
    Printf((STRPTR)"Error opening device(%s,%lu)!\n", (ULONG)name, unit);
    return FALSE;
  }

  /* clone request */
  CopyMem(write_req, read_req, sizeof(struct IOSana2Req));
  /* restore msg port */
  read_req->ios2_Req.io_Message.mn_ReplyPort = read_port;

  /* fetch device */
  sana_dev = write_req->ios2_Req.io_Device;

  /* some device info */
  Printf((STRPTR)"[%s (%d.%d)]\n",
         (ULONG)sana_dev->dd_Library.lib_IdString,
         sana_dev->dd_Library.lib_Version,
         sana_dev->dd_Library.lib_Revision);

  return TRUE;
}

/* close sana device */
static void close_device(void)
{
  /* close device */
  if(sana_dev != NULL) {
    CloseDevice((struct IORequest *)write_req);
    sana_dev = NULL;
  }

  /* free IO request */
  if(write_req != NULL) {
    DeleteIORequest(write_req);
    write_req = NULL;
  }

  /* free IO request */
  if(read_req != NULL) {
    DeleteIORequest(read_req);
    read_req = NULL;
  }

  /* free msg port */
  if(write_port != NULL) {
    DeleteMsgPort(write_port);
    write_port = NULL;
  }

  /* free msg port */
  if(read_port != NULL) {
    DeleteMsgPort(read_port);
    read_port = NULL;
  }
}

static void sana_error(struct IOSana2Req *sana_req)
{
  UWORD error = sana_req->ios2_Req.io_Error;
  UWORD wire_error = sana_req->ios2_WireError;
  Printf((STRPTR)"IO failed: cmd=%04lx -> error=%d, wire_error=%d\n",
         sana_req->ios2_Req.io_Command, error, wire_error);  
}

static BOOL sana_cmd(struct IOSana2Req *sana_req, UWORD cmd)
{
  sana_req->ios2_Req.io_Command = cmd;

  if(DoIO((struct IORequest *)sana_req) != 0) {
    sana_error(sana_req);
    return FALSE;
  } else {
    return TRUE;
  }
}

static BOOL sana_online(void)
{
  return sana_cmd(write_req, S2_ONLINE);
}

static BOOL sana_offline(void)
{
  return sana_cmd(write_req, S2_OFFLINE);
}

static BOOL sana_get_station_address(mac_t cur_mac, mac_t def_mac)
{
  BOOL ok = sana_cmd(write_req, S2_GETSTATIONADDRESS);
  if(ok) {
    CopyMem(write_req->ios2_SrcAddr, cur_mac, MAC_SIZE);
    CopyMem(write_req->ios2_DstAddr, def_mac, MAC_SIZE);
  }
  return ok;
}

static BOOL plipbox_set_mac(mac_t new_mac)
{
  CopyMem(new_mac, write_req->ios2_SrcAddr, MAC_SIZE);
  return sana_cmd(write_req, S2PB_SET_MAC);
}

static BOOL plipbox_set_mode(UWORD mode)
{
  write_req->ios2_WireError = mode;
  return sana_cmd(write_req, S2PB_SET_MODE);
}

static BOOL plipbox_get_mode(UWORD *mode)
{
  BOOL ok = sana_cmd(write_req, S2PB_GET_MODE);
  if(ok) {
    *mode = write_req->ios2_WireError;
  }
  return ok;
}

static void fill_packet(void)
{
  /* fill packet */
  for(ULONG i=0;i<frame_len;i++) {
    UBYTE ch = (UBYTE)((i + data_offset) & 0xff);
    write_buf[i] = ch;
  }
}

static void check_packet(void)
{
  for(ULONG i=0;i<frame_len;i++) {
    UBYTE ch = (UBYTE)((i + data_offset) & 0xff);
    if(ch != read_buf[i]) {
      Printf("Mismatch: @%04lx: got=%02lx want=%02lx\n", i, (ULONG)read_buf[i], (ULONG)write_buf[i]);
    }
  }
}

static void send_packet(void)
{
  Printf((STRPTR)"Send packet... %lu bytes\n", (ULONG)frame_len);
  /* write request */
  write_req->ios2_Req.io_Command = CMD_WRITE;
  write_req->ios2_Req.io_Flags = 0; /*SANA2IOF_RAW;*/
  write_req->ios2_DataLength = frame_len;
  write_req->ios2_PacketType = 0x800;
  write_req->ios2_Data = write_buf;
  DoIO((struct IORequest *)write_req);
  PutStr((STRPTR)"Done\n");
}

static void dump_mac(STRPTR msg, mac_t mac)
{
  Printf("%s: %02lx:%02lx:%02lx:%02lx:%02lx:%02lx\n", msg,
    (ULONG)mac[0],
    (ULONG)mac[1],
    (ULONG)mac[2],
    (ULONG)mac[3],
    (ULONG)mac[4],
    (ULONG)mac[5]);
}

static atime_stamp_t ts_begin;

static void timing_begin(void) {
  atimer_eclock_get(atimer, &ts_begin);
}

static void timing_end(ULONG bytes) {
  atime_stamp_t ts_end;
  atime_stamp_t delta;
  atimer_eclock_get(atimer, &ts_end);
  atimer_eclock_delta(&ts_end, &ts_begin, &delta);
  ULONG us = atimer_eclock_to_us(atimer, delta.lo);
  ULONG kBps = atimer_eclock_to_kBps(atimer, delta.lo, bytes);
  Printf("Speed: bytes=%lu eclk=%lu us=%lu kBps=%lu\n", bytes, delta.lo, us, kBps);
}


static void reply_loop(void)
{
  ULONG wmask;
  ULONG verbose = args_array[VERBOSE_ARG];
  ULONG do_reply = args_array[REPLY_ARG];
  ULONG delay = 50;
  if(args_array[DELAY_ARG] != 0) {
    delay = *((ULONG *)args_array[DELAY_ARG]);
  }

  fill_packet();
  send_packet();

  PutStr((STRPTR)"Waiting for incoming packets...\n");
  if(do_reply) {
    Printf((STRPTR)"With reply after %ld ticks\n", delay);
  }

  for(;;) {
    /* read request */
    read_req->ios2_Req.io_Command = S2_READORPHAN; /*CMD_READ;*/
    read_req->ios2_Req.io_Flags = 0; /*SANA2IOF_RAW;*/
    read_req->ios2_DataLength = pkt_buf_size;
    read_req->ios2_PacketType = 0x800;
    read_req->ios2_Data = read_buf;
    BeginIO((struct IORequest *)read_req);
    wmask = Wait(SIGBREAKF_CTRL_C | (1UL << read_port->mp_SigBit));
    
    /* user break */
    if(wmask & SIGBREAKF_CTRL_C) {
      AbortIO((struct IORequest *)read_req);
      WaitIO((struct IORequest *)read_req);
      PutStr((STRPTR)"***Break\n");
      break;
    }

    /* got a packet? */
    if(WaitIO((struct IORequest *)read_req) != 0)
    {
      sana_error(read_req);
      break;
    } else {

      check_packet();

      if(verbose) {
        Printf((STRPTR)"Recv: size=%lu\n", read_req->ios2_DataLength);
      }

      if(do_reply) {
        /* swap adresses */
        CopyMem(read_req->ios2_SrcAddr, write_req->ios2_DstAddr, SANA2_MAX_ADDR_BYTES);
        CopyMem(read_req->ios2_DstAddr, write_req->ios2_SrcAddr, SANA2_MAX_ADDR_BYTES);

        Delay(delay);

        data_offset++;
        fill_packet();

        if(verbose) {
          PutStr((STRPTR)"Send\n");
        }

        /* send packet back */
        write_req->ios2_Req.io_Command = CMD_WRITE;
        write_req->ios2_Req.io_Flags = 0;
        write_req->ios2_DataLength = frame_len;
        write_req->ios2_PacketType = 0x800;
        write_req->ios2_Data = write_buf;
        timing_begin();
        if(DoIO((struct IORequest *)write_req) != 0) {
          sana_error(write_req);
          break;
        }
        timing_end(frame_len);
      }
    }
  }
}

/* ---------- main ---------- */
int main(void)
{
  BOOL ok = TRUE;
  ULONG unit;
  ULONG mtu;
  ULONG mode;
  char *dev_name;

#ifndef __SASC
  DOSBase = (struct DosLibrary *)OpenLibrary((STRPTR)"dos.library", 0L);
#endif

  /* parse args */
  args_rd = ReadArgs((STRPTR)args_template, args_array, NULL);
  if(args_rd == NULL) {
    PutStr((STRPTR)"Error parsing arguments!\n");
    exit(RETURN_ERROR);
  }

  /* parse device name and unit number */
  if(args_array[UNIT_ARG] != 0) {
    unit = *((ULONG *)args_array[UNIT_ARG]);
  } else {
    unit = 0;
  }
  if(args_array[DEVICE_ARG] != 0) {
    dev_name = (char *)args_array[DEVICE_ARG];
  } else {
    dev_name = "plipbox.device";
  }
  if(args_array[MTU_ARG] != 0) {
    mtu = *((ULONG *)args_array[MTU_ARG]);
  } else {
    mtu = 1500;
  }
  if(args_array[MODE_ARG] != 0) {
    mode = *((ULONG *)args_array[MODE_ARG]);
  } else {
    mode = S2PB_MODE_LOOPBACK_BUF;
  }
  if(args_array[FRAME_LEN_ARG] != 0) {
    frame_len = *((ULONG *)args_array[FRAME_LEN_ARG]);
  } else {
    frame_len = mtu;
  }

  /* alloc buffer */
  pkt_buf_size = mtu;
  write_buf = AllocMem(pkt_buf_size, MEMF_CLEAR);
  if(write_buf != NULL) {
    read_buf = AllocMem(pkt_buf_size, MEMF_CLEAR);
    if(read_buf != NULL) {
      /* open device */
      Printf((STRPTR)"device: %s:%lu\n", (ULONG)dev_name, unit);
      if(open_device(dev_name, unit, 0)) {
        atimer = atimer_init((struct Library *)SysBase);
        if(atimer != NULL) {

          /* set custom mac */
          mac_t my_mac = { 0xde, 0xad, 0xbe, 0xef, 0xba, 0xbe };
          plipbox_set_mac(my_mac);

          /* get mac again */
          mac_t cur_mac, def_mac;
          sana_get_station_address(cur_mac, def_mac);
          dump_mac("cur_mac", cur_mac);
          dump_mac("def_mac", def_mac);

          /* set mode */
          UWORD pb_mode = (UWORD)mode;
          plipbox_set_mode(pb_mode);
          plipbox_get_mode(&pb_mode);
          Printf("mode:%ld\n", (ULONG)pb_mode);

          /* set device online */
          if(sana_online()) {

            reply_loop();

            /* finally offline again */
            if(!sana_offline()) {
              PutStr((STRPTR)"Error going offline!\n");
            }
          } else {
            PutStr((STRPTR)"Error going online!\n");
          }

          atimer_exit(atimer);
        } else {
          PutStr((STRPTR)"Error opening timer!\n");
        }
      }
      close_device();

      /* free packet buffer */
      FreeMem(read_buf, pkt_buf_size);
    } else {
      PutStr((STRPTR)"Error allocating read_buf!\n");
    }
  } else {
    PutStr((STRPTR)"Error allocating write_buf!\n");
  }

  /* free args */
  FreeArgs(args_rd);

#ifndef __SASC
  CloseLibrary((struct Library *)DOSBase);
#endif

  /* return status */
  if(ok) {
    exit(RETURN_OK);
  } else {
    exit(RETURN_ERROR);
  }
}

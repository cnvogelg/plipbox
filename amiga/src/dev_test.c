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

#include "compiler.h"

/* SAS stuff */
extern struct ExecBase *SysBase;
#ifdef __SASC
extern struct DosLibrary *DOSBase;
#else
struct DosLibrary *DOSBase;
#endif

/* ---------- globals ---------------------------------------- */

static struct MsgPort *msg_port = NULL;
static struct IOSana2Req *sana_req = NULL;
static struct Device *sana_dev = NULL;
static UBYTE *pkt_buf = NULL;
static ULONG pkt_buf_size;

/* arg parsing */
static char *args_template =
  "-D=DEVICE/K,-U=UNIT/N/K,-M=MTU/N/K,-V=VERBOSE/S";
enum args_offset {
  DEVICE_ARG,
  UNIT_ARG,
  MTU_ARG,
  VERBOSE_ARG,
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

  /* create msg port */
  msg_port = CreateMsgPort();
  if(msg_port == NULL) {
  	PutStr((STRPTR)"Error creating msg port!\n");
  	return FALSE;
  }

  /* create IO request */
  sana_req = (struct IOSana2Req *)CreateIORequest(msg_port, sizeof(struct IOSana2Req));
  if(sana_req == NULL) {
  	PutStr((STRPTR)"Error creatio IO request!\n");
  	return FALSE;
  }

  /* store copy buffer pointers */
  sana_req->ios2_BufferManagement = sana_tags;

  /* open device */
  if(OpenDevice((STRPTR)name, unit, (struct IORequest *)sana_req, flags) != 0) {
  	Printf((STRPTR)"Error opening device(%s,%lu)!\n", (ULONG)name, unit);
  	return FALSE;
  }

  sana_dev = sana_req->ios2_Req.io_Device;

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
    CloseDevice((struct IORequest *)sana_req);
    sana_dev = NULL;
	}

  /* free IO request */
  if(sana_req != NULL) {
    DeleteIORequest(sana_req);
    sana_req = NULL;
  }

  /* free msg port */
  if(msg_port != NULL) {
    DeleteMsgPort(msg_port);
    msg_port = NULL;
  }
}

static void sana_error(void)
{
  UWORD error = sana_req->ios2_Req.io_Error;
  UWORD wire_error = sana_req->ios2_WireError;
  Printf((STRPTR)"IO failed: cmd=%04lx -> error=%d, wire_error=%d\n",
         sana_req->ios2_Req.io_Command, error, wire_error);  
}

static BOOL sana_cmd(UWORD cmd)
{
  sana_req->ios2_Req.io_Command = cmd;

  if(DoIO((struct IORequest *)sana_req) != 0) {
    sana_error();
    return FALSE;
  } else {
    return TRUE;
  }
}

static BOOL sana_online(void)
{
  return sana_cmd(S2_ONLINE);
}

static BOOL sana_offline(void)
{
  return sana_cmd(S2_OFFLINE);
}

static void reply_loop(void)
{
  ULONG wmask;
  ULONG verbose = args_array[VERBOSE_ARG];

  PutStr((STRPTR)"Waiting for incoming packets...\n");
  for(;;) {
    /* read request */
    sana_req->ios2_Req.io_Command = S2_READORPHAN; /*CMD_READ;*/
    sana_req->ios2_Req.io_Flags = 0; /*SANA2IOF_RAW;*/
    sana_req->ios2_DataLength = pkt_buf_size;
    /*sana_req->ios2_PacketType = type;*/
    sana_req->ios2_Data = pkt_buf;
    BeginIO((struct IORequest *)sana_req);
    wmask = Wait(SIGBREAKF_CTRL_C | (1UL << msg_port->mp_SigBit));
    
    /* user break */
    if(wmask & SIGBREAKF_CTRL_C) {
      AbortIO((struct IORequest *)sana_req);
      WaitIO((struct IORequest *)sana_req);
      PutStr((STRPTR)"***Break\n");
      break;
    }

    /* got a packet? */
    if(WaitIO((struct IORequest *)sana_req) != 0)
    {
      sana_error();
      break;
    } else {
      if(verbose) {
        PutStr((STRPTR)"+\n");
      }

      /* inconmig dst will be new src */
      memcpy(sana_req->ios2_SrcAddr, sana_req->ios2_DstAddr, SANA2_MAX_ADDR_BYTES);

      /* send packet back */
      sana_req->ios2_Req.io_Command = CMD_WRITE;
      sana_req->ios2_Req.io_Flags = 0;
      if(DoIO((struct IORequest *)sana_req) != 0) {
        sana_error();
        break;
      } else {
        if(verbose) {
          PutStr((STRPTR)"-\n");
        }
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

  /* alloc buffer */
  pkt_buf_size = mtu;
  pkt_buf = AllocMem(pkt_buf_size, MEMF_CLEAR);
  if(pkt_buf != NULL) {
    /* open device */
    Printf((STRPTR)"device: %s:%lu\n", (ULONG)dev_name, unit);
    if(open_device(dev_name, unit, 0)) {
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
    }
    close_device();

    /* free packet buffer */
    FreeMem(pkt_buf, pkt_buf_size);
  } else {
    PutStr((STRPTR)"Error allocating pkt_buf!\n");
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

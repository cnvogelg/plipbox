/*
 * udp_test.c: test tool for stack level test using UDP
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

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <exec/types.h>
#include <exec/errors.h>
#include <exec/memory.h>
#include <exec/devices.h>
#include <dos/dos.h>
#include <dos/rdargs.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/alib_protos.h>
#include <clib/socket_protos.h>
#include <clib/netlib_protos.h>

#include <stdlib.h>
#include <string.h>

#include <pragmas/exec_sysbase_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/socket_pragmas.h>

/* SAS stuff */
extern struct ExecBase *SysBase;
extern struct DosLibrary *DOSBase;
struct Library *SocketBase;
#define exit XCEXIT

/* ---------- globals ---------------------------------------- */

/* arg parsing */
static char *args_template =
  "-V=VERBOSE/S,-P=PORT/K/N";
enum args_offset {
  VERBOSE_ARG,
  PORT_ARG,
  NUM_ARGS
};
static struct RDArgs *args_rd = NULL;
static LONG args_array[NUM_ARGS];

/* data */
static UBYTE *pkt_buf = NULL;
static ULONG pkt_buf_size;

/* ---------- helpers ----------------------------------------- */

static void udp_main_loop(long fd)
{
  struct sockaddr_in c_addr;
  long c_addr_len = sizeof(c_addr);
  fd_set fds;

  FD_ZERO(&fds);
  FD_SET(fd, &fds);

  while(1) {
    /* wait for fd read or sigmask */
    ULONG bmask = SIGBREAKF_CTRL_C;
    long n = WaitSelect(fd + 1, &fds, NULL, NULL, NULL, &bmask);

    /* use break with Ctrl-C? */
    if(bmask & SIGBREAKF_CTRL_C) {
      PutStr("***Break\n");
      break;
    }

    /* a packet was received */
    if(n==1) {
      long res = recvfrom(fd, pkt_buf, pkt_buf_size, 0,
                          (struct sockaddr *)&c_addr, &c_addr_len);
      if(res < 0) {
        PutStr("Error in recvfrom!\n");
        break;
      }

      /* show some infos if verbose */
      if(args_array[VERBOSE_ARG]) {
        char *hostaddrp = inet_ntoa(c_addr.sin_addr);
        Printf("Got %ld bytes from %s!\n", res, hostaddrp);
      }

      /* send packet back */
      res = sendto(fd, pkt_buf, res, 0, 
                   (struct sockaddr *) &c_addr, c_addr_len);
      if (res < 0) {
        PutStr("Error in sendto!\n");
        break;
      }
    }
  }

}

static void udp_server(void)
{
  struct sockaddr_in s_addr;

  /* open socket */
  long sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if(sockfd >= 0) {
    long port = 6800;
    long optval = 1;

    /* overwrite port? */
    if(args_array[PORT_ARG] != 0) {
      port = *((LONG *)args_array[PORT_ARG]);
    }

    /* reuse sock addr option */
    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, 
                  (caddr_t)&optval , sizeof(long)) == -1) {
      PutStr("Warning: setsockopt failed?!\n");
    }

    /* setup address */
    bzero(&s_addr, sizeof(s_addr));
    s_addr.sin_family = AF_INET;
    s_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    s_addr.sin_port = htons((unsigned short)port);

    /* bind */
    if (bind(sockfd, (struct sockaddr *)&s_addr, sizeof(s_addr)) == 0) {
      Printf("Bound to port %ld\n", port);

      udp_main_loop(sockfd);

    } else {
      PutStr("Error binding socket!\n");
    }

    CloseSocket(sockfd);
  } else {
    PutStr("Eorror opening socket!\n");
  }
}

/* ---------- main -------------------------------------------- */
void __stdargs _main(char *cmdline)
{
  BOOL ok = TRUE;

  /* open socket library */
  SocketBase = OpenLibrary("bsdsocket.library", 3);
  if(SocketBase == NULL) {
    PutStr("Error opening 'bsdsocket.library'!\n");
    ok = FALSE;
  } else {
    /* parse args */
    args_rd = ReadArgs(args_template, args_array, NULL);
    if(args_rd == NULL) {
      PutStr("Error parsing arguments!\n");
      ok = FALSE;
    } else {
      /* alloc data buffer */
      pkt_buf_size = 1500;
      pkt_buf = AllocMem(pkt_buf_size, MEMF_CLEAR);
      if(pkt_buf != NULL) {

        udp_server();

        FreeMem(pkt_buf, pkt_buf_size);
      } else {
        PutStr("Error allocating packet buffer!\n");
      }
  
      FreeArgs(args_rd);
    }
    CloseLibrary(SocketBase);
  }

  /* return status */
  if(ok) {
    exit(RETURN_OK);
  } else {
    exit(RETURN_ERROR);
  }
}

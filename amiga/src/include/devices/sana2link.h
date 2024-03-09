/* sana2link.h

   an experimental extension to SANA-II that allows to deal with
   the link status of an ethernet controller.
*/

#ifndef SANA2_SANA2LINK_H
#define SANA2_SANA2LINK_H 1

/* new command */
#define S2_LINK_STATUS 0xC008

struct Sana2LinkStatus
{
   ULONG            s2ls_Size;
   WORD             s2ls_QueryMode;
   BYTE             s2ls_PreviousStatus;
   BYTE             s2ls_CurrentStatus;
   struct EClockVal s2ls_TimeStamp;
};

/* s2ls_QueryMode */
#define S2LS_QUERYMODE_IMMEDIATE 0
#define S2LS_QUERYMODE_ONCHANGE  1

/* s2ls_LinkStatus */
#define S2LINKSTATUS_UNKNOWN -1
#define S2LINKSTATUS_DOWN 0
#define S2LINKSTATUS_UP 1

/* new wire error */
#define S2WERR_LINK_DOWN 24 /* no carrier */

#endif

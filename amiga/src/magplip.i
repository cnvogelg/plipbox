   IFND    MAGPLIP_I
MAGPLIP_I     SET     1

;
;  $VER: magplip.i 1.9 (01 Apr 1998)
;
;  magplip.device - Parallel Line Internet Protocol
;
;  Original code written by Oliver Wagner and Michael Balzer.
;
;  This version has been completely reworked by Marius Gröger, introducing
;  slight protocol changes. The new source is a lot better organized and
;  maintainable.
;
;  Additional changes and code cleanup by Jan Kratochvil and Martin Mares.
;  The new source is significantly faster and yet better maintainable.
;
;  (C) Copyright 1993-1994 Oliver Wagner & Michael Balzer
;  (C) Copyright 1995 Jan Kratochvil & Martin Mares
;  (C) Copyright 1995-1996 Marius Gröger
;      All Rights Reserved
;
;  $HISTORY:
;
;  01 Apr 1998 : 001.009 :  integrated modifications for linPLIP from Stephane
;  10 Apr 1996 : 001.008 :  + PLIPF_REPLYSS didn't fit into byte
;                           + pb_ExtFlags
;  29 Mar 1996 : 001.007 :  changed copyright note
;  30 Dec 1995 : 001.006 :  single dynamic frame buffer
;  29 Dec 1995 : 001.005 :  + pb_Startup
;                        :  + new flag PLIPF_REPLYSS
;  03 Sep 1995 : 001.004 :  + removed PLIP(F|B)_SIDEA
;                           + hardware addressing fields in PLIPBase
;  30 Aug 1995 : 001.003 :  changed to match magplip.h
;  20 Aug 1995 : 001.002 :  + ASM parts really don't need to know about the
;                             configuration defaults.
;                           + using BITDEF macro from exec/types.i
;  13 Aug 1995 : 001.001 :  code cleanup
;  12 Feb 1995 : 001.000 :  created
;


    IFND EXEC_TYPES_I
    INCLUDE "exec/types.i"
    ENDC

    IFND EXEC_LISTS_I
    INCLUDE "exec/lists.i"
    ENDC

    IFND EXEC_INTERRUPTS_I
    INCLUDE "exec/interrupts.i"
    ENDC

    IFND EXEC_LIBRARIES_I
    INCLUDE "exec/libraries.i"
    ENDC

    IFND EXEC_SEMAPHORES_I
    INCLUDE "exec/semaphores.i"
    ENDC

    IFND DEVICES_TIMER_I
    INCLUDE "devices/timer.i"
    ENDC

    IFND DEVICES_SANA2_I
    INCLUDE "devices/sana2.i"
    ENDC

    IFND DOS_DOS_I
    INCLUDE "dos/dos.i"
    ENDC


;****************************************************************************


PLIP_MAXMTU         equ 8192
PLIP_ADDRFIELDSIZE  equ 1


;****************************************************************************


   ; Each data packet is put into this kind of frame before sent over the
   ; line. See below for definitions.
   ;

 IFD LINPLIP
 ; linux version
ETH_ALEN        equ     6

   STRUCTURE PLIPFrame,0
     SHORT    pf_Size
     STRUCT   pf_DstAddr,ETH_ALEN
     STRUCT   pf_SrcAddr,ETH_ALEN
     UWORD    pf_Type
;*** UBYTE    pf_Data[MTU];
     LABEL PLIPFrame_SIZE

PKTFRAMESIZE_1   equ     2
PKTFRAMESIZE_2   equ     (2+2*ETH_ALEN)

 ENDC

 IFND LINPLIP
   ; original version
   STRUCTURE PLIPFrame,0
     USHORT   pf_Sync
     SHORT    pf_Size
     USHORT   pf_CRC
     ULONG    pf_Type
;*** UBYTE    pf_Data[MTU];
     LABEL PLIPFrame_SIZE

PKTFRAMESIZE_1   equ     4
PKTFRAMESIZE_2   equ     6

 ENDC

SYNCBYTE_HEAD    equ     $42
SYNCBYTE_CRC     equ     $01
SYNCBYTE_NOCRC   equ     $02
SYNCWORD_CRC     equ     ((SYNCBYTE_HEAD<<8)|SYNCBYTE_CRC)
SYNCWORD_NOCRC   equ     ((SYNCBYTE_HEAD<<8)|SYNCBYTE_NOCRC)


;****************************************************************************


S2SS_TXERRORS     equ 0
S2SS_COLLISIONS   equ 1
S2SS_COUNT        equ 2


S2SS_PLIP_TXERRORS equ ((((S2WireType_PLIP) & $ffff) << 16) ! S2SS_TXERRORS)
S2SS_PLIP_COLLISIONS equ ((((S2WireType_PLIP) & $ffff) << 16) ! S2SS_COLLISIONS)


;****************************************************************************


   ;
   ; Central driver static storage. See Flags bits below.
   ;
   STRUCTURE PLIPBase,LIB_SIZE
     UBYTE  pb_Unit
     UBYTE  pb_pad1
     BPTR   pb_SegList
     APTR   pb_MiscBase
     APTR   pb_CIAABase
     APTR   pb_UtilityBase
     APTR   pb_TimerBase
     APTR   pb_DOSBase
     APTR   pb_SysBase
     APTR   pb_Startup
     APTR   pb_Server
     APTR   pb_Task
     STRUCT pb_Interrupt,IS_SIZE
     ULONG  pb_IntSig
     ULONG  pb_IntSigMask
     ULONG  pb_ServerStoppedSigMask
     APTR   pb_ServerPort
     APTR   pb_TimeoutPort
     APTR   pb_CollPort
     STRUCT pb_TimeoutReq,IOTV_SIZE
     STRUCT pb_CollReq,IOTV_SIZE
     STRUCT pb_DevStats,S2DS_SIZE
     STRUCT pb_SpecialStats,S2SSR_SIZE*S2SS_COUNT
     STRUCT pb_ReadList,LH_SIZE
     STRUCT pb_WriteList,LH_SIZE
     STRUCT pb_EventList,LH_SIZE
     STRUCT pb_ReadOrphanList,LH_SIZE
     STRUCT pb_TrackList,LH_SIZE
     STRUCT pb_BufferManagement,LH_SIZE
     STRUCT pb_EventListSem,SS_SIZE
     STRUCT pb_ReadListSem,SS_SIZE
     STRUCT pb_WriteListSem,SS_SIZE
     STRUCT pb_TrackListSem,SS_SIZE
     STRUCT pb_ReadOrphanListSem,SS_SIZE
     STRUCT pb_Lock,SS_SIZE
     ULONG  pb_Retries
     ULONG  pb_ReportBPS
     ULONG  pb_MTU
     ULONG  pb_AllocFlags
     ULONG  pb_Timeout;
     LONG   pb_CollisionDelay
     LONG   pb_ArbitrationDelay
     UBYTE  pb_TimeoutSet
     UBYTE  pb_Flags
     UWORD  pb_ExtFlags
     APTR   pb_OldExceptCode
     APTR   pb_OldExceptData
     ULONG  pb_OldExcept
 IFND LINPLIP
     STRUCT pb_HandshakeMask,2              ; must be changed when
     STRUCT pb_HandshakeBit,2               ; changing the datatype in .h
     STRUCT pb_SrcAddr,PLIP_ADDRFIELDSIZE
     STRUCT pb_DstAddr,PLIP_ADDRFIELDSIZE
 ENDC
     APTR   pb_Frame
   LABEL PLIPBase_SIZE

;
; Bits for PLIPBase->pb_Flags
;
   BITDEF PLIP,REPLYSS,0
   BITDEF PLIP,EXCLUSIVE,1
   BITDEF PLIP,NOTCONFIGURED,2
   BITDEF PLIP,OFFLINE,3
   BITDEF PLIP,SENDCRC,4
   BITDEF PLIP,RECEIVING,5
   BITDEF PLIP,COLLISION,6
   BITDEF PLIP,SERVERSTOPPED,7

;
; Bits for PLIPBase->pb_ExtFlags
;
   BITDEF PLIPE,NOSPECIALSTATS,0
   BITDEF PLIPE,NIBBLEACK,1                 ; only used by linport.asm

   ;
   ; Index for PLIPBase->pb_HandshakeBit[]
   ; Note: in contrast to the C-Version, this is actually not the index
   ; but the offset in the array _in_bytes_. Therefore, if you change the
   ; array's datatype, you need to adjust this.
   ;
HS_LINE    equ 0
HS_REQUEST equ 1  ; better: 1*sizeof(UBYTE)


;****************************************************************************

   ;
   ; Why isn't this in exec/types.i ?
   ;

   IFND TRUE
TRUE  equ 1
   ENDC
   IFND FALSE
FALSE equ 0
   ENDC


;****************************************************************************


   ENDC MAGPLIP_I

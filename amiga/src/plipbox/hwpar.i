   IFND    HWPAR_I
HWPAR_I     SET     1

    IFND EXEC_TYPES_I
    INCLUDE "exec/types.i"
    ENDC

    IFND EXEC_LISTS_I
    INCLUDE "exec/lists.i"
    ENDC


;****************************************************************************


PLIP_MAXMTU         equ 8192
PLIP_ADDRFIELDSIZE  equ 6


;****************************************************************************


   ; Each data packet is put into this kind of frame before sent over the
   ; line. See below for definitions.
   ;

   STRUCTURE HWFrame,0
     SHORT    hwf_Size

HWF_CMD_SEND     equ     $11
HWF_CMD_RECV     equ     $22
HWF_CMD_SEND_BURST equ   $33
HWF_CMD_RECV_BURST equ   $44

PKTFRAMESIZE_1   equ     4
PKTFRAMESIZE_2   equ     2
PKTFRAMESIZE_3   equ     14

SYNCBYTE_HEAD    equ     $42
SYNCBYTE_CRC     equ     $01
SYNCBYTE_NOCRC   equ     $02
SYNCWORD_CRC     equ     ((SYNCBYTE_HEAD<<8)|SYNCBYTE_CRC)
SYNCWORD_NOCRC   equ     ((SYNCBYTE_HEAD<<8)|SYNCBYTE_NOCRC)


;****************************************************************************

   ;
   ; begin of HWBase is shared with asm code
   ;
   STRUCTURE HWBase,0
     APTR   hwb_SysBase
     APTR   hwb_CIAABase
     APTR   hwb_Server
     ULONG  hwb_IntSigMask
     UWORD  hwb_MaxFrameSize
     UBYTE  hwb_TimeoutSet
     UBYTE  hwb_Flags
   LABEL HWBase_SIZE

   BITDEF HW,RECV_PENDING,0

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

   ENDC HWPAR_I

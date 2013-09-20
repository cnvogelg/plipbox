   IFND    MAGPLIP_I
MAGPLIP_I     SET     1

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

   STRUCTURE PLIPFrame,0
     USHORT   pf_Sync
     SHORT    pf_Size
     USHORT   pf_CRC
     LABEL PLIPFrame_CRC_Offset
     STRUCT   pf_DstAddr,PLIP_ADDRFIELDSIZE
     STRUCT   pf_SrcAddr,PLIP_ADDRFIELDSIZE
     USHORT   pf_Type
;*** UBYTE    pf_Data[MTU];

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
     UWORD  hwb_MaxMTU
     UBYTE  hwb_TimeoutSet
     UBYTE  hwb_Flags
   LABEL HWBase_SIZE

   BITDEF HW,RECV_PENDING,0
   BITDEF HW,SEND_CRC,2

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

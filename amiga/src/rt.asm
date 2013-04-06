;*
;*  $VER: rt.asm 1.3 (01 Apr 1998)
;*
;*  magplip.device - Parallel Line Internet Protocol
;*
;*  Original code written by Oliver Wagner and Michael Balzer.
;*
;*  This version has been completely reworked by Marius Gröger, introducing
;*  slight protocol changes. The new source is a lot better organized and
;*  maintainable.
;*
;*  Additional changes and code cleanup by Jan Kratochvil and Martin Mares.
;*  The new source is significantly faster and yet better maintainable.
;*
;*  (C) Copyright 1993-1994 Oliver Wagner & Michael Balzer
;*  (C) Copyright 1995 Jan Kratochvil & Martin Mares
;*  (C) Copyright 1995-1996 Marius Gröger
;*      All Rights Reserved
;*
;*  $HISTORY:
;*
;*  01 Apr 1998 : 001.003 :  integrated modifications for linPLIP from Stephane
;*  29 Mar 1996 : 001.002 :  changed copyright note
;*  20 Aug 1995 : 001.001 :  stub() MUST return 0 (jk/mm)
;*  12 Feb 1995 : 001.000 :  reworked original
;*

;*
;* include files
;*
    IFND EXEC_NODES_I
    INCLUDE "exec/nodes.i"
    ENDC
    IFND EXEC_RESIDENT_I
    INCLUDE "exec/resident.i"
    ENDC
    IFND EXEC_INITIALIZERS_I
    INCLUDE "exec/initializers.i"
    ENDC
    IFND DOS_DOS_I
    INCLUDE "dos/dos.i"
    ENDC

    INCLUDE "magplip.i"

; Revision informations
    INCLUDE "magplip_rev.i"
;*
;* imports
;*
    xref  _DevInit

    xref  _DevOpen
    xref  _DevClose
    xref  _DevExpunge
    xref  _DevExtFunc
    xref  _DevBeginIO
    xref  _DevAbortIO

    section "text",code

    moveq #RETURN_FAIL,d0
    rts

romtag:
    dc.w  RTC_MATCHWORD
    dc.l  romtag
    dc.l  endskip
    dc.b  RTF_AUTOINIT
    dc.b  VERSION
    dc.b  NT_DEVICE
    dc.b  0               ; priority
    dc.l  devname
    dc.l  devid
    dc.l  inittable

devname:
    dc.b "magplip.device",0
    dc.b  0,'$VER: '
devid:
    VSTRING

    cnop 0,2

inittable:
    dc.l  PLIPBase_SIZE
    dc.l  functable,datatable,_DevInit

functable:
    dc.l  _DevOpen
    dc.l  _DevClose
    dc.l  _DevExpunge
    dc.l  stub
    dc.l  _DevBeginIO
    dc.l  _DevAbortIO
    dc.l  -1

datatable:
    INITBYTE LN_TYPE,NT_DEVICE
    INITLONG LN_NAME,devname
    INITBYTE LIB_FLAGS,LIBF_SUMUSED!LIBF_CHANGED
    INITWORD LIB_VERSION,VERSION
    INITWORD LIB_REVISION,REVISION
    INITLONG LIB_IDSTRING,devid
    dc.w  0

stub:
    moveq #0,d0
    rts

endskip:

    END

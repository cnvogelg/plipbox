    IFND EXEC_NODES_I
    INCLUDE "exec/nodes.i"
    ENDC
    IFND EXEC_RESIDENT_I
    INCLUDE "exec/resident.i"
    ENDC
    IFND EXEC_INITIALIZERS_I
    INCLUDE "exec/initializers.i"
    ENDC
    IFND EXEC_LIBRARIES_I
    INCLUDE "exec/libraries.i"
    ENDC
    IFND DOS_DOS_I
    INCLUDE "dos/dos.i"
    ENDC


VERSION         EQU        0
REVISION        EQU        5
DATE    MACRO
              dc.b        '31.10.2013'
      ENDM
VERS    MACRO
              dc.b        'plipbox 0.5'
      ENDM
VSTRING MACRO
              dc.b        'plipbox 0.5 (31.10.2013)',13,10,0
      ENDM


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
    dc.b "plipbox.device",0
    dc.b  0,'$VER: '
devid:
    VSTRING

    cnop 0,2

inittable:
    dc.l  700   ; hack!! - use libsize tool to recalc!
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

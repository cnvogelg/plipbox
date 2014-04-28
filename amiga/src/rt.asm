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

    include "libsize.i"

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
    dc.b  DEVICE_VERSION
    dc.b  NT_DEVICE
    dc.b  0               ; priority
    dc.l  devname
    dc.l  devid
    dc.l  inittable

devname:
    dc.b DEVICE_NAME
    dc.b ".device",0
    dc.b  0,'$VER: '
devid:
    dc.b DEVICE_NAME
    dc.b " "
    dc.b DEVICE_VERSION
    dc.b "."
    dc.b DEVICE_REVISION
    dc.b " ("
    dc.b DEVICE_DATE
    dc.b ")",13,10,0

    cnop 0,2

inittable:
    dc.l  libsize
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
    INITWORD LIB_VERSION,DEVICE_VERSION
    INITWORD LIB_REVISION,DEVICE_REVISION
    INITLONG LIB_IDSTRING,devid
    dc.w  0

stub:
    moveq #0,d0
    rts

endskip:

    END

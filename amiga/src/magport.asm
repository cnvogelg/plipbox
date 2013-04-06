;
;  $VER: port.asm 1.7 (21 Feb 1998)
;
;  magplip.device - Parallel Line Internet Protocol
;
;  Assembler routines for efficient port handling.
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
;  21 Feb 1998 : 001.007 :  added SysBase to A6 in the interrupt handler
;  29 Mar 1996 : 001.006 :  changed copyright note
;  24 Feb 1996 : 001.005 :  + added PRTRSEL data direction signal
;                           + minor re-arrangements
;  30 Dec 1995 : 001.004 :  support for single frame buffer
;  05 Dec 1995 : 001.003 :  + short branches explicitly coded as such
;                           + some condition jumps which were effectively
;                             unconditional changed to bra's
;  30 Aug 1995 : 001.002 :  support for timer-timed timeout :-)
;  20 Aug 1995 : 001.001 :  + removed need for conditional compiling, as we
;                             we want a generic, symmetrical code
;                           + in interrupt handlers, A6 points already to
;                             SysBase
;                           + using JSRLIB/JMPLIB macros
;                           + removed implicit some assumptions on the values
;                             behind symbolic names
;  13 Feb 1995 : 001.000 :  created (in fact manually compiled from 'C'-
;                           original) (jk/mm)
;
      section "text",code


;----------------------------------------------------------------------------


      IFND HARDARE_CIA_I
      include "hardware/cia.i"
      ENDC

      IFND EXEC_MACROS_I
      include "exec/macros.i"
      ENDC

      IFND MAGPLIP_I
      include "magplip.i"
      ENDC


;----------------------------------------------------------------------------


      xref    _CRC16

      xdef    _interrupt
      xdef    _hwsend
      xdef    _hwrecv

;----------------------------------------------------------------------------

ciaa     equ     $bfe001
ciab     equ     $bfd000
BaseAX   equ     ciab+ciapra


SETCIAOUTPUT MACRO
      bset     #CIAB_PRTRSEL,ciab+ciapra-BaseAX(a5)   ; raise PRTSEL line
      st       ciaa+ciaddrb-BaseAX(\1)                ; data dir. => output
      ENDM

SETCIAINPUT MACRO
      bclr     #CIAB_PRTRSEL,ciab+ciapra-BaseAX(a5)   ; lower PRTSEL line
      sf       ciaa+ciaddrb-BaseAX(\1)                ; data dir. => input
      ENDM


;----------------------------------------------------------------------------
;
; NAME
;     interrupt() - ICR FLG interrupt server
;
; SYNOPSIS
;     void interrupt(struct PLIPBase *)
;                    A1
;
; FUNCTION
;     This is called from CIA resource on the receipt of an parallel port
;     FLG line interrupt. This is the case if the other side starts
;     transmission and writes the first byte to our port.
;
;     We recognise this here and propagate the information to the server
;     task by Signal()ing it and by setting the PLIPB_RECEIVING bit
;     in the flags field.
;
_interrupt:
	btst    #PLIPB_RECEIVING,pb_Flags(a1)
	bne.s   skipint
	move.b  pb_HandshakeBit+HS_LINE(a1),d0
	btst    d0,ciab+ciapra
	beq.s   skipint
	bset    #PLIPB_RECEIVING,pb_Flags(a1)
	move.l  pb_IntSigMask(a1),d0
	move.l  pb_SysBase(a1),a6
	move.l  pb_Server(a1),a1
	JSRLIB  Signal
skipint:
	moveq #0,d0
	rts

;----------------------------------------------------------------------------
;
; NAME
;     hwsend() - low level send routine
;
; SYNOPSIS
;     void hwsend(struct PLIPBase *)
;                  A0
;
; FUNCTION
;     This routine sends the PLIPBase->pb_Frame to the other side. It
;     cares for CRC encoding, if wanted. The frame header will be
;     set up except of the pf_Size field, which must be pre-initialized.
;
_hwsend:
	 movem.l  d2-d7/a2-a6,-(sp)
	 move.l   a0,a2                               ; a2 = PLIPBase
	 moveq    #FALSE,d2                           ; d2 = return value
	 moveq    #0,d3
	 move.l   d3,d4
	 move.b   pb_HandshakeBit+HS_REQUEST(a2),d3   ; d3 = HS_REQUEST
	 move.b   pb_HandshakeBit+HS_LINE(a2),d4      ; d4 = HS_LINE
	 move.l   pb_Frame(a2),a4                     ; a4 = Frame

	 ;
	 ; CRC wanted ?
	 ;
	 btst     #PLIPB_SENDCRC,pb_Flags(a2)
	 beq.s    hww_NoCRC
	 ; yes
	 move.w   #SYNCWORD_CRC,pf_Sync(a4)
	 lea      PLIPFrame_SIZE(a4),a0
	 move.w   pf_Size(a4),d0
	 subq.w   #PKTFRAMESIZE_2,d0
	 jsr      _CRC16(pc)
	 move.w   d0,pf_CRC(a4)
	 bra.s    hww_CRC

hww_NoCRC:
	 move.w   #SYNCWORD_NOCRC,pf_Sync(a4)

hww_CRC  move.l   pb_CIAABase(a2),a6
	 moveq    #CIAICRF_FLG,d0
	 JSRLIB   AbleICR                             ; DISABLEINT
	 lea      BaseAX,a5
	 SETCIAOUTPUT a5
	 move.b   (a5),d7                             ; SAMPLEINPUT, d7 = State
	 move.l   a4,a3
	 move.w   pf_Size(a4),d6
	 addq.w   #PKTFRAMESIZE_1-2,d6
	 move.b   (a3)+,ciaa+ciaprb-BaseAX(a5)        ; WRITECIA *p++
hww_LoopShake1:
	 move.b   (a5),d0                             ; ciab+ciapra
	 eor.b    d7,d0
	 btst     d4,d0                               ; WAITINPUTTOGGLE
	 bne.s    hww_cont1
	 tst.b    pb_TimeoutSet(a2)
	 beq.s    hww_LoopShake1
	 bra.s    hww_TimedOut
hww_cont1:
	 eor.b    d0,d7

hww_MainLoop:
	 move.b   (a3)+,ciaa+ciaprb-BaseAX(a5)        ; WRITECIA *p++
	 bchg     d3,(a5)                             ; OUTPUTTOGGLE
hww_LoopShake2:
	 move.b   (a5),d0                             ; ciab+ciapra
	 eor.b    d7,d0
	 btst     d4,d0                               ; WAITINPUTTOGGLE
	 bne.s    hww_cont2
	 tst.b    pb_TimeoutSet(a2)
	 beq.s    hww_LoopShake2
	 bra.s    hww_TimedOut
hww_cont2:
	 eor.b    d0,d7
	 dbra     d6,hww_MainLoop
	 moveq    #TRUE,d2                            ; rc = TRUE
hww_TimedOut:
	 SETCIAINPUT a5
	 bclr     d3,(a5)                             ; CLEARREQUEST ciab+ciapra
	 moveq    #CIAICRF_FLG,d0
	 JSRLIB   SetICR                              ; CLEARINT
	 move.w   #CIAICRF_FLG|CIAICRF_SETCLR,d0
	 JSRLIB   AbleICR                             ; ENABLEINT

	 move.l   d2,d0                               ; return rc
	 movem.l  (sp)+,d2-d7/a2-a6
Return   rts


;----------------------------------------------------------------------------
;
; NAME
;     hwrecv() - low level receive routine
;
; SYNOPSIS
;     void hwrecv(struct PLIPBase *)
;                 A0
;
; FUNCTION
;     This routine receives a magPLIP frame and stores it into
;     PLIPBase->pb_Frame. It cares for CRC decoding, if the incoming
;     packet is encoded. The pf_Size field will indicate the length
;     of the received data.
;
_hwrecv:
	 movem.l  d2-d7/a2-a6,-(sp)
	 move.l   a0,a2                               ; a2 = PLIPBase
	 moveq    #FALSE,d5                           ; d5 = return value
	 move.l   pb_CIAABase(a2),a6                  ; a6 = CIABase
	 moveq    #0,d3
	 move.l   d3,d2
	 move.b   pb_HandshakeBit+HS_REQUEST(a2),d3   ; d3 = HS_REQUEST
	 move.b   pb_HandshakeBit+HS_LINE(a2),d2      ; d2 = HS_LINE
	 lea      BaseAX,a5                           ; a5 = ciab+ciapra
	 move.l   pb_Frame(a2),a4                     ; a4 = Frame

	 moveq    #CIAICRF_FLG,d0
	 JSRLIB   AbleICR                             ; DISABLEINT

	 move.b   (a5),d7                             ; SAMPLEINPUT
	 cmp.b    #SYNCBYTE_HEAD,ciaa+ciaprb-BaseAX(a5) ; READCIABYTE
	 bne      hwr_TimedOut

	 bchg     d3,(a5)                             ; OUTPUTTOGGLE
hwr_LoopShake1:
	 move.b   (a5),d0                             ; ciab+ciapra
	 eor.b    d7,d0
	 btst     d2,d0                               ; WAITINPUTTOGGLE
	 bne.s    hwr_cont1
	 tst.b    pb_TimeoutSet(a2)
	 beq.s    hwr_LoopShake1
	 bra      hwr_TimedOut
hwr_cont1:
	 eor.b    d0,d7
	 move.b   ciaa+ciaprb-BaseAX(a5),d4           ; READCIABYTE
	 bchg     d3,(a5)                             ; OUTPUTTOGGLE
	 move.b   d4,d0
	 subq.b   #SYNCBYTE_CRC,d0
	 bcs.s    hwr_TimedOut
	 subq.b   #SYNCBYTE_NOCRC,d0
	 bcc.s    hwr_TimedOut
	 lea      pf_Size(a4),a3

	 ; Read 1st length byte
	 ;
hwr_LoopShake2
	 move.b   (a5),d0                             ; ciab+ciapra
	 eor.b    d7,d0
	 btst     d2,d0                               ; WAITINPUTTOGGLE
	 bne.s    hwr_cont2
	 tst.b    pb_TimeoutSet(a2)
	 beq.s    hwr_LoopShake2
	 bra.s    hwr_TimedOut
hwr_cont2:
	 eor.b    d0,d7
	 move.b   ciaa+ciaprb-BaseAX(a5),(a3)+        ; READCIABYTE
	 bchg     d3,(a5)                             ; OUTPUTTOGGLE

	 ; Read 2nd length byte
	 ;
hwr_LoopShake3:
	 move.b   (a5),d0                             ; ciab+ciapra
	 eor.b    d7,d0
	 btst     d2,d0                               ; WAITINPUTTOGGLE
	 bne.s    hwr_cont3
	 tst.b    pb_TimeoutSet(a2)
	 beq.s    hwr_LoopShake3
	 bra.s    hwr_TimedOut
hwr_cont3:
	 eor.b    d0,d7
	 move.b   ciaa+ciaprb-BaseAX(a5),(a3)+        ; READCIABYTE
	 bchg     d3,(a5)                             ; OUTPUTTOGGLE ciab+ciapra

	 move.w   -2(a3),d6                           ; = length
	 subq.w   #PKTFRAMESIZE_2,d6
	 bcs.s    hwr_TimedOut
	 cmp.w    pb_MTU+2(a2),d6
	 bhi.s    hwr_TimedOut
	 addq.w   #PKTFRAMESIZE_2-1,d6

	 ; Read main packet body
	 ;
hwr_MainLoop:
hwr_LoopShake4:
	 move.b   (a5),d0                             ; ciab+ciapra
	 eor.b    d7,d0
	 btst     d2,d0
	 bne.s    hwr_cont4
	 tst.b    pb_TimeoutSet(a2)
	 beq.s    hwr_LoopShake4
	 bra.s    hwr_TimedOut
hwr_cont4:
	 eor.b    d0,d7
	 move.b   ciaa+ciaprb-BaseAX(a5),(a3)+        ; READCIABYTE
	 bchg     d3,(a5)                             ; OUTPUTTOGGLE ciab+ciapra
	 dbra     d6,hwr_MainLoop

hwr_DoneRead:
	 subq.b   #SYNCBYTE_CRC,d4
	 bne.s    hwr_ReadOkay
	 lea      PLIPFrame_SIZE(a4),a0
	 move.w   pf_Size(a4),d0
	 subq.w   #PKTFRAMESIZE_2,d0
	 jsr      _CRC16(pc)
	 cmp.w    pf_CRC(a4),d0
	 bne.s    hwr_TimedOut

hwr_ReadOkay:
	 moveq    #TRUE,d5
hwr_TimedOut:
	 bclr     #PLIPB_RECEIVING,pb_Flags(a2)
	 SETCIAINPUT a5
	 bclr     d3,(a5)                             ; CLEARREQUEST ciab+ciapra
	 moveq    #CIAICRF_FLG,d0
	 JSRLIB   SetICR                              ; CLEARINT
	 move.w   #CIAICRF_FLG|CIAICRF_SETCLR,d0
	 JSRLIB   AbleICR                             ; ENABLEINT

	 move.l   d5,d0                               ; return value
	 movem.l  (sp)+,d2-d7/a2-a6
	 rts

	 end

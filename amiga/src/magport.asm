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

HS_LINE_BIT     equ     CIAB_PRTRBUSY;
HS_REQUEST_BIT  equ     CIAB_PRTRPOUT;

SETSELECT MACRO
      bset     #CIAB_PRTRSEL,ciab+ciapra-BaseAX(a5)   ; raise PRTSEL line
      ENDM

CLRSELECT MACRO
      bclr     #CIAB_PRTRSEL,ciab+ciapra-BaseAX(a5)   ; lower PRTSEL line
      ENDM

SETCIAOUTPUT MACRO
      st       ciaa+ciaddrb-BaseAX(\1)                ; data dir. => output
      ENDM

SETCIAINPUT MACRO
      sf       ciaa+ciaddrb-BaseAX(\1)                ; data dir. => input
      ENDM


;----------------------------------------------------------------------------
;
; NAME
;     interrupt() - ICR FLG interrupt server
;
; SYNOPSIS
;     void interrupt(struct HWBase *)
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
        btst    #HWB_RECV_PENDING,hwb_Flags(a1)
        bne.s   skipint
        moveq   #HS_LINE_BIT,d0
        btst    d0,ciab+ciapra
        beq.s   skipint
        bset    #HWB_RECV_PENDING,hwb_Flags(a1)
        move.l  hwb_IntSigMask(a1),d0
        move.l  hwb_SysBase(a1),a6
        move.l  hwb_Server(a1),a1
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
;     void hwsend(struct HWBase *, struct HWFrame *)
;                 A0               A1
;
; FUNCTION
;     This routine sends the PLIPBase->pb_Frame to the other side. It
;     cares for CRC encoding, if wanted. The frame header will be
;     set up except of the pf_Size field, which must be pre-initialized.
;
_hwsend:
         movem.l  d2-d7/a2-a6,-(sp)
         move.l   a0,a2                               ; a2 = HWBase
         move.l   a1,a4                               ; a4 = Frame
         moveq    #FALSE,d2                           ; d2 = return value
         moveq    #0,d3
         move.l   d3,d4
         moveq    #HS_REQUEST_BIT,d3                  ; d3 = HS_REQUEST
         moveq    #HS_LINE_BIT,d4                     ; d4 = HS_LINE

         ;
         ; CRC wanted ?
         ;
         btst     #HWB_SEND_CRC,hwb_Flags(a2)
         beq.s    hww_NoCRC
         ; yes
         move.w   #SYNCWORD_CRC,pf_Sync(a4)
         lea      PLIPFrame_CRC_Offset(a4),a0
         move.w   pf_Size(a4),d0
         subq.w   #PKTFRAMESIZE_2,d0
         jsr      _CRC16(pc)
         move.w   d0,pf_CRC(a4)
         bra.s    hww_CRC

hww_NoCRC:
         move.w   #SYNCWORD_NOCRC,pf_Sync(a4)

hww_CRC  move.l   hwb_CIAABase(a2),a6
         moveq    #CIAICRF_FLG,d0
         JSRLIB   AbleICR                             ; DISABLEINT
         lea      BaseAX,a5
         SETSELECT
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
         tst.b    hwb_TimeoutSet(a2)
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
         tst.b    hwb_TimeoutSet(a2)
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
         CLRSELECT

         move.l   d2,d0                               ; return rc
         movem.l  (sp)+,d2-d7/a2-a6
Return   rts


;----------------------------------------------------------------------------
;
; NAME
;     hwrecv() - low level receive routine
;
; SYNOPSIS
;     void hwrecv(struct HWBase *, struct HWFrame *)
;                 A0               A1
;
; FUNCTION
;     This routine receives a magPLIP frame and stores it into
;     PLIPBase->pb_Frame. It cares for CRC decoding, if the incoming
;     packet is encoded. The pf_Size field will indicate the length
;     of the received data.
;
_hwrecv:
         movem.l  d2-d7/a2-a6,-(sp)
         move.l   a0,a2                               ; a2 = HWBase
         move.l   a1,a4
         moveq    #FALSE,d5                           ; d5 = return value
         move.l   hwb_CIAABase(a2),a6                 ; a6 = CIABase
         moveq    #0,d3
         move.l   d3,d2
         moveq    #HS_REQUEST_BIT,d3                  ; d3 = HS_REQUEST
         moveq    #HS_LINE_BIT,d2                     ; d2 = HS_LINE
         lea      BaseAX,a5                           ; a5 = ciab+ciapra

         moveq    #CIAICRF_FLG,d0
         JSRLIB   AbleICR                             ; DISABLEINT
         SETSELECT

         move.b   (a5),d7                             ; SAMPLEINPUT
         cmp.b    #SYNCBYTE_HEAD,ciaa+ciaprb-BaseAX(a5) ; READCIABYTE
         bne      hwr_TimedOut

         bchg     d3,(a5)                             ; OUTPUTTOGGLE
hwr_LoopShake1:
         move.b   (a5),d0                             ; ciab+ciapra
         eor.b    d7,d0
         btst     d2,d0                               ; WAITINPUTTOGGLE
         bne.s    hwr_cont1
         tst.b    hwb_TimeoutSet(a2)
         beq.s    hwr_LoopShake1
         bra      hwr_TimedOut
hwr_cont1:
         eor.b    d0,d7
         move.b   ciaa+ciaprb-BaseAX(a5),d4           ; READCIABYTE
         bchg     d3,(a5)                             ; OUTPUTTOGGLE
         move.b   d4,d0
         subq.b   #SYNCBYTE_CRC,d0
         bcs      hwr_TimedOut
         subq.b   #SYNCBYTE_NOCRC,d0
         bcc      hwr_TimedOut
         lea      pf_Size(a4),a3

         ; Read 1st length byte
         ;
hwr_LoopShake2
         move.b   (a5),d0                             ; ciab+ciapra
         eor.b    d7,d0
         btst     d2,d0                               ; WAITINPUTTOGGLE
         bne.s    hwr_cont2
         tst.b    hwb_TimeoutSet(a2)
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
         tst.b    hwb_TimeoutSet(a2)
         beq.s    hwr_LoopShake3
         bra.s    hwr_TimedOut
hwr_cont3:
         eor.b    d0,d7
         move.b   ciaa+ciaprb-BaseAX(a5),(a3)+        ; READCIABYTE
         bchg     d3,(a5)                             ; OUTPUTTOGGLE ciab+ciapra

         move.w   -2(a3),d6                           ; = length
         sub.w    #PKTFRAMESIZE_2+PKTFRAMESIZE_3,d6   ; check MTU size (without eth hdr)
         bcs.s    hwr_TimedOut
         cmp.w    hwb_MaxMTU(a2),d6
         bhi.s    hwr_TimedOut
         add.w    #PKTFRAMESIZE_2+PKTFRAMESIZE_3-1,d6

         ; Read main packet body
         ;
hwr_MainLoop:
hwr_LoopShake4:
         move.b   (a5),d0                             ; ciab+ciapra
         eor.b    d7,d0
         btst     d2,d0
         bne.s    hwr_cont4
         tst.b    hwb_TimeoutSet(a2)
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
         lea      PLIPFrame_CRC_Offset(a4),a0
         move.w   pf_Size(a4),d0
         subq.w   #PKTFRAMESIZE_2,d0
         jsr      _CRC16(pc)
         cmp.w    pf_CRC(a4),d0
         bne.s    hwr_TimedOut

hwr_ReadOkay:
         moveq    #TRUE,d5
hwr_TimedOut:
         bclr     #HWB_RECV_PENDING,hwb_Flags(a2)
         SETCIAINPUT a5
         bclr     d3,(a5)                             ; CLEARREQUEST ciab+ciapra
         moveq    #CIAICRF_FLG,d0
         JSRLIB   SetICR                              ; CLEARINT
         move.w   #CIAICRF_FLG|CIAICRF_SETCLR,d0
         JSRLIB   AbleICR                             ; ENABLEINT
  CLRSELECT

         move.l   d5,d0                               ; return value
         movem.l  (sp)+,d2-d7/a2-a6
         rts

         end

      section "text",code

      IFND HARDARE_CIA_I
      include "hardware/cia.i"
      ENDC

      IFND EXEC_MACROS_I
      include "exec/macros.i"
      ENDC

      IFND HWPAR_I
      include "hwpar.i"
      ENDC


      xdef    _interrupt
      xdef    _hwburstsend
      xdef    _hwburstrecv


ciaa     equ     $bfe001
ciab     equ     $bfd000
BaseAX   equ     ciab+ciapra

HS_RAK_BIT      equ     CIAB_PRTRBUSY
HS_REQ_BIT      equ     CIAB_PRTRPOUT

SETSELECT MACRO
      bset     #CIAB_PRTRSEL,ciab+ciapra-BaseAX(\1)   ; raise PRTSEL line
      ENDM

CLRSELECT MACRO
      bclr     #CIAB_PRTRSEL,ciab+ciapra-BaseAX(\1)   ; lower PRTSEL line
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
;     FLG line interrupt.
;
;     If the server has a frame ready to be send out then it signals this
;     condition by triggering the FLG interrupt.
;
_interrupt:
        btst    #HWB_RECV_PENDING,hwb_Flags(a1)
        bne.s   skipint
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
;     hwburstsend() - low level send routine in burst mode
;
; SYNOPSIS
;     void hwburstsend(struct HWBase *, struct HWFrame *)
;                      A0               A1
;
; FUNCTION
;     This functions sends a HW frame with the plipbox protocol via
;     the parallel port and uses the fast burst protocol
;     burstSize in words
_hwburstsend:
         movem.l  d2-d7/a2-a6,-(sp)
         move.l   a0,a2                               ; a2 = HWBase
         move.l   a1,a3                               ; a3 = Frame
         moveq    #FALSE,d2                           ; d2 = return value
         moveq    #HS_REQ_BIT,d3                      ; d3 = HS_REQ
         moveq    #HS_RAK_BIT,d4                      ; d4 = HS_RAK
         lea      BaseAX,a5                           ; a5 = CIA HW base

         ; a4 = data reg of CIA
         move.l   a5,a4
         add.l    #(ciaa+ciaprb-BaseAX),a4

         ; --- size calc for burst
         ; packet size (in bytes) rounded to words (d6)
         move.w   (a3),d6
         subq.w   #1,d6
         lsr.w    #1,d6                               ; d6 = packet size in words - 1

         ; --- prepare
         ; Wait RAK == 0
bww_WaitRak1:
         move.b   (a5),d0                             ; ciab+ciapra
         btst     d4,d0
         beq.s    bww_RakOk1
         ; check for timeout
         tst.b    hwb_TimeoutSet(a2)
         beq.s    bww_WaitRak1
         bra      bww_ExitError
bww_RakOk1:
         ; --- init handshake
         ; [OUT]
         SETCIAOUTPUT a5

         ; Set <CMD_SEND_BURST>
         move.b   #HWF_CMD_SEND_BURST,(a4)

         ; Set SEL = 1 -> Trigger Plipbox
         SETSELECT a5

         ; --- send size (without burst)
         ; Wait RAK == 1
bww_WaitRak2a:
         move.b   (a5),d0                             ; ciab+ciapra
         btst     d4,d0                               ; RAK toggled?
         bne.s    bww_RakOk2a
         ; check for timeout
         tst.b    hwb_TimeoutSet(a2)
         beq.s    bww_WaitRak2a
         bra.s    bww_ExitError
bww_RakOk2a:
         ; Set <size> hi byte
         move.b   (a3)+,(a4)                          ; write data to port
         ; Toggle REQ
         bset     d3,(a5)                             ; set REQ=1

         ; Wait RAK == 0
bww_WaitRak2b:
         move.b   (a5),d0                             ; ciab+ciapra
         btst     d4,d0                               ; RAK toggled?
         beq.s    bww_RakOk2b
         ; check for timeout
         tst.b    hwb_TimeoutSet(a2)
         beq.s    bww_WaitRak2b
         bra.s    bww_ExitError
bww_RakOk2b:
         ; Set <size> lo byte
         move.b   (a3)+,(a4)                          ; write data to port
         ; Toggle REQ
         bclr     d3,(a5)                             ; set REQ=0

         ; ---- burst enter sync
         ; Wait RAK == 1 (sync before burst)
bww_WaitRak3a:
         move.b   (a5),d0                             ; ciab+ciapra
         btst     d4,d0                               ; RAK toggled?
         bne.s    bww_RakOk3a
         ; check for timeout
         tst.b    hwb_TimeoutSet(a2)
         beq.s    bww_WaitRak3a
         bra.s    bww_ExitError
bww_RakOk3a:

         ; disable all irq
         JSRLIB   Disable

         ; --- burst loop begin
bww_BurstLoop:
         ; set even data 0,2,4,...
         move.b   (a3)+,(a4)                          ; write data to port
         ; Toggle REQ
         bset     d3,(a5)                             ; set REQ=1

         ; set odd data 1,3,5,...
         move.b   (a3)+,(a4)                          ; write data to port
         ; Toggle REQ
         bclr     d3,(a5)                             ; set REQ=1

         dbra     d6,bww_BurstLoop
         ; --- burst loop end

         ; enable all irq
         JSRLIB   Enable

         bset     d3,(a5)                             ; set REQ=1

         ; ---- burst exit sync
         ; Wait RAK == 0 (sync after burst)
bww_WaitRak3b:
         move.b   (a5),d0                             ; ciab+ciapra
         btst     d4,d0                               ; RAK toggled?
         beq.s    bww_RakOk3b
         ; check for timeout
         tst.b    hwb_TimeoutSet(a2)
         beq.s    bww_WaitRak3b
         bra.s    bww_ExitError
bww_RakOk3b:

         bclr     d3,(a5)                             ; set REQ=0

         ; --- wait final RAK
         ; final Wait RAK == 1
bww_WaitRak4:
         move.b   (a5),d0                             ; ciab+ciapra
         btst     d4,d0                               ; RAK toggled?
         bne.s    bww_ExitOk
         ; check for timeout
         tst.b    hwb_TimeoutSet(a2)
         beq.s    bww_WaitRak4
         bra.s    bww_ExitError

         ; --- exit
bww_ExitOk:
         moveq    #TRUE,d2                            ; rc = TRUE
bww_ExitError:

         ; [IN]
         SETCIAINPUT a5

         ; SEL = 0
         CLRSELECT a5

         move.l   d2,d0                               ; return rc
         movem.l  (sp)+,d2-d7/a2-a6
         rts

;----------------------------------------------------------------------------
;
; NAME
;     hwburstrecv() - low level receive routine in burst mode
;
; SYNOPSIS
;     void hwburstrecv(struct HWBase *, struct HWFrame *)
;                      A0               A1
;
; FUNCTION
;     This functions receives a HW frame with the plipbox protocol via
;     the parallel port and uses the fast burst protocol
;     burstSize = words
_hwburstrecv:
         movem.l  d2-d7/a2-a6,-(sp)
         move.l   a0,a2                               ; a2 = HWBase
         move.l   a1,a3                               ; a3 = Frame
         move.w   d0,d5                               ; d5 = burstSize in words
         moveq    #FALSE,d2                           ; d2 = return value
         moveq    #HS_REQ_BIT,d3                      ; d3 = HS_REQ
         moveq    #HS_RAK_BIT,d4                      ; d4 = HS_RAK
         lea      BaseAX,a5                           ; a5 = CIA HW base

         ; a4 = data reg of CIA
         move.l   a5,a4
         add.l    #(ciaa+ciaprb-BaseAX),a4

         ; --- prepare
         ; Wait RAK == 0
bwr_WaitRak1:
         move.b   (a5),d0                             ; ciab+ciapra
         btst     d4,d0
         beq.s    bwr_RakOk1
         ; check for timeout
         tst.b    hwb_TimeoutSet(a2)
         beq.s    bwr_WaitRak1
         bra      bwr_ExitError
bwr_RakOk1:
         ; --- init handshake
         ; [OUT]
         SETCIAOUTPUT a5

         ; Set <CMD_RECV_BURST>
         move.b   #HWF_CMD_RECV_BURST,(a4)

         ; Set SEL = 1 -> Trigger Plipbox
         SETSELECT a5

         ; --- toggle to input
         ; Wait RAK == 1
bwr_WaitRak2a:
         move.b   (a5),d0                             ; ciab+ciapra
         btst     d4,d0                               ; RAK toggled?
         bne.s    bwr_RakOk2a
         ; check for timeout
         tst.b    hwb_TimeoutSet(a2)
         beq.s    bwr_WaitRak2a
         bra      bwr_ExitError
bwr_RakOk2a:
         ; [IN]
         SETCIAINPUT a5
         ; Toggle REQ
         bset     d3,(a5)                             ; set REQ=1

         ; --- read size word ---
         ; Wait RAK == 0
bwr_WaitRak2b:
         move.b   (a5),d0                             ; ciab+ciapra
         btst     d4,d0                               ; RAK toggled?
         beq.s    bwr_RakOk2b
         ; check for timeout
         tst.b    hwb_TimeoutSet(a2)
         beq.s    bwr_WaitRak2b
         bra.s    bwr_ExitError
bwr_RakOk2b:

         ; Read <Size_Hi>
         move.b   (a4),(a3)+                          ; read par port
         ; Set REQ = 0
         bclr     d3,(a5)                             ; REQ toggle

         ; Wait RAK == 1
bwr_WaitRak2c:
         move.b   (a5),d0                             ; ciab+ciapra
         btst     d4,d0
         bne.s    bwr_RakOk2c
         ; check for timeout
         tst.b    hwb_TimeoutSet(a2)
         beq.s    bwr_WaitRak2c
         bra.s    bwr_ExitError
bwr_RakOk2c:
         ; Read <Size_Lo>
         move.b   (a4),(a3)+                          ; READCIABYTE
         ; Set REQ = 1
         bset     d3,(a5)                             ; REQ toggle

         ; --- check size
         ; now fetch full size word and check for max frame size
         move.w   -2(a3),d6                           ; = length
         tst.w    d6
         beq.s    bwr_ExitOk                          ; empty size? ok
         cmp.w    hwb_MaxFrameSize(a2),d6             ; buffer too large
         bhi.s    bwr_ExitError

         ; convert packet size (d6) to words-1 (and round up if necessary)
         subq.w   #1,d6
         lsr.w    #1,d6

         ; ---- burst enter
         ; Wait RAK == 0 (sync before burst)
bwr_WaitRak3a:
         move.b   (a5),d0                             ; ciab+ciapra
         btst     d4,d0                               ; RAK toggled?
         beq.s    bwr_RakOk3a
         ; check for timeout
         tst.b    hwb_TimeoutSet(a2)
         beq.s    bwr_WaitRak3a
         bra.s    bwr_ExitError
bwr_RakOk3a:

         ; disable all irq
         JSRLIB   Disable

         ; --- burst loop begin
bwr_BurstLoop:
         ; Toggle REQ
         bclr     d3,(a5)                             ; set REQ=0
         ; get even data 0,2,4,...
         move.b   (a4),(a3)+                          ; read data from port

         ; Toggle REQ
         bset     d3,(a5)                             ; set REQ=1
         ; get odd data 1,3,5,...
         move.b   (a4),(a3)+                          ; read data from port

         dbra     d6,bwr_BurstLoop
         ; --- burst loop end

         ; enable all irq
         JSRLIB   Enable

         bclr     d3,(a5)                             ; set REQ=0

         ; ---- burst leave
         ; Wait RAK == 1 (sync after burst)
bwr_WaitRak3b:
         move.b   (a5),d0                             ; ciab+ciapra
         btst     d4,d0                               ; RAK toggled?
         bne.s    bwr_RakOk3b
         ; check for timeout
         tst.b    hwb_TimeoutSet(a2)
         beq.s    bwr_WaitRak3b
         bra.s    bwr_ExitError
bwr_RakOk3b:

         bset     d3,(a5)                             ; set REQ=1

         ; --- wait final RAK
         ; final Wait RAK == 0
bwr_WaitRak4:
         move.b   (a5),d0                             ; ciab+ciapra
         btst     d4,d0                               ; RAK toggled?
         beq.s    bwr_ExitOk
         ; check for timeout
         tst.b    hwb_TimeoutSet(a2)
         beq.s    bwr_WaitRak4
         bra.s    bwr_ExitError

         ; --- exit
bwr_ExitOk:
         moveq    #TRUE,d2                            ; rc = TRUE
bwr_ExitError:

         ; reset signal
         moveq    #0,d0
         move.l   hwb_IntSigMask(a1),d1
         JSRLIB   SetSignal

         ; clear RECV_PENDING flag set by irq
         bclr     #HWB_RECV_PENDING,hwb_Flags(a2)

         ; clear REQ
         bclr     d3,(a5)

         ; SEL = 0
         CLRSELECT a5

         move.l   d2,d0                               ; return rc
         movem.l  (sp)+,d2-d7/a2-a6
         rts

         end

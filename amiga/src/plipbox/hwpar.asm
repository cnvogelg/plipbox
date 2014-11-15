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
      xdef    _hwsend
      xdef    _hwrecv
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
;     hwsend() - low level send routine
;
; SYNOPSIS
;     void hwsend(struct HWBase *, struct HWFrame *)
;                 A0               A1
;
; FUNCTION
;     This functions sends a HW frame with the plipbox protocol via
;     the parallel port
;
_hwsend:
         movem.l  d2-d7/a2-a6,-(sp)
         move.l   a0,a2                               ; a2 = HWBase
         move.l   a1,a3                               ; a3 = Frame
         moveq    #FALSE,d2                           ; d2 = return value
         moveq    #HS_REQ_BIT,d3                      ; d3 = HS_REQ
         moveq    #HS_RAK_BIT,d4                      ; d4 = HS_RAK
         lea      BaseAX,a5                           ; a5 = CIA HW base

         ; --- prepare
         ; Wait RAK == 0
hww_WaitRak1:
         move.b   (a5),d0                             ; ciab+ciapra
         btst     d4,d0
         beq.s    hww_RakOk1
         ; check for timeout
         tst.b    hwb_TimeoutSet(a2)
         beq.s    hww_WaitRak1
         bra.s    hww_ExitError
hww_RakOk1:         
         ; --- init handshake 
         ; [OUT]
         SETCIAOUTPUT a5
         
         ; Set <CMD_SEND>
         move.b   #HWF_CMD_SEND,ciaa+ciaprb-BaseAX(a5)
         
         ; Set SEL = 1 -> Trigger Plipbox
         SETSELECT a5
         
         ; --- (size +) data loop         
         ; packet size (in bytes)
         move.w   (a3),d6
         btst     #0,d6
         beq.s    hww_even
         addq.w   #1,d6
hww_even:
         ; convert to words
         ; (includes size field for dbra)
         lsr.w    #1,d6

         ; -- even byte 0,2,4,...
         ; Wait RAK == 1
hww_WaitRak2a:
         move.b   (a5),d0                             ; ciab+ciapra
         btst     d4,d0                               ; RAK toggled?
         bne.s    hww_RakOk2a
         ; check for timeout
         tst.b    hwb_TimeoutSet(a2)
         beq.s    hww_WaitRak2a
         bra.s    hww_ExitError
hww_RakOk2a:
         ; Set <Size|Data_n>
         move.b   (a3)+,ciaa+ciaprb-BaseAX(a5)        ; write data to port
         ; Toggle REQ
         bset     d3,(a5)                             ; set REQ=1

         ; -- odd byte 1,3,5,...
         ; Wait RAK == 0
hww_WaitRak2b:
         move.b   (a5),d0                             ; ciab+ciapra
         btst     d4,d0                               ; RAK toggled?
         beq.s    hww_RakOk2b
         ; check for timeout
         tst.b    hwb_TimeoutSet(a2)
         beq.s    hww_WaitRak2b
         bra.s    hww_ExitError
hww_RakOk2b:
         ; Set <Size|Data_n>
         move.b   (a3)+,ciaa+ciaprb-BaseAX(a5)        ; write data to port
         ; Toggle REQ
         bclr     d3,(a5)                             ; set REQ=0

         ; loop for all packet bytes
         dbra     d6,hww_WaitRak2a

         ; --- shutdown
         ; final Wait RAK == 1
hww_WaitRak3:
         move.b   (a5),d0                             ; ciab+ciapra
         btst     d4,d0                               ; RAK toggled?
         bne.s    hww_RakOk3
         ; check for timeout
         tst.b    hwb_TimeoutSet(a2)
         beq.s    hww_WaitRak3
         bra.s    hww_ExitError
hww_RakOk3:
        
         ; --- send is OK ---
         moveq    #TRUE,d2                            ; rc = TRUE
hww_ExitError:
         ; --- exit ---
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
;     hwrecv() - low level receive routine
;
; SYNOPSIS
;     void hwrecv(struct HWBase *, struct HWFrame *)
;                 A0               A1
;
; FUNCTION
;     Receive a packet with the plipbox protocol
;
_hwrecv:
         movem.l  d2-d7/a2-a6,-(sp)
         move.l   a0,a2                               ; a2 = HWBase
         move.l   a1,a3                               ; a3 = HWFrame
         moveq    #FALSE,d5                           ; d5 = return value
         move.l   hwb_SysBase(a2),a6                  ; a6 = CIABase
         moveq    #HS_REQ_BIT,d3                      ; d3 = HS_REQ
         moveq    #HS_RAK_BIT,d4                      ; d4 = HS_RAK
         lea      BaseAX,a5                           ; a5 = ciab+ciapra

         ; --- prepare
         ; Wait RAK == 0
hwr_WaitRak1:
         move.b   (a5),d0                             ; ciab+ciapra
         btst     d4,d0
         beq.s    hwr_RakOk1
         ; check for timeout
         tst.b    hwb_TimeoutSet(a2)
         beq.s    hwr_WaitRak1
         bra      hwr_ExitError
hwr_RakOk1:
   
         ; --- init handshake 
         ; [OUT]
         SETCIAOUTPUT a5
         
         ; Set <CMD_RECV>
         move.b   #HWF_CMD_RECV,ciaa+ciaprb-BaseAX(a5)
         
         ; Set SEL = 1 -> trigger Plipbox
         SETSELECT a5
        
         ; Wait RAK == 1
hwr_WaitRak2:
         move.b   (a5),d0                             ; ciab+ciapra
         btst     d4,d0
         bne.s    hwr_RakOk2
         ; check for timeout
         tst.b    hwb_TimeoutSet(a2)
         beq.s    hwr_WaitRak2
         bra.s    hwr_ExitError
hwr_RakOk2:

         ; [IN]
         SETCIAINPUT a5
 
         ; Set REQ = 1
         bset     d3,(a5)
         
         ; --- read size word ---
         ; Wait RAK == 0
hwr_WaitRak3:
         move.b   (a5),d0                             ; ciab+ciapra
         btst     d4,d0                               ; RAK toggled?
         beq.s    hwr_RakOk3
         ; check for timeout
         tst.b    hwb_TimeoutSet(a2)
         beq.s    hwr_WaitRak3
         bra.s    hwr_ExitError
hwr_RakOk3:
         
         ; Read <Size_Hi>
         move.b   ciaa+ciaprb-BaseAX(a5),(a3)+        ; read par port
         ; Set REQ = 0
         bclr     d3,(a5)                             ; REQ toggle
         
         ; Wait RAK == 1
hwr_WaitRak4:
         move.b   (a5),d0                             ; ciab+ciapra
         btst     d4,d0
         bne.s    hwr_RakOk4
         ; check for timeout
         tst.b    hwb_TimeoutSet(a2)
         beq.s    hwr_WaitRak4
         bra.s    hwr_ExitError
hwr_RakOk4:
         ; Read <Size_Lo>
         move.b   ciaa+ciaprb-BaseAX(a5),(a3)+        ; READCIABYTE
         ; Set REQ = 1
         bset     d3,(a5)                             ; REQ toggle

         ; --- check size
         ; now fetch full size word and check for max frame size
         move.w   -2(a3),d6                           ; = length
         tst.w    d6
         beq.s    hwr_ExitOk                          ; empty size? ok
         cmp.w    hwb_MaxFrameSize(a2),d6             ; buffer too large
         bhi.s    hwr_ExitError

         ; convert to words
         ; (-1 for dbra)
         btst     #0,d6
         bne.s    hwr_odd
         subq.w   #1,d6
hwr_odd:
         lsr.w    #1,d6
         
         ; --- main packet data loop ---
         ; wait for incoming RAK on data byte

         ; -- even bytes: 0,2,4,...
         ; Wait RAK == 0
hwr_WaitRak5a:
         move.b   (a5),d0                             ; ciab+ciapra
         btst     d4,d0                               ; RAK toggled?
         beq.s    hwr_RakOk5a
         ; check for timeout
         tst.b    hwb_TimeoutSet(a2)
         beq.s    hwr_WaitRak5a
         bra.s    hwr_ExitError
hwr_RakOk5a: 
         ; Read <DATA_n>
         move.b   ciaa+ciaprb-BaseAX(a5),(a3)+        ; read par port byte
         bclr     d3,(a5)                             ; toggle REQ

         ; -- odd bytes: 1,3,5,...
         ; Wait RAK == 1
hwr_WaitRak5b:
         move.b   (a5),d0                             ; ciab+ciapra
         btst     d4,d0                               ; RAK toggled?
         bne.s    hwr_RakOk5b
         ; check for timeout
         tst.b    hwb_TimeoutSet(a2)
         beq.s    hwr_WaitRak5b
         bra.s    hwr_ExitError
hwr_RakOk5b: 
         ; Read <DATA_n>
         move.b   ciaa+ciaprb-BaseAX(a5),(a3)+        ; read par port byte
         bset    d3,(a5)                              ; toggle REQ

         ; loop for all packet bytes
         dbra     d6,hwr_WaitRak5a

hwr_ExitOk:
         moveq    #TRUE,d5
hwr_ExitError:
         ; --- exit ---
         
         ; reset signal
         moveq    #0,d0
         move.l   hwb_IntSigMask(a1),d1
         JSRLIB   SetSignal
         
         ; clear RECV_PENDING flag set by irq
         bclr     #HWB_RECV_PENDING,hwb_Flags(a2)
         
         ; clear REQ
         bclr     d3,(a5)
         
         ; clear SEL
         CLRSELECT a5

         move.l   d5,d0                               ; return value
         movem.l  (sp)+,d2-d7/a2-a6
         rts

;----------------------------------------------------------------------------
;
; NAME
;     hwburstsend() - low level send routine in burst mode
;
; SYNOPSIS
;     void hwburstsend(struct HWBase *, struct HWFrame *, WORD burstSize)
;                      A0               A1                D0
;
; FUNCTION
;     This functions sends a HW frame with the plipbox protocol via
;     the parallel port and uses the fast burst protocol
;     burstSize in words
_hwburstsend:
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

         ; d1 = burst size -1
         move.w   d0,d1
         subq.w   #1,d1

         ; --- size calc for burst
         ; packet size (in bytes) rounded to words (d6)
         move.w   (a3),d6
         addq.w   #1,d6
         lsr.w    #1,d6                               ; d6 = packet size in words

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

         ; --- send burst size 
         move.w   d5,d7
         lsr.w    #8,d7 ; prepare burst hi
         ; Wait RAK == 1
bww_WaitRak2a:
         move.b   (a5),d0                             ; ciab+ciapra
         btst     d4,d0                               ; RAK toggled?
         bne.s    bww_RakOk2a
         ; check for timeout
         tst.b    hwb_TimeoutSet(a2)
         beq.s    bww_WaitRak2a
         bra      bww_ExitError
bww_RakOk2a:
         ; Set <burst_words> hi byte
         move.b   d7,(a4)                             ; write data to port
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
         bra      bww_ExitError
bww_RakOk2b:
         ; Set <burst_words> lo byte
         move.b   d5,(a4)                             ; write data to port
         ; Toggle REQ
         bclr     d3,(a5)                             ; set REQ=0

         ; --- send size (without burst)
         ; Wait RAK == 1
bww_WaitRak2c:
         move.b   (a5),d0                             ; ciab+ciapra
         btst     d4,d0                               ; RAK toggled?
         bne.s    bww_RakOk2c
         ; check for timeout
         tst.b    hwb_TimeoutSet(a2)
         beq.s    bww_WaitRak2c
         bra.s    bww_ExitError
bww_RakOk2c:
         ; Set <size> hi byte
         move.b   (a3)+,(a4)                          ; write data to port
         ; Toggle REQ
         bset     d3,(a5)                             ; set REQ=1

         ; Wait RAK == 0
bww_WaitRak2d:
         move.b   (a5),d0                             ; ciab+ciapra
         btst     d4,d0                               ; RAK toggled?
         beq.s    bww_RakOk2d
         ; check for timeout
         tst.b    hwb_TimeoutSet(a2)
         beq.s    bww_WaitRak2d
         bra.s    bww_ExitError
bww_RakOk2d:
         ; Set <size> lo byte
         move.b   (a3)+,(a4)                          ; write data to port
         ; Toggle REQ
         bclr     d3,(a5)                             ; set REQ=0

         ; disable all irq
         JSRLIB   Disable

         ; ---- burst chunk loop
bww_BurstChunk:
         ; setup size of even burst chunk: d7 = words-1 per chunk
         cmp.w    d6,d5    ; compare remaining size with burst size
         blt.s    bww_bce_ok
         ; last loop - use remaining size for loop
         move.w   d6,d7
         subq.w   #1,d7 ; correct for dbra
         moveq    #0,d6 ; update remaining size
         bra.s    bww_WaitRak3a
bww_bce_ok:
         ; full lopp - use burst size for loop
         move.w   d1,d7
         sub.w    d5,d6 ; update remaining size
         
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
         
         ; --- burst loop
bww_BurstLoop:
         ; set even data 0,2,4,...
         move.b   (a3)+,(a4)                          ; write data to port
         ; Toggle REQ
         bset     d3,(a5)                             ; set REQ=1
         
         ; set odd data 1,3,5,...
         move.b   (a3)+,(a4)                          ; write data to port
         ; Toggle REQ
         bclr     d3,(a5)                             ; set REQ=1

         dbra     d7,bww_BurstLoop

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

         ; done?
         tst.w    d6
         bne.s    bww_BurstChunk

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

         ; enable all irq
         JSRLIB   Enable

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
;     void hwburstrecv(struct HWBase *, struct HWFrame *, WORD burstSize)
;                      A0               A1                D0
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

         ; d1 = burst size -1
         move.w   d0,d1
         subq.w   #1,d1

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

         ; --- send burst size 
         move.w   d5,d7
         lsr.w    #8,d7 ; prepare burst hi
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
         ; Set <burst_words> hi byte
         move.b   d7,(a4)                             ; write data to port
         ; Toggle REQ
         bset     d3,(a5)                             ; set REQ=1

         ; Wait RAK == 0
bwr_WaitRak2b:
         move.b   (a5),d0                             ; ciab+ciapra
         btst     d4,d0                               ; RAK toggled?
         beq.s    bwr_RakOk2b
         ; check for timeout
         tst.b    hwb_TimeoutSet(a2)
         beq.s    bwr_WaitRak2b
         bra      bwr_ExitError
bwr_RakOk2b:
         ; Set <burst_words> lo byte
         move.b   d5,(a4)                             ; write data to port
         ; Toggle REQ
         bclr     d3,(a5)                             ; set REQ=0

         ; --- toggle to input
         ; Wait RAK == 1
bwr_WaitRak2c:
         move.b   (a5),d0                             ; ciab+ciapra
         btst     d4,d0                               ; RAK toggled?
         bne.s    bwr_RakOk2c
         ; check for timeout
         tst.b    hwb_TimeoutSet(a2)
         beq.s    bwr_WaitRak2c
         bra      bwr_ExitError
bwr_RakOk2c:
         ; [IN]
         SETCIAINPUT a5
         ; Toggle REQ
         bset     d3,(a5)                             ; set REQ=1

         ; --- read size word ---
         ; Wait RAK == 0
bwr_WaitRak2d:
         move.b   (a5),d0                             ; ciab+ciapra
         btst     d4,d0                               ; RAK toggled?
         beq.s    bwr_RakOk2d
         ; check for timeout
         tst.b    hwb_TimeoutSet(a2)
         beq.s    bwr_WaitRak2d
         bra      bwr_ExitError
bwr_RakOk2d:
         
         ; Read <Size_Hi>
         move.b   (a4),(a3)+                          ; read par port
         ; Set REQ = 0
         bclr     d3,(a5)                             ; REQ toggle
         
         ; Wait RAK == 1
bwr_WaitRak2e:
         move.b   (a5),d0                             ; ciab+ciapra
         btst     d4,d0
         bne.s    bwr_RakOk2e
         ; check for timeout
         tst.b    hwb_TimeoutSet(a2)
         beq.s    bwr_WaitRak2e
         bra.s    bwr_ExitError
bwr_RakOk2e:
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

         ; convert packet size (d6) to words (and round up if necessary)
         addq.w   #1,d6
         lsr.w    #1,d6

         ; disable all irq
         JSRLIB   Disable
         
         ; ---- burst chunk loop
bwr_BurstChunk:
         ; setup size of burst chunk: d7 = words-1 per chunk
         cmp.w    d6,d5    ; compare with remaining size
         blt.s    bwr_bce_ok
         ; last loop
         move.w   d6,d7    ; use remaining size for final loop
         subq.w   #1,d7 
         moveq    #0,d6    ; size done
         bra.s    bwr_WaitRak3a
bwr_bce_ok:
         move.w   d1,d7    ; use burst size - 1
         sub.w    d5,d6    ; update size

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
         
         ; --- burst loop
bwr_BurstLoop:
         ; Toggle REQ
         bclr     d3,(a5)                             ; set REQ=0
         ; get even data 0,2,4,...
         move.b   (a4),(a3)+                          ; read data from port
         
         ; Toggle REQ
         bset     d3,(a5)                             ; set REQ=1
         ; get odd data 1,3,5,...
         move.b   (a4),(a3)+                          ; read data from port

         dbra     d7,bwr_BurstLoop

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

         ; done?
         tst.w    d6
         bne.s    bwr_BurstChunk

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

         ; enable all irq
         JSRLIB   Enable

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

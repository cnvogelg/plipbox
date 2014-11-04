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
         move.l   a1,a4                               ; a4 = Frame
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
         move.w   (a4),d6
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
         move.l   a1,a4                               ; a4 = HWFrame
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
;     burstBytes = 2 * (burstSize+1)
_hwburstsend:
         movem.l  d2-d7/a2-a6,-(sp)
         move.l   a0,a2                               ; a2 = HWBase
         move.l   a1,a4                               ; a4 = Frame
         move.w   d0,d5                               ; d5 = burstSize in words - 1
         moveq    #FALSE,d2                           ; d2 = return value
         moveq    #HS_REQ_BIT,d3                      ; d3 = HS_REQ
         moveq    #HS_RAK_BIT,d4                      ; d4 = HS_RAK
         lea      BaseAX,a5                           ; a5 = CIA HW base

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
         move.b   #HWF_CMD_SEND_BURST,ciaa+ciaprb-BaseAX(a5)
         
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
         move.b   (a3)+,ciaa+ciaprb-BaseAX(a5)        ; write data to port
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
         move.b   (a3)+,ciaa+ciaprb-BaseAX(a5)        ; write data to port
         ; Toggle REQ
         bclr     d3,(a5)                             ; set REQ=0

         ; --- size calc for burst
         ; packet size (in bytes)
         ; rounded to words (size = 1 -> 2)
         move.w   (a4),d6
         btst     #0,d6
         bne.s    bww_odd
         subq.w   #1,d6
bww_odd:
         ; convert to words
         lsr.w    #1,d6

         ; ---- burst chunk loop
bww_BurstChunk:
         ; setup size of even burst chunk: d7 = words-1 per chunk
         move.w   d5,d7    ; get burst size
         cmp.w    d6,d7    ; compare with remaining size
         blt.s    bww_bce_ok
         move.w   d6,d7
bww_bce_ok:
         sub.w    d7,d6    ; update total size

         ; Wait RAK == 1
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
         move.b   (a3)+,ciaa+ciaprb-BaseAX(a5)        ; write data to port
         ; Toggle REQ
         bset     d3,(a5)                             ; set REQ=1
         
         ; set odd data 1,3,5,...
         move.b   (a3)+,ciaa+ciaprb-BaseAX(a5)        ; write data to port
         ; Toggle REQ
         bclr     d3,(a5)                             ; set REQ=1

         dbra     d7,bww_BurstLoop

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
         ; [IN]
         SETCIAINPUT a5

         ; SEL = 0
         CLRSELECT a5

         move.l   d2,d0                               ; return rc
         movem.l  (sp)+,d2-d7/a2-a6
         rts

         end

;
;  $VER$;
;  linplip.device - Parallel Line Internet Protocol
;
;  Assembler routines for efficient port handling.
;
;  Written by Stéphane Zermatten, based on the code written
;  by Oliver Wagner and Michael Balzer, and modified by 
;  Marius Gröger. 
;  
;  (C) Copyright 1997 Stéphane Zermatten
;      All Rights Reserved
;
;  HISTORY:
;
;  1.3 (97.10.26 23:18:33)      -- Protocol changed (once again)
;                                  to lighten the charge of the
;                                  68k CPU (I suppose the PC more
;                                  powerful)
;  1.2 (97.10.25 16:34:24)      -- Receive protocol changed. 
;                                  Now, 3 nibbles are used, instead of 4. 
;  1.1 (97.10.25 08:20:23)      -- initial revision
;                                  based on a C version of linport
;
; 
;  FUTUR CHANGES
;
;   I'm working on an interrupt-based version, to see which 
;   approach is better.    
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


      xdef    _interrupt
      xdef    _hwsend
      xdef    _hwrecv

;----------------------------------------------------------------------------

ciaa     equ     $bfe001
ciab     equ     $bfd000
BaseAX   equ     ciab+ciapra

OutReg   equ     ciaa+ciaprb
StatusReg equ    ciab+ciapra

OUTB_BUSY equ 4


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
;----------------------------------------------------------------------------

_interrupt:
	
	btst    #PLIPB_RECEIVING,pb_Flags(a1)  
	bne.s   setack          
					; PLIPF_RECEIVING&pb_Flags == 0

	move.b  StatusReg,d0            ; PAR_STATUS==CIAF_PRTRPOUT ? 
	andi.b  #7,d0
	cmp.b   #2,d0   
	bne.s   skipint

	bset    #PLIPB_RECEIVING,pb_Flags(a1)
	move.l  pb_IntSigMask(a1),d0
	movea.l pb_SysBase(a1),a6       ; Signal
	movea.l pb_Server(a1),a1
gosig:  
;       movem.l a6,-(sp)                ; ExecBase is not in a6...
	JSRLIB  Signal
;       movem.l (sp)+,a6
skipint: 
	moveq #0,d0                     ; set Z flag => next server
	rts

setack:                                 ; PLIPF_RECEIVING&pb_Flags != 0
	bset.b  #PLIPEB_NIBBLEACK,(pb_ExtFlags+1)(a1)
	moveq #0,d0                     ; set Z flag => next server
	rts


;----------------------------------------------------------------------------
;
; NAME
;     send_bit() - writes one byte to the parallel port
;
; SYNOPSIS
;     ULONG send_bit(UBYTE *TimeoutSet, UBYTE)
;                       a2               d0
;
;
; FUNCTION
;     This function writes one byte to the parallel port.
;     Implemented in a really stupid way (using loops),
;     but I can't see how I could use signals or interrupts
;     without changing the protocol.
;
; RESULT -> D0, Z
;     0         All went right
;     1         Timeout
;       
;----------------------------------------------------------------------------
_send_bit:
	move.l  d0,d1                   ; data -> d1

	andi.b  #$0f,d0                 ; first 4 bits 0-3
	move.b  d0,OutReg
	bset    #OUTB_BUSY,d0           ; BUSY on
	move.b  d0,OutReg               ; bset #OUTB_BUSY,OutReg won't work 

loop1:                                  ; wait for BUSY on or Timeout
	tst.b   (a2)
	bne     snd_timeout
	btst    #CIAB_PRTRBUSY,StatusReg
	beq.s   loop1

	lsr.b   #4,d1                   ; last bits 4-7
	bset    #OUTB_BUSY,d1
	move.b  d1,OutReg
	bclr    #OUTB_BUSY,d1           ; BUSY off
	move.b  d1,OutReg               ; bclr #OUTB_BUSY,OutReg won't work

loop2:
	tst.b   (a2)                    ; wait for BUSY off or Timeout
	bne     snd_timeout
	btst    #CIAB_PRTRBUSY,StatusReg
	bne.s   loop2

	
;allright:
	moveq   #0,d0                   ; return OK
	rts

snd_timeout: 
	moveq   #1,d0                   ; return TIMEOUT
	rts

;----------------------------------------------------------------------------
;
; NAME
;     receive_bit() - reads one byte from the parallel port
;
; SYNOPSIS
;     ULONG receive_bit(volatile UBYTE *timeoutset, UBYTE *ExtFlags + 1, UBYTE busystate)
;                               a2                        a0                     d2
;
;
; FUNCTION
;     This function reads one byte from the parallel port, 
;     relying on messages sent by the interrupt. 
;
; ARGUMENTS
;
;     timeoutset        : the byte at this address is checked periodically
;                         and the function returns an error if a non-null
;                         value is found there.
;     ExtFlags+1        : The bit 1 of this byte contains the flag NIBBLEACK
;                         set by the interrupt = ((UBYTE *)pb->pb_ExtFlags)+1
;     busystate         : The state in which the BUSY output line was left
;                         by a previous call of this function of hwrecv.
;                         value : 0x00 or 0x10
;
; RESULT
;  Dans d0 (et dans Z)
;     0         All went right
;     1         Timeout 
;  Dans d1 : 
;     Le bit lu
;
; NOTE
;     a0 and a2 are left untouched ! guaranteed  !
; 
;     I've tried a version using signals sent by the interrupt instead
;     of those stupid loops, but it's too slow. (about 3 times as slow)
;
;----------------------------------------------------------------------------
_receive_bit:

WAITFOR_NIBBLEACK       MACRO   ; should be a function, but I'm looking for speed               
rloop\@:
	tst.b   (a2)
	bne     rcv_timeout
	btst.b  #PLIPEB_NIBBLEACK,(a0)
	beq     rloop\@
	
	bclr.b  #PLIPEB_NIBBLEACK,(a0)
	ENDM

RECV_ACKNOWLEDGE        MACRO   ; invert the state of the output BUSY line (uses d2)
	bchg    #OUTB_BUSY,d2
	move.b  d2,OutReg
	ENDM                            


	WAITFOR_NIBBLEACK

	; d1 = in_sbp()
	clr.b   d1

	move.b  StatusReg,d1            ; statusreg -> d1 
	RECV_ACKNOWLEDGE

	andi.b  #7,d1

	WAITFOR_NIBBLEACK

	; d1 |= in_sbp()<<3
	move.b  StatusReg,d0
	RECV_ACKNOWLEDGE

	andi.b  #7,d0
	lsl.b   #3,d0
	eor.b   d0,d1

	WAITFOR_NIBBLEACK       

	; d1 |= in_sbp() << 6
	move.b  StatusReg,d0
	RECV_ACKNOWLEDGE

	andi.b  #3,d0
	lsl.b   #6,d0
	eor.b   d0,d1

;allright:
	moveq   #0,d0                   ; return OK
	rts

rcv_timeout: 
	moveq  #1,d0                    ; return TIMEOUT
	rts


;----------------------------------------------------------------------------
;
; NAME
;    hwrecv(struct PLIPBase *) - receive a frame from the parallel port
;           a0
;
; FUNCTION
;    First allocate a signal from the current task and set pb_ModSigMask and
;    pb_ModTask for the interrpution. 
;    Then, it acknowledge the beginning of a packet by sending 0x02 (SELECT),
;    gets the size, the data and the checksum (one byte). 
;    
; RESULT
;    boolean (ULONG) 
;    TRUE => OK
;    FALSE => ERROR
;
;----------------------------------------------------------------------------
_hwrecv:
	movem.l d2-d7/a2-a5,-(sp)

	movea.l a0,a5                   ; pb -> a5
	lea.l   pb_TimeoutSet(a5),a2    ; &pb->pb_TimeoutSet -> a2 (pour receive_bit)
	lea.l   (pb_ExtFlags+1)(a5),a0  ; &pb->pb_ExtFlags+1 -> a0 (pour receive_bit)
	movea.l pb_Frame(a5),a3         ; frame 
	lea.l   pf_DstAddr(a3),a4       ; &frame->data -> a4
	lea.l   pf_Size(a3),a3          ; &frame->pf_Size -> a3
	moveq   #0,d2                   ; state of the BUSY output (pour receive_bit)

	moveq   #0,d4                   ; retval = d4 (ERROR=0)

	bclr.b  #PLIPEB_NIBBLEACK,(a0)

	moveq   #2,d0
	move.b  d0,OutReg               ; Acknowledge

	clr.w   d5                      ; d5 = data_len

	jsr     _receive_bit            ; Lower part of Length
	bne.s   ffrec_error
	move.b  d1,d5                   ; -> d5 (low)


	jsr     _receive_bit            ; Upper part of Length
	bne.s   ffrec_error
	lsl.w   #8,d1
	eor.w   d1,d5
	beq.s   ffrec_error             ; size == 0 ?


	move.w  d5,(a3)                 ; d5 -> pf_Size 

	move.l  pb_MTU(a5),d0
	add.w   #PKTFRAMESIZE_2,d0
	cmp.w   d5,d0
	blt.s   ffrec_error             ; d0<d5=data_len Too big ? 
	tst.w   d5
	blt.s   ffrec_error             ; d5 < 0 ??


	subq.w  #1,d5                   ; d5 = size-1 (better for loops)
	move.w  d5,d6                   ; d6 = size-1 (for checksum loop)
dataloop:                               ; do {
	jsr     _receive_bit
	bne.s   ffrec_error
	move.b  d1,(a4)+
	dbf.w   d5,dataloop             ; }while(--d5!=-1);


	clr.b   d5                      ; Checksum -> d5
checkloop:                              ; do {
	add.b   -(a4),d5
	dbf.w   d6,checkloop            ; }while(--d6!=-1);

	jsr     _receive_bit
	bne.s   ffrec_error


	cmp.b   d5,d1                   ; Checksum OK ?
	bne.s   ffrec_error

					; Clean up the port
	moveq   #1,d4                   ; return OK

	tst.b   d2                      ; d2==0 ? 
	beq.s   ffrec_error             ; Not an error. Skipping

lineloop:                               ; Clean the line. 
	tst.b   (a2)
	bne     ffrec_error             ; The retval stays to TRUE
	btst.b  #PLIPEB_NIBBLEACK,(a0)
	beq     lineloop

ffrec_error:
	clr.b   OutReg
	bclr    #PLIPB_RECEIVING,pb_Flags(a5)
	move.l  d4,d0                   ; return retval
frrec_end:
	movem.l (sp)+,d2-d7/a2-a5
	rts

;----------------------------------------------------------------------------
;
; NAME
;    hwsend(struct PLIPBase *) - receive a frame to the parallel port
;           a0
;
; FUNCTION
;    First, it triggers the other side by sending an ACK signal, and
;    then it waits for an answer (a SEL signal).
;    It sends : two bytes containing the size, the data, and a crc (1 byte)
;    
; RESULT
;    boolean (ULONG) 
;    TRUE => OK
;    FALSE => ERROR
;
;----------------------------------------------------------------------------
_hwsend:
	movem.l d4-d6/a2-a6,-(sp)

	movea.l a0,a5                   ; pb -> a5
	lea.l   pb_TimeoutSet(a5),a2    ; &pb->pb_TimeoutSet -> a2 (for send_bit)
	movea.l pb_Frame(a5),a3         ; frame 
	lea.l   pf_DstAddr(a3),a4       ; data -> a4
	lea.l   pf_Size(a3),a3          ; *size -> a3
	lea.l   pb_Flags(a5),a0         ; *pb_Flags -> a0 (temporary !)

	moveq   #0,d4                   ; d4=retval=FALSE (error)

	btst    #PLIPB_RECEIVING,(a0)
	bne     snd_error               ; Collision ?

	moveq   #8,d0
	move.b  d0,OutReg               ; Trigger

sndloop0:
	tst.b   (a2)                    ; TimeoutSet ?
	bne     snd_error
	btst    #PLIPB_RECEIVING,(a0)
	bne     snd_error
	btst    #CIAB_PRTRSEL,StatusReg
	beq.s   sndloop0                ; }while(!(StatusReg&CIAF_PRTRSEL));

	movea.l pb_CIAABase(a5),a6      ; CIAABase -> a6
	moveq   #CIAICRF_FLG,d0
	JSRLIB  AbleICR                 ; Disable interrupt

	move.w  (a3),d5                 ; data_len -> d5
	move.b  d5,d0                   ; Send lower byte of Size
	jsr     _send_bit
	bne     snd_inton

	move.w  d5,d0
	lsr.w   #8,d0                   ; Send upper byte of Size
	jsr     _send_bit
	bne     snd_inton

	subq.w  #1,d5                   ; d5=size-1 (for loops)
	move.w  d5,d6                   ; d6=size-1

sndmainloop:                            ; do{
	move.b  (a4)+,d0
	jsr     _send_bit
	bne     snd_inton
	dbf.w   d5,sndmainloop          ; }while(--d5!=-1)

	clr.b   d0                      ; checksum -> d0
sndchecksum:
	add.b   -(a4),d0
	dbf.w   d6,sndchecksum          ; }while(--d6!=-1)

	jsr     _send_bit       
	bne     snd_inton

	moveq   #1,d4                   ; retval = TRUE (OK)

snd_inton:
	moveq   #CIAICRF_FLG,d0
	JSRLIB  SetICR                  ; Clear interrupt
	move.w  #CIAICRF_FLG|CIAICRF_SETCLR,d0
	JSRLIB  AbleICR                 ; Enable interrupt

snd_error:
	clr.b   OutReg                  ; put the line back into order
	move.b  d4,d0

snd_end:
	movem.l (sp)+,d4-d6/a2-a6
	rts

	 end

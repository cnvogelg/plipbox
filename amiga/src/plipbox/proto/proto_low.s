; --- proto signals ---
; POUT = /CLK  (Amiga out)
; BUSY = /RAK  (Amiga in)
; SEL  = /PEND (Amiga in)

        include         "pario.i"
        include         "proto.i"

        xdef            _proto_low_action
        xdef            _proto_low_action_no_busy
        xdef            _proto_low_action_bench
        xdef            _proto_low_write_word
        xdef            _proto_low_read_word
        xdef            _proto_low_write_long
        xdef            _proto_low_read_long
        xdef            _proto_low_write_block
        xdef            _proto_low_read_block

; ----- macros --------------------------------------------------------------

; --- setup_port_regs ---
; in:  a0 = port_ptr
; out: d2 = rak bit
;      d3 = clk bit
;      d4 = old ctrl val
;      d7 = busy bit
;      a3 = data port
;      a4 = data ddr
;      a5 = ctrl port
setup_port_regs  MACRO
        moveq   #0,d2
        moveq   #0,d3
        moveq   #0,d7
        move.b  PO_POUT_BIT(a0),d2
        move.b  PO_SEL_BIT(a0),d3
        move.b  PO_BUSY_BIT(a0),d7
        move.l  PO_DATA_PORT(a0),a3
        move.l  PO_DATA_DDR(a0),a4
        move.l  PO_CTRL_PORT(a0),a5
        ; read current ctrl val
        move.b  (a5),d4
        ENDM


; --- check_rak_hi ---
; check if RAK is high otherwise rts with error
; \1 end label
check_rak_hi  MACRO
        btst    d2,(a5)
        bne.s   \@
        moveq   #RET_RAK_INVALID,d0
        bra     \1
\@:
        ENDM


; --- check_rak_lo ---
; check if RAK is high otherwise rts with error
; \1 end label
check_rak_lo  MACRO
        btst    d2,(a5)
        beq.s   \@
        moveq   #RET_RAK_INVALID,d0
        bra     \1
\@:
        ENDM


; --- check_busy ---
; check if busy is high and if yes abort with error
; \1 end label
check_busy  MACRO
        btst    d7,(a5)
        beq.s   \@
        moveq   #RET_DEVICE_BUSY,d0
        bra     \1
\@:
        ENDM


; --- clk_lo ---
; set CLK signal to low
clk_lo  MACRO
        bclr    d3,d4
        move.b  d4,(a5)
        ENDM


; --- clk_hi ---
; set CLK signal to high
clk_hi  MACRO
        bset    d3,d4
        move.b  d4,(a5)
        ENDM


; --- clk_set ---
; set clock signal
clk_set MACRO
        move.b  \1,(a5)
        ENDM


; --- wait_rak_lo ---
; wait for RAK to become low or if timeout triggers
; \1 = jump label on timeout
wait_rak_lo  MACRO
        ; check RAK level
\@1:    btst    d2,(a5)
        beq.s   \@2
        ; check for timeout
        tst.b   (a1)
        beq.s   \@1
        ; error
        moveq   #RET_TIMEOUT,d0
        bra     \1
\@2:
        ENDM


; --- wait_rak_lo_busy ---
; wait for RAK to become low or if timeout triggers or busy is set
; \1 = jump label on timeout/busy
wait_rak_lo_busy  MACRO
        ; check RAK level
\@1:    btst    d2,(a5)
        beq.s   \@2
        ; check for busy bit
        btst    d7,(a5)
        bne.s   \@3
        ; check for timeout
        tst.b   (a1)
        beq.s   \@1
        ; timeout error
        moveq   #RET_TIMEOUT,d0
        bra     \1
\@3:    ; device busy
        moveq   #RET_DEVICE_BUSY,d0
        bra     \1
\@2:
        ENDM


; --- wait_rak_hi ---
; wait for RAK to become high or if timeout triggers
; \1 = jump label on timeout
wait_rak_hi  MACRO
        ; check RAK level
\@1:    btst    d2,(a5)
        bne.s   \@2
        ; check for timeout
        tst.b   (a1)
        beq.s   \@1
        ; error
        moveq   #RET_TIMEOUT,d0
        bra     \1
\@2:
        ENDM


; --- ddr_in ---
; set data direction to input
ddr_in  MACRO
        sf.b    (a4)
        ENDM


; --- ddr_out ---
; set data direction to output
ddr_out  MACRO
        st.b    (a4)
        ENDM


; --- set_cmd_idle ---
; set command to idle (0)
set_cmd_idle  MACRO
        sf.b    (a3)
        ENDM


; --- set_cmd ---
; set a command byte to data port
; \1 = cmd constant
set_cmd  MACRO
        move.b  \1,(a3)
        ENDM


; --- set_data ---
; set data port
; \1 = value to set
set_data MACRO
        move.b  \1,(a3)
        ENDM


; --- get_data ---
; get data from port
; \1 = store value
get_data MACRO
        move.b  (a3),\1
        ENDM


; --- call_callback ---
; \1 = id
call_cb MACRO
        moveq   \1,d0
        movem.l a0-a1,-(sp)
        move.l  (a2),a0
        jsr     (a0)
        movem.l (sp)+,a0-a1
        ENDM


; --- delay ---
; waste a few CIA cycles
delay   MACRO
        tst.b           (a5)
        tst.b           (a5)
        tst.b           (a5)
        tst.b           (a5)
        tst.b           (a5)
        ENDM


; ----- functions -----------------------------------------------------------

; --- proto_low_action ---
; a simple command that does not transfer any value
;
;   in:
;       a0      struct pario_port *port
;       a1      volatile UBYTE *timeout_flag
;       d0      action constant
;   out:
;       d0      return code
_proto_low_action:
        movem.l d2-d7/a2-a6,-(sp)

        ; setup regs with port values and read old ctrl value
        setup_port_regs

        ; -- sync with slave
        ; check RAK to be high or abort
        check_rak_hi    pla_end
        check_busy      pla_end
        ; set cmd to data port
        set_cmd         d0
        ; set CLK to low (active) to trigger command at slave
        clk_lo
        ; busy wait with timeout for RAK to go low
        ; (we wait for the slave to react/sync)
        wait_rak_lo_busy  pla_abort

        ; now we are in sync with slave
        ; we are now in work phase
        ; but ping does nothing here

        ; -- final sync
        ; now raise CLK again
        clk_hi
        ; expect slave to raise rak, too
        wait_rak_hi     pla_end

        ; ok
        moveq   #RET_OK,d0
pla_end:
        ; restore cmd
        set_cmd_idle

        movem.l (sp)+,d2-d7/a2-a6
        rts
pla_abort:
        ; ensure CLK is hi
        clk_hi
        bra.s   pla_end


; --- proto_low_action_no_busy ---
; a simple command that does not transfer any value
;
;   in:
;       a0      struct pario_port *port
;       a1      volatile UBYTE *timeout_flag
;       d0      action constant
;   out:
;       d0      return code
_proto_low_action_no_busy:
        movem.l d2-d7/a2-a6,-(sp)
        setup_port_regs
        check_rak_hi    planb_end
        set_cmd         d0
        clk_lo
        ; ignore busy here!
        wait_rak_lo     planb_abort

        clk_hi
        wait_rak_hi     planb_end

        moveq   #RET_OK,d0
planb_end:
        set_cmd_idle
        movem.l (sp)+,d2-d7/a2-a6
        rts
planb_abort:
        clk_hi
        bra.s   planb_end


; --- proto_low_action_bench ---
; the proto_low_action routine instrumented with benchmark callbacks
;
;   in:
;       a0      struct pario_port *port
;       a1      volatile UBYTE *timeout_flag
;       a2      callback struct
;       d0      CMD_PING constant
;   out:
;       d0      return code
_proto_low_action_bench:
        movem.l d2-d7/a2-a6,-(sp)

        ; setup regs with port values and read old ctrl value
        setup_port_regs

        ; -- sync with slave
        ; check RAK to be high or abort
        check_rak_hi    plab_end
        check_busy      plab_end
        ; set cmd to data port
        set_cmd         d0
        ; set CLK to low (active) to trigger command at slave
        clk_lo

        ; callback 0: set clock low
        call_cb         #0

        ; busy wait with timeout for RAK to go low
        ; (we wait for the slave to react/sync)
        wait_rak_lo_busy  plab_abort

        ; callback 1: got rak lo
        call_cb         #1

        ; -- final sync
        ; now raise CLK again
        clk_hi
        ; expect slave to raise rak, too
        wait_rak_hi     plab_end

        ; callback 2: got rak hi
        call_cb         #2

        ; ok
        moveq   #RET_OK,d0
plab_end:
        ; restore cmd
        set_cmd_idle

        movem.l (sp)+,d2-d7/a2-a6
        rts
plab_abort:
        ; ensure CLK is hi
        clk_hi
        bra.s   plab_end


; --- proto_low_write_word ---
; in:  a0 = port ptr
;      a1 = timeout byte ptr
;      a2 = ptr to data
;      d0 = cmd byte
; out: d0 = result
_proto_low_write_word:
        movem.l d2-d7/a2-a6,-(sp)
        setup_port_regs

        ; sync with slave
        check_rak_hi    plrw_end
        check_busy      plrw_end
        set_cmd         d0
        clk_lo
        wait_rak_lo_busy  plrw_abort

        ; -- first byte
        ; setup test value on data lines
        set_data        (a2)+
        ; signal to slave to read the value
        clk_hi

        ; -- second byte
        set_data        (a2)+
        clk_lo

        ; final sync
        clk_hi
        ; wait for slave done
        wait_rak_hi     plrw_end

        ; ok
        moveq   #RET_OK,d0
plrw_end:
        set_cmd_idle
        movem.l (sp)+,d2-d7/a2-a6
        rts
plrw_abort:
        ; ensure CLK is hi
        clk_hi
        bra.s    plrw_end


; --- proto_low_write_long ---
; in:  a0 = port ptr
;      a1 = timeout byte ptr
;      a2 = ptr to data
;      d0 = cmd byte
; out: d0 = result
_proto_low_write_long:
        movem.l d2-d7/a2-a6,-(sp)
        setup_port_regs

        ; sync with slave
        check_rak_hi    plrwl_end
        check_busy      plrwl_end
        set_cmd         d0
        clk_lo
        wait_rak_lo_busy  plrwl_abort

        ; -- first byte
        ; setup test value on data lines
        set_data        (a2)+
        ; signal to slave to read the value
        clk_hi

        ; -- second byte
        set_data        (a2)+
        clk_lo

        ; -- byte 3
        set_data        (a2)+
        clk_hi

        ; -- byte 4
        set_data        (a2)+
        clk_lo

        ; final sync
        clk_hi
        ; wait for slave done
        wait_rak_hi     plrwl_end

        ; ok
        moveq   #RET_OK,d0
plrwl_end:
        set_cmd_idle
        movem.l (sp)+,d2-d7/a2-a6
        rts
plrwl_abort:
        ; ensure CLK is hi
        clk_hi
        bra.s    plrwl_end


; --- proto_low_read_word ---
; in:  a0 = port ptr
;      a1 = timeout byte ptr
;      a2 = ptr to test byte
;      d0 = cmd byte
; out: d0 = result
_proto_low_read_word:
        movem.l d2-d7/a2-a6,-(sp)
        setup_port_regs

        ; sync with slave
        check_rak_hi    plrr_end
        check_busy      plrr_end
        set_cmd         d0
        clk_lo
        wait_rak_lo_busy  plrr_abort

        ; ddr: in
        ddr_in
        clk_hi

        ; first byte
        ; signal read to slave
        clk_lo
        ; read value from data port
        get_data        (a2)+

        ; second byte
        clk_hi
        get_data        (a2)+

        ; ddr: out
        clk_lo
        ddr_out

        ; final sync
        clk_hi
        wait_rak_hi     plrr_end

        ; ok
        moveq   #RET_OK,d0
plrr_end:
        set_cmd_idle
        movem.l (sp)+,d2-d7/a2-a6
        rts
plrr_abort:
        ; ensure CLK is hi
        clk_hi
        bra.s    plrr_end


; --- proto_low_read_long ---
; in:  a0 = port ptr
;      a1 = timeout byte ptr
;      a2 = ptr to test byte
;      d0 = cmd byte
; out: d0 = result
_proto_low_read_long:
        movem.l d2-d7/a2-a6,-(sp)
        setup_port_regs

        ; sync with slave
        check_rak_hi    plrrl_end
        check_busy      plrrl_end
        set_cmd         d0
        clk_lo
        wait_rak_lo_busy  plrrl_abort

        ; ddr: in
        ddr_in
        clk_hi

        ; first byte
        ; signal read to slave
        clk_lo
        ; read value from data port
        get_data        (a2)+

        ; second byte
        clk_hi
        get_data        (a2)+

        ; byte 3
        clk_lo
        get_data        (a2)+

        ; byte 4
        clk_hi
        get_data        (a2)+

        ; ddr: out
        clk_lo
        ddr_out

        ; final sync
        clk_hi
        wait_rak_hi     plrrl_end

        ; ok
        moveq   #RET_OK,d0
plrrl_end:
        set_cmd_idle
        movem.l (sp)+,d2-d7/a2-a6
        rts
plrrl_abort:
        ; ensure CLK is hi
        clk_hi
        bra.s    plrrl_end


; --- proto_low_write_block ---
; in:  a0 = port ptr
;      a1 = timeout byte ptr
;      a2 = ptr to data
;      d0 = cmd
;      d1 = num_words
; out: d0 = result
_proto_low_write_block:
        movem.l d2-d7/a2-a6,-(sp)
        setup_port_regs

        ; sync with slave
        check_rak_hi    plmw_end
        check_busy      plmw_end
        set_cmd         d0
        clk_lo
        wait_rak_lo_busy  plmw_abort

        ; prepare clock signals
        move.w          d4,d5
        bclr            d3,d4 ; d4 = clk_lo
        bset            d3,d5 ; d5 = clk_hi

        ; enter chunk copy loop
        subq.w          #1,d1 ; for dbra

        ; data block loop
plmw_loop:
        ; odd byte
        set_data        (a2)+
        clk_set         d5
        ; even byte
        set_data        (a2)+
        clk_set         d4

        dbra            d1,plmw_loop

        ; ok
        moveq   #RET_OK,d0

        ; final sync
        clk_hi
        wait_rak_hi    plmw_end
plmw_end:
        set_cmd_idle
        movem.l (sp)+,d2-d7/a2-a6
        rts
plmw_abort:
        ; ensure CLK is hi
        clk_hi
        bra.s    plmw_end


; --- proto_low_read_block ---
; in:  a0 = port ptr
;      a1 = timeout byte ptr
;      a2 = ptr to data
;      d0 = cmd byte
;      d1 = num_words
; out: d0 = result
_proto_low_read_block:
        movem.l d2-d7/a2-a6,-(sp)
        setup_port_regs

        ; sync with slave
        check_rak_hi    plmr_end
        check_busy      plmr_end
        set_cmd         d0
        clk_lo
        wait_rak_lo_busy  plmr_abort

        ; switch: port read
        ddr_in
        clk_hi

        ; prepare clock signals
        move.w          d4,d6
        bclr            d3,d4 ; d4 = clk_lo
        bset            d3,d6 ; d6 = clk_hi

        ; enter chunk copy loop
        subq.w          #1,d1 ; for dbra

plmr_loop:
        ; odd byte
        clk_set         d4
        get_data        (a2)+
        ; even byte
        clk_set         d6
        get_data        (a2)+
        dbra            d1,plmr_loop

        ; ok
        moveq   #RET_OK,d0

        ; switch: port write
        clk_lo
        ddr_out

        ; final sync
        clk_hi
        wait_rak_hi    plmr_end
plmr_end:
        set_cmd_idle
        movem.l (sp)+,d2-d7/a2-a6
        rts
plmr_abort:
        ; ensure CLK is hi
        clk_hi
        bra.s           plmr_end


#include <avr/io.h>
#include "pario_pins.h"
#include "pario_pins.S"

    ; some convenience macros for the control signals
    ; CLK = POUT
    ; RAK = BUSY

    .macro sbic_clk
    sbic    _SFR_IO_ADDR(PAR_SELECT_PIN), PAR_SELECT_BIT
    .endm

    .macro sbis_clk
    sbis    _SFR_IO_ADDR(PAR_SELECT_PIN), PAR_SELECT_BIT
    .endm

    .macro wait_clk_lo
l\@: sbic    _SFR_IO_ADDR(PAR_SELECT_PIN), PAR_SELECT_BIT
    rjmp     l\@
    .endm

    .macro wait_clk_hi
l\@: sbis    _SFR_IO_ADDR(PAR_SELECT_PIN), PAR_SELECT_BIT
    rjmp     l\@
    .endm

    .macro ack_hi
    sbi     _SFR_IO_ADDR(PAR_ACK_PORT), PAR_ACK_BIT
    .endm

    .macro ack_lo
    cbi     _SFR_IO_ADDR(PAR_ACK_PORT), PAR_ACK_BIT
    .endm

    .macro rak_hi
    sbi     _SFR_IO_ADDR(PAR_POUT_PORT), PAR_POUT_BIT
    .endm

    .macro rak_lo
    cbi     _SFR_IO_ADDR(PAR_POUT_PORT), PAR_POUT_BIT
    .endm

    .macro busy_hi
    sbi     _SFR_IO_ADDR(PAR_BUSY_PORT), PAR_BUSY_BIT
    .endm

    .macro busy_lo
    cbi     _SFR_IO_ADDR(PAR_BUSY_PORT), PAR_BUSY_BIT
    .endm

#ifdef NO_IRQ
    .macro irq_off
    .endm
    .macro irq_on
    .endm
#else
    .macro irq_off
    cli
    .endm

    .macro irq_on
    sei
    .endm
#endif

    ; ----- proto_low_init -----
    ; void proto_low_init(void)
    ; setup parallel port lines
    .section .text.proto_low_init
    .global  proto_low_init
proto_low_init:
    ; setup data port: idle mode
    ddr_in
    ; set default value
    ldi     r18,0xff
    data_out_prepare
    data_out_set
    ; set OUTPUT levels
    ack_hi
    rak_hi
    busy_lo
    ; setup control lines
    ; DDR INPUT: strobe, select
    cbi     _SFR_IO_ADDR(PAR_STROBE_DDR), PAR_STROBE_BIT
    cbi     _SFR_IO_ADDR(PAR_SELECT_DDR), PAR_SELECT_BIT
    ; DDR OUTPUT: ack, busy, select
    sbi     _SFR_IO_ADDR(PAR_ACK_DDR), PAR_ACK_BIT
    sbi     _SFR_IO_ADDR(PAR_BUSY_DDR), PAR_BUSY_BIT
    sbi     _SFR_IO_ADDR(PAR_POUT_DDR), PAR_POUT_BIT
    ret


    ; ----- ack/pend signals -----
    ; void proto_low_ack_lo(void)
    .section .text.proto_low_ack_lo
    .global  proto_low_ack_lo
proto_low_ack_lo:
    ack_lo
    ret


    ; void proto_low_ack_hi(void)
    .section .text.proto_low_ack_hi
    .global  proto_low_ack_hi
proto_low_ack_hi:
    ack_hi
    ret


    ; void proto_low_busy_hi(void)
    .section .text.proto_low_busy_hi
    .global  proto_low_busy_hi
proto_low_busy_hi:
    busy_hi
    ret


    ; void proto_low_busy_lo(void)
    .section .text.proto_low_busy_lo
    .global  proto_low_busy_lo
proto_low_busy_lo:
    busy_lo
    ret


    ; ----- proto_low_get_cmd -----
    ; u08 proto_low_get_cmd(void)
    ; return 0xff for idle or command byte
    .section .text.proto_low_get_cmd
    .global  proto_low_get_cmd
proto_low_get_cmd:
    clr     r25
    ldi     r24, 0xff
    sbic_clk
    ret
    ; read command from data port
    data_in
    mov     r24, r18
    ret


    ; ----- proto_low_action -----
    ; void proto_low_action(void)
    ; confirm command with no value
    ; CL | WL | CH
    .section .text.proto_low_action
    .global  proto_low_action
proto_low_action:
    ; when we enter here the clk from master is already low
    ; now signal rak lo to the master. we are now in sync
    rak_lo
    ret


    ; ----- proto_low_end -----
    ; void proto_low_end(void)
    ; end an action or function call
    ; in: -
    ; out: -
    .section .text.proto_low_end
    .global  proto_low_end
proto_low_end:
    ; command finishes with a last clk hi transition
    wait_clk_hi
    ; confirm it
    rak_hi
    ret


    ; ----- proto_low_write_word -----
    ; master wants to write a u16 function value
    ; u16 proto_low_write_word(void)
    ; in:  -
    ; out: r24,r25 = reg_num
    .section .text.proto_low_write_word
    .global  proto_low_write_word
proto_low_write_word:
    irq_off
    ; confirm to master we are ready
    rak_lo

    ; byte 1
    ; wait clk
    wait_clk_hi
    ; get data and save as result
    data_in
    mov     r25, r18

    ; byte 2
    ; wait clk
    wait_clk_lo
    data_in
    mov     r24, r18

    irq_on
    ret


    ; ----- proto_low_write_long -----
    ; master wants to write a u32 function value
    ; u32 proto_low_write_long(void)
    ; in:  -
    ; out: r22,r23,r24,r25 = value
    .section .text.proto_low_write_long
    .global  proto_low_write_long
proto_low_write_long:
    irq_off
    ; confirm to master we are ready
    rak_lo

    ; byte 1
    ; wait clk
    wait_clk_hi
    ; get data and save as result
    data_in
    mov     r25, r18

    ; byte 2
    ; wait clk
    wait_clk_lo
    data_in
    mov     r24, r18

    ; byte 3
    ; wait clk
    wait_clk_hi
    data_in
    mov     r23, r18

    ; byte 4
    ; wait clk
    wait_clk_lo
    data_in
    mov     r22, r18

    irq_on
    ret


    ; ----- proto_low_read_word -----
    ; master wants to read a u16 function value
    ; void proto_low_read_word(u16 v)
    ; in:  r24,r25 = read val
    ; out: -
    .section .text.proto_low_read_word
    .global  proto_low_read_word
proto_low_read_word:
    irq_off
    rak_lo

    ; ddr: output
    wait_clk_hi
    ddr_out

    ; byte 1
    mov     r18, r25
    data_out_prepare
    wait_clk_lo
    data_out_set

    ; byte 2
    mov     r18, r24
    data_out_prepare
    wait_clk_hi
    data_out_set

    ; ddr: idle
    wait_clk_lo
    ddr_in

    irq_on
    ret


    ; ----- proto_low_read_long -----
    ; master wants to read a u32 function value
    ; void proto_low_read_long(u32 v)
    ; in:  r22,r23,r24,r25 = read val
    ; out: -
    .section .text.proto_low_read_long
    .global  proto_low_read_long
proto_low_read_long:
    irq_off
    rak_lo

    ; ddr: output
    wait_clk_hi
    ddr_out

    ; byte 1
    mov     r18, r25
    data_out_prepare
    wait_clk_lo
    data_out_set

    ; byte 2
    mov     r18, r24
    data_out_prepare
    wait_clk_hi
    data_out_set

    ; byte 3
    mov     r18, r23
    data_out_prepare
    wait_clk_lo
    data_out_set

    ; byte 4
    mov     r18, r22
    data_out_prepare
    wait_clk_hi
    data_out_set

    ; ddr: idle
    wait_clk_lo
    ddr_in

    irq_on
    ret


    ; ----- proto_low_write_block -----
    ; master wants to write a message
    ; u16 proto_low_write_block(u16 num_words, u08 *buffer)
    ; in:  r25,r24 = num_words
    ; in:  r23,r22 = buffer ptr
    ; out: -
    .section .text.proto_low_write_block
    .global  proto_low_write_block
proto_low_write_block:
    ; Z is buffer pointer
    movw    r30,r22

    ; double size: convert to bytes
    add     r24,r24
    adc     r25,r25

    ; add start address: end address: r23:r22
    add     r24,r30
    adc     r25,r31

    irq_off

    ; confirm to master we are ready
    rak_lo

    ; copy loop
.wb_loop:
    ; check end
    cp      r30,r24
    cpc     r31,r25
    breq    .wb_end

    ; odd byte
    wait_clk_hi
    data_in
    st      Z+,r18
    ; even byte
    wait_clk_lo
    data_in
    st      Z+,r18

    rjmp    .wb_loop

.wb_end:
    ; done
    irq_on
    ret


    ; ----- proto_low_read_block -----
    ; master wants to read a message
    ; u08 proto_low_read_block(u16 num_words, const u08 *buffer)
    ; in:  r25,r24 = max_words we can receive
    ; in:  r23,r22 = buffer ptr
    ; out: -
    .section .text.proto_low_read_block
    .global  proto_low_read_block
proto_low_read_block:
    ; Z is buffer pointer
    movw    r30,r22

    ; convert size to bytes
    add     r24,r24
    adc     r25,r25

    ; set end address: r24:r25
    add     r24,r30
    adc     r25,r31

    irq_off

    ; confirm to master we are ready
    rak_lo

    ; ddr out
    wait_clk_hi
    ddr_out

    ; copy loop
.rb_loop:
    ; check end
    cp      r30,r24
    cpc     r31,r25
    breq    .rb_end

    ; odd byte
    ld      r18,Z+
    data_out_prepare
    wait_clk_lo
    data_out_set

    ; even byte
    ld      r18,Z+
    data_out_prepare
    wait_clk_hi
    data_out_set

    rjmp    .rb_loop

.rb_end:
    ; ddr idle
    wait_clk_lo
    ddr_in

    irq_on
    ret

#ifndef BOOTLOADER
    ; ========== direct SPI message I/O ==========

    ; ----- proto_low_write_block -----
    ; master wants to write a message
    ; u16 proto_low_write_block(u16 num_words, u08 *buffer)
    ; in:  r25,r24 = num_words
    ; out: -
    .section .text.proto_low_write_block_spi
    .global  proto_low_write_block_spi
proto_low_write_block_spi:
    ; clear Z counter
    clr     r30
    clr     r31

    irq_off

    ; confirm to master we are ready
    rak_lo

    ; copy loop
.wb_loop_spi:
    ; check end
    cp      r30,r24
    cpc     r31,r25
    breq    .wb_end_spi

    ; odd byte in r18
    wait_clk_hi
    data_in
    out     _SFR_IO_ADDR(SPDR), r18

    ; even byte in r18
    wait_clk_lo
    data_in
    out     _SFR_IO_ADDR(SPDR), r18

    adiw    r30,1
    rjmp    .wb_loop_spi

    ; read spi status to clear flag for next operation
.wb_end_spi:
    in r18, _SFR_IO_ADDR(SPSR)
    sbrs r18, SPIF
    rjmp .wb_end_spi

    ; done
    irq_on
    ret

    ; ----- proto_low_read_block_spi -----
    ; master wants to read a message
    ; u08 proto_low_read_block(u16 num_words, const u08 *buffer)
    ; in:  r25,r24 = num words
    ; out: -
    .section .text.proto_low_read_block_spi
    .global  proto_low_read_block_spi
proto_low_read_block_spi:
    ; clear Z counter
    clr     r30
    clr     r31

    irq_off

    ; confirm to master we are ready
    rak_lo

    ; setup write value for SPI and push first byte
    ldi     r21,0xff
    out     _SFR_IO_ADDR(SPDR), r21   
 
    ; ddr out
    wait_clk_hi
    ddr_out

    ; copy loop
.rb_loop_spi:
    ; odd byte
    in      r18, _SFR_IO_ADDR(SPDR)
    data_out_prepare
    out     _SFR_IO_ADDR(SPDR), r21   
    wait_clk_lo
    data_out_set

    adiw    r30,1

    ; check end
    cp      r30,r24
    cpc     r31,r25
    breq    .rb_end_spi

    ; even byte
    in      r18, _SFR_IO_ADDR(SPDR)
    data_out_prepare
    out     _SFR_IO_ADDR(SPDR), r21   
    wait_clk_hi
    data_out_set

    rjmp    .rb_loop_spi

.rb_end_spi:
    ; last even byte without pushing an SPI byte
    in      r18, _SFR_IO_ADDR(SPDR)
    data_out_prepare
    wait_clk_hi
    data_out_set

    ; ddr idle
    wait_clk_lo
    ddr_in

   ; read spi status to clear flag for next operation
.rb_done_spi:
    in r18, _SFR_IO_ADDR(SPSR)
    sbrs r18, SPIF
    rjmp .rb_done_spi

    irq_on
    ret
#endif

    .end

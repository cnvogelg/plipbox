    ; ----- 8 bit data port -----

    ; avrnetio uses full portc for data

    ; ddr_out
    ; set data direction to output
    ; in: -
    ; out: -
    ; regs: r18
    .macro ddr_out
    ldi     r18, 0xff                       ; 1
    out     _SFR_IO_ADDR(DDRC), r18         ; 1
    .endm                                   ; = 2

    ; ddr_in
    ; set data direction to input
    ; in: -
    ; out: -
    ; regs: r18
    .macro ddr_in
    clr     r18                             ; 1
    out     _SFR_IO_ADDR(DDRC), r18         ; 1
    .endm                                   ; = 2

    ; data_out
    ; set data values on parallel port
    ; in: r18=value
    ; out: -
    ; regs: -
    .macro data_out_prepare
    .endm
    .macro data_out_set
    out     _SFR_IO_ADDR(PORTC), r18        ; 1
    .endm                                   ; = 1

    ; data_in
    ; read data values from parallel port
    ; in: -
    ; out: r18
    ; regs: -
    .macro data_in
    in      r18, _SFR_IO_ADDR(PINC)         ; 1
    .endm                                   ; = 1

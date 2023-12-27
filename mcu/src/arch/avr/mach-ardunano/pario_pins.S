    ; ----- 8 bit data port -----

    ; Note: on Nano data port is split across two GPIO ports
    ; data port C: don't care about above bits (PC6, PC7)
    ; as they are not used (reset on PC6 is disabled)
    ; this saves us some instruction for OR/ANDin values

    ; ddr_out
    ; set data direction to output
    ; in: -
    ; out: -
    ; regs: r18, r19
    .macro ddr_out
    ldi     r18, 0x3f                       ; 1
    in      r19, _SFR_IO_ADDR(DDRD)         ; 1
    ori     r19, 0xc0                       ; 1
    out     _SFR_IO_ADDR(DDRC), r18         ; 1
    out     _SFR_IO_ADDR(DDRD), r19         ; 1
    .endm                                   ; = 5

    ; ddr_in
    ; set data direction to input
    ; in: -
    ; out: -
    ; regs: r18, r19
    .macro ddr_in
    clr     r18                             ; 1
    in      r19, _SFR_IO_ADDR(DDRD)         ; 1
    andi    r19, 0x3f                       ; 1
    out     _SFR_IO_ADDR(DDRC), r18         ; 1
    out     _SFR_IO_ADDR(DDRD), r19         ; 1
    .endm                                   ; = 5

    ; data_out
    ; set data values on parallel port
    ; in: r18=value
    ; out: -
    ; regs: r19, r20
    .macro data_out_prepare
    mov     r19, r18                        ; 1
    in      r20, _SFR_IO_ADDR(PIND)         ; 1
    andi    r20, 0x3f                       ; 1
    andi    r19, 0xc0                       ; 1
    or      r19, r20                        ; 1
    .endm
    .macro data_out_set
    out     _SFR_IO_ADDR(PORTC), r18        ; 1
    out     _SFR_IO_ADDR(PORTD), r19        ; 1
    .endm                                   ; = 7 (!)

    ; data_in
    ; read data values from parallel port
    ; in: -
    ; out: r18
    ; regs: r19
    .macro data_in
    in      r18, _SFR_IO_ADDR(PINC)         ; 1
    in      r19, _SFR_IO_ADDR(PIND)         ; 1
    andi    r18, 0x3f                       ; 1
    andi    r19, 0xc0                       ; 1
    or      r18, r19                        ; 1
    .endm                                   ; = 5



        include "exec/macros.i"

        xdef _pario_irq_handler

        ; a1 = pario_handle
_pario_irq_handler:
        ; update irq counter
        addq.w  #1,14(a1)

        tst.w   12(a1)              ; was signal already sent?
        bne.s   exit
        st.w    12(a1)              ; set pending flag

        ; signal counter
        addq.w  #1,16(a1)

        ; send signal to task
        move.l  (a1),a6             ; sysbase
        move.l  8(a1),d0            ; sigMask
        move.l  4(a1),a1            ; sigTask
        ;JSRLIB  Signal
        jsr     -324(a6)
exit:
        moveq #0,d0
        rts

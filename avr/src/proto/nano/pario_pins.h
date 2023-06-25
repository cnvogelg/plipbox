/*
    Parallel Port Connection
                     Arduino        Nano
    DATA 0 ... 5     PC 0 ... 5     same      IN/OUT
    DATA 6 ... 7     PD 6 ... 7     same      IN/OUT

    /STROBE          PD 2           PD3       IN (INT0 / INT1)
    SELECT           PD 3           PB1       OUT
    BUSY             PD 4           same      OUT
    POUT             PD 5           same      IN
    /ACK             PB 0           same      OUT
*/

// /STROBE (IN) (INT1) (D3)
#define PAR_STROBE_BIT          3
#define PAR_STROBE_MASK         _BV(PAR_STROBE_BIT)
#define PAR_STROBE_PORT         PORTD
#define PAR_STROBE_PIN          PIND
#define PAR_STROBE_DDR          DDRD

#define PAR_STROBE_INT          INT1
#define PAR_STROBE_VECT         INT1_vect
#define PAR_STROBE_ISC          ISC11
#define PAR_STROBE_EICR         EICRA

// SELECT (OUT) (B1)
#define PAR_SELECT_BIT          1
#define PAR_SELECT_MASK         _BV(PAR_SELECT_BIT)
#define PAR_SELECT_PORT         PORTB
#define PAR_SELECT_PIN          PINB
#define PAR_SELECT_DDR          DDRB

// BUSY (OUT) (D4)
#define PAR_BUSY_BIT            4
#define PAR_BUSY_MASK           _BV(PAR_BUSY_BIT)
#define PAR_BUSY_PORT           PORTD
#define PAR_BUSY_PIN            PIND
#define PAR_BUSY_DDR            DDRD

// POUT (IN) (D5)
#define PAR_POUT_BIT            5
#define PAR_POUT_MASK           _BV(PAR_POUT_BIT)
#define PAR_POUT_PORT           PORTD
#define PAR_POUT_PIN            PIND
#define PAR_POUT_DDR            DDRD

// /ACK (OUT) (D8)
#define PAR_ACK_BIT             0
#define PAR_ACK_MASK            _BV(PAR_ACK_BIT)
#define PAR_ACK_PORT            PORTB
#define PAR_ACK_PIN             PINB
#define PAR_ACK_DDR             DDRB

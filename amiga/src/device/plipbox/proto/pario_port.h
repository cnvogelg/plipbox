#ifndef PARIO_PORT_H
#define PARIO_PORT_H

struct pario_port {
    volatile UBYTE *data_port; /* +0 */
    volatile UBYTE *data_ddr;  /* +4 */
    volatile UBYTE *ctrl_port; /* +8 */
    volatile UBYTE *ctrl_ddr;  /* +12 */
    UBYTE busy_bit;            /* +16 */
    UBYTE pout_bit;            /* +17 */
    UBYTE sel_bit;             /* +18 */
    UBYTE dummy1;
    UBYTE busy_mask;           /* +20 */
    UBYTE pout_mask;           /* +21 */
    UBYTE sel_mask;            /* +22 */
    UBYTE all_mask;            /* +23 */
};

#endif
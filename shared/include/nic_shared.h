#ifndef NIC_SHARED_H
#define NIC_SHARED_H

/* capabilities bit field (requested in NCAP) */
#define NIC_CAP_DIRECT_IO       1
#define NIC_CAP_LOOP_BUF        2
#define NIC_CAP_LOOP_MAC        4

#define NIC_CAP_BROADCAST       0x10
#define NIC_CAP_FULL_DUPLEX     0x20
#define NIC_CAP_FLOW_CONTROL    0x40

/* result values */
#define NIC_OK                        0
#define NIC_ERROR_INVALID_PORT        1
#define NIC_ERROR_DEVICE_NOT_FOUND    2
#define NIC_ERROR_IOCTL_NOT_FOUND     3
#define NIC_ERROR_RX                  4
#define NIC_ERROR_TX                  5

#endif

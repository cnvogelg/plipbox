#ifndef HW_SYSTEM_H
#define HW_SYSTEM_H

extern void hw_system_init(void);
extern void hw_system_reset(void);

#ifdef HAVE_OTP_MAC
extern void hw_system_otp_mac(mac_t mac);
#endif

#endif

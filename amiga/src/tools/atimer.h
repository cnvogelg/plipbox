#ifndef ATIMER_H
#define ATIMER_H

struct atimer_handle;
typedef struct atimer_handle atimer_handle_t;

struct atime_stamp { unsigned long hi, lo; };
typedef struct atime_stamp atime_stamp_t;

extern atimer_handle_t *atimer_init(struct Library *execbase);
extern void atimer_exit(atimer_handle_t *th);

extern void atimer_get_sys_time(atimer_handle_t *th, atime_stamp_t *val);

extern ULONG atimer_eclock_get(atimer_handle_t *th, atime_stamp_t *val);
extern void atimer_eclock_delta(atime_stamp_t *end, atime_stamp_t *begin, atime_stamp_t *delta);

extern ULONG atimer_eclock_to_us(atimer_handle_t *th, ULONG delta);
extern ULONG atimer_eclock_to_kBps(atimer_handle_t *th, ULONG delta, ULONG bytes);

extern BOOL atimer_sig_init(struct atimer_handle *th);
extern ULONG atimer_sig_get_mask(struct atimer_handle *th);
extern void atimer_sig_exit(struct atimer_handle *th);
extern void atimer_sig_start(struct atimer_handle *th, ULONG secs, ULONG micros);
extern void atimer_sig_stop(struct atimer_handle *th);

#endif

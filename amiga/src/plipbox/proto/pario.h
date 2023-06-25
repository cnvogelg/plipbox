#ifndef PARIO_H
#define PARIO_H

struct pario_handle;
struct pario_port;

typedef struct pario_handle pario_handle_t;

extern pario_handle_t *pario_init(struct Library *execbase);
extern void pario_exit(pario_handle_t *ph);

extern int pario_setup_ack_irq(pario_handle_t *ph, struct Task *sigTask, BYTE signal);
extern void pario_cleanup_ack_irq(pario_handle_t *ph);

extern struct pario_port *pario_get_port(pario_handle_t *ph);

extern UWORD pario_get_ack_irq_counter(pario_handle_t *ph);
extern UWORD pario_get_signal_counter(pario_handle_t *ph);
extern void pario_confirm_ack_irq(pario_handle_t *ph);

#endif

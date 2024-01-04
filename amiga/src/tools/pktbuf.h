#ifndef PKTBUF_H
#define PKTBUF_H

struct pktbuf {
  UBYTE *data;
  ULONG size;
};
typedef struct pktbuf pktbuf_t;

pktbuf_t *pktbuf_alloc(ULONG size);
void pktbuf_free(pktbuf_t *buf);

void pktbuf_fill(pktbuf_t *buf, UBYTE start);
BOOL pktbuf_check(pktbuf_t *buf, UBYTE start, ULONG *pos);

#endif

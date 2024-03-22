#include <exec/exec.h>
#include <proto/exec.h>
#include <proto/dos.h>

#include "pktbuf.h"

pktbuf_t *pktbuf_alloc(ULONG size)
{
  pktbuf_t *buf = (pktbuf_t *)AllocMem(sizeof(pktbuf_t), MEMF_ANY | MEMF_CLEAR);
  if(buf == NULL) {
    return NULL;
  }

  buf->size = size;
  buf->data = AllocMem(size, MEMF_ANY | MEMF_CLEAR);
  if(buf->data == NULL) {
    FreeMem(buf, sizeof(pktbuf_t));
    return NULL;
  }

  return buf;
}

void pktbuf_free(pktbuf_t *buf)
{
  if(buf != NULL) {
    if(buf->data != NULL) {
      FreeMem(buf->data, buf->size);
    }
    FreeMem(buf, sizeof(pktbuf_t));
  }
}

void pktbuf_fill(pktbuf_t *buf, UBYTE start)
{
  UBYTE *ptr = buf->data;
  for(ULONG i=0;i<buf->size;i++) {
    *(ptr++) = start++;
  }
}

BOOL pktbuf_check(pktbuf_t *buf, UBYTE start, ULONG *pos)
{
  UBYTE *ptr = buf->data;
  for(ULONG i=0;i<buf->size;i++) {
    UBYTE val = start++;
    if(*ptr != val) {
      if(pos != NULL) {
        *pos = ptr - buf->data;
      }
      return FALSE;
    }
    ptr++;
  }
  return TRUE;
}

#define __NOLIBBASE__
#include <proto/exec.h>

#include "compiler.h"
#include "debug.h"

#include "proto_cmd.h"
#include "proto_cmd_shared.h"

int proto_cmd_attach(proto_handle_t *proto)
{
  int res;

  d8(("proto_cmd_attach:\n"));
  res = proto_atom_action(proto, PROTO_CMD_ATTACH);
  d8(("-> res=%ld\n", (LONG)res));
  return res;
}

int proto_cmd_detach(proto_handle_t *proto)
{
  int res;

  d8(("proto_cmd_detach:\n"));
  res = proto_atom_action(proto, PROTO_CMD_DETACH);
  d8(("-> res=%ld\n", (LONG)res));
  return res;
}

int proto_cmd_get_status(proto_handle_t *proto, UWORD *status)
{
  int res;

  d8(("proto_cmd_get_status:\n"));
  res = proto_atom_read_word(proto, PROTO_CMD_GET_STATUS, status);
  d8(("-> status=%lx res=%ld\n", (ULONG)*status, (LONG)res));
  return res;
}

int proto_cmd_send_frame(proto_handle_t *proto, UBYTE *buf, UWORD num_bytes)
{
  int res;
  UWORD status = 0;

  d8(("proto_cmd_send_frame: size=%lu\n", (ULONG)num_bytes));

  res = proto_atom_write_word(proto, PROTO_CMD_TX_SIZE, num_bytes);
  d8(("-> size: res=%ld\n", (LONG)res));
  if(res != PROTO_RET_OK) {
    return res;
  }

  res = proto_atom_write_block(proto, PROTO_CMD_TX_BUF, buf, num_bytes);
  d8(("-> block: res=%ld\n", (LONG)res));
  if(res != PROTO_RET_OK) {
    return res;
  }

  res = proto_atom_read_word(proto, PROTO_CMD_TX_RESULT, &status);
  d8(("-> status=%lx res=%ld\n", (ULONG)status, (LONG)res));
  if(res != PROTO_RET_OK) {
    return res;
  }

  if(status != 0) {
    return PROTO_RET_TX_ERROR;
  } else {
    return PROTO_RET_OK;
  }
}

int proto_cmd_recv_frame(proto_handle_t *proto, UBYTE *buf, UWORD max_bytes, UWORD *num_bytes)
{
  int res;
  UWORD status = 0;

  d8(("proto_cmd_recv_frame: %lu\n", (ULONG)max_bytes));

  res = proto_atom_read_word(proto, PROTO_CMD_RX_SIZE, num_bytes);
  d8(("-> size=%ld res=%ld\n", (LONG)*num_bytes, (LONG)res));
  if(res != PROTO_RET_OK) {
    return res;
  }

  /* check max size */
  if(*num_bytes > max_bytes) {
    return PROTO_RET_RX_TOO_LARGE;
  }

  /* anything to read? */
  if(*num_bytes > 0) {
    res = proto_atom_read_block(proto, PROTO_CMD_RX_BUF, buf, *num_bytes);
    d8(("-> block %ld\n", (LONG)res));
    if(res != PROTO_RET_OK) {
      return res;
    }
  }

  res = proto_atom_read_word(proto, PROTO_CMD_RX_RESULT, &status);
  d8(("-> status=%lx %ld\n", (ULONG)status, (LONG)res));
  if(res != PROTO_RET_OK) {
    return res;
  }

  if(status != 0) {
    return PROTO_RET_RX_ERROR;
  } else {
    return PROTO_RET_OK;
  }
}

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

int proto_cmd_set_mode(proto_handle_t *proto, UWORD mode)
{
  int res;

  d8(("proto_cmd_set_mode:%ld\n", (ULONG)mode));
  res = proto_atom_write_word(proto, PROTO_CMD_SET_MODE, mode);
  d8(("-> res=%ld\n", (LONG)res));
  return res;
}

int proto_cmd_get_mode(proto_handle_t *proto, UWORD *mode)
{
  int res;

  d8(("proto_cmd_get_mode:\n"));
  res = proto_atom_read_word(proto, PROTO_CMD_GET_MODE, mode);
  d8(("-> mode=%04lx res=%ld\n", (ULONG)*mode, (LONG)res));
  return res;
}

int proto_cmd_set_mac(proto_handle_t *proto, mac_t mac)
{
  int res;

  d8(("proto_cmd_set_mac:%02lx:%02lx:%02lx:%02lx:%02lx:%02lx\n",
    (ULONG)mac[0], (ULONG)mac[1], (ULONG)mac[2],
    (ULONG)mac[3], (ULONG)mac[4], (ULONG)mac[5]));
  res = proto_atom_write_block(proto, PROTO_CMD_SET_MAC, mac, MAC_SIZE);
  d8(("-> res=%ld\n", (LONG)res));
  return res;
}

int proto_cmd_get_mac(proto_handle_t *proto, mac_t mac)
{
  int res;

  d8(("proto_cmd_get_mac:"));
  res = proto_atom_read_block(proto, PROTO_CMD_GET_MAC, mac, MAC_SIZE);
  d8(("-> res=%ld %02lx:%02lx:%02lx:%02lx:%02lx:%02lx\n", (LONG)res,
    (ULONG)mac[0], (ULONG)mac[1], (ULONG)mac[2],
    (ULONG)mac[3], (ULONG)mac[4], (ULONG)mac[5]));
  return res;
}

int proto_cmd_get_def_mac(proto_handle_t *proto, mac_t mac)
{
  int res;

  d8(("proto_cmd_get_def_mac:\n"));
  res = proto_atom_read_block(proto, PROTO_CMD_GET_DEF_MAC, mac, MAC_SIZE);
  d8(("-> res=%ld %02lx:%02lx:%02lx:%02lx:%02lx:%02lx\n", (LONG)res,
    (ULONG)mac[0], (ULONG)mac[1], (ULONG)mac[2],
    (ULONG)mac[3], (ULONG)mac[4], (ULONG)mac[5]));
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

  /* align size to be even */
  if(num_bytes & 1) {
    num_bytes++;
  }

  res = proto_atom_write_block(proto, PROTO_CMD_TX_BUF, buf, num_bytes);
  d8(("-> block: size=%lu res=%ld\n", (ULONG)num_bytes, (LONG)res));
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
  UWORD rx_bytes = *num_bytes;
  if(rx_bytes > max_bytes) {
    return PROTO_RET_RX_TOO_LARGE;
  }

  /* anything to read? */
  if(rx_bytes > 0) {

    /* align size to be even */
    if(rx_bytes & 1) {
      rx_bytes++;
    }

    res = proto_atom_read_block(proto, PROTO_CMD_RX_BUF, buf, rx_bytes);
    d8(("-> block: size=%lu res=%ld\n", (ULONG)rx_bytes, (LONG)res));
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

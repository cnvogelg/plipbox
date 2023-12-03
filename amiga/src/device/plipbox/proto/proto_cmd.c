#define __NOLIBBASE__
#include <proto/exec.h>

#include "compiler.h"
#include "debug.h"

#include "proto_cmd.h"
#include "proto_cmd_shared.h"
#include "param_shared.h"

int proto_cmd_attach(proto_handle_t *proto)
{
  int res;

  d8(("proto_cmd_attach:"));
  res = proto_atom_action(proto, PROTO_CMD_ATTACH);
  d8r((" res=%ld\n", (LONG)res));
  return res;
}

int proto_cmd_detach(proto_handle_t *proto)
{
  int res;

  d8(("proto_cmd_detach:"));
  res = proto_atom_action(proto, PROTO_CMD_DETACH);
  d8r((" res=%ld\n", (LONG)res));
  return res;
}

int proto_cmd_get_status(proto_handle_t *proto, UWORD *status)
{
  int res;

  d8(("proto_cmd_get_status:"));
  res = proto_atom_read_word(proto, PROTO_CMD_GET_STATUS, status);
  d8r((" status=%lx res=%ld\n", (ULONG)*status, (LONG)res));
  return res;
}

int proto_cmd_get_version(proto_handle_t *proto, UWORD *version)
{
  int res;

  d8(("proto_cmd_get_version:"));
  res = proto_atom_read_word(proto, PROTO_CMD_GET_VERSION, version);
  d8r((" version=%lx res=%ld\n", (ULONG)*version, (LONG)res));
  return res;
}

int proto_cmd_get_cur_mac(proto_handle_t *proto, mac_t mac)
{
  int res;

  d8(("proto_cmd_get_cur_mac:"));
  res = proto_atom_read_block(proto, PROTO_CMD_GET_CUR_MAC, mac, MAC_SIZE);
  d8r((" res=%ld %02lx:%02lx:%02lx:%02lx:%02lx:%02lx\n", (LONG)res,
      (ULONG)mac[0], (ULONG)mac[1], (ULONG)mac[2],
      (ULONG)mac[3], (ULONG)mac[4], (ULONG)mac[5]));
  return res;
}

int proto_cmd_get_def_mac(proto_handle_t *proto, mac_t mac)
{
  int res;

  d8(("proto_cmd_get_def_mac:"));
  res = proto_atom_read_block(proto, PROTO_CMD_GET_DEF_MAC, mac, MAC_SIZE);
  d8r((" res=%ld %02lx:%02lx:%02lx:%02lx:%02lx:%02lx\n", (LONG)res,
      (ULONG)mac[0], (ULONG)mac[1], (ULONG)mac[2],
      (ULONG)mac[3], (ULONG)mac[4], (ULONG)mac[5]));
  return res;
}

// ----- param -----

int proto_cmd_param_get_num(proto_handle_t *proto, UWORD *num)
{
  int res;

  d8(("proto_cmd_param_get_num:"));
  res = proto_atom_read_word(proto, PROTO_CMD_PARAM_GET_NUM, num);
  d8r((" num=%ld res=%ld\n", (LONG)*num, (LONG)res));
  return res;
}

int proto_cmd_param_find_tag(proto_handle_t *proto, ULONG tag, UWORD *id)
{
  int res;

  d8(("proto_cmd_param_find_tag: tag=%lx", tag));
  res = proto_atom_write_long(proto, PROTO_CMD_PARAM_FIND_TAG, tag);
  d8r((" res=%ld, ", (LONG)res));

  if(res != PROTO_RET_OK) {
    return res;
  }

  d8r(("get_id:"));
  res = proto_atom_read_word(proto, PROTO_CMD_PARAM_GET_ID, id);
  d8r((" id=%ld res=%ld\n", (ULONG)*id, (LONG)res));
  return res;
}

int proto_cmd_param_get_def(proto_handle_t *proto, UWORD id, proto_param_def_t *def)
{
  int res;
  UBYTE buf[PARAM_DEF_SIZE];

  d8(("proto_cmd_param_get_def: id=%ld", (LONG)id));
  res = proto_atom_write_word(proto, PROTO_CMD_PARAM_SET_ID, id);
  d8r((" res=%ld\n", (LONG)res));
  if(res != PROTO_RET_OK) {
    return res;
  }

  d8r(("get_def[%ld]:", (LONG)sizeof(*def)));
  res = proto_atom_read_block(proto, PROTO_CMD_PARAM_GET_DEF, buf, PARAM_DEF_SIZE);
  d8r((" res=%ld\n", (LONG)res));
  if(res != PROTO_RET_OK) {
    return res;
  }

  // convert wire def
  // param description
  // +00 u08 index
  // +01 u08 type
  // +02 u08 format
  // +03 u08 reserved
  // +04 u16 size
  // +06 u32 tag
  // =10
  def->index = buf[0];
  def->type = buf[1];
  def->format = buf[2];
  def->size = *(UWORD *)&buf[4]; // is big endian
  def->tag = *(ULONG *)&buf[6];
  d8(("got def: index=%ld type=%ld format=%ld size=%ld tag=%lx",
      (ULONG)def->index, (ULONG)def->type, (ULONG)def->format,
      (ULONG)def->size, def->tag));

  return res;
}

int proto_cmd_param_get_val(proto_handle_t *proto, UWORD id, UWORD size, UBYTE *data)
{
  int res;

  d8(("proto_cmd_param_get_def: id=%ld", (LONG)id));
  res = proto_atom_write_word(proto, PROTO_CMD_PARAM_SET_ID, id);
  d8r((" res=%ld\n", (LONG)res));

  if(res != PROTO_RET_OK) {
    return res;
  }

  d8r(("get_val[%ld]:", (LONG)size));
  res = proto_atom_read_block(proto, PROTO_CMD_PARAM_GET_VAL, data, size);
  d8r((" res=%ld\n", (LONG)res));
  return res;
}

int proto_cmd_param_set_val(proto_handle_t *proto, UWORD id, UWORD size, UBYTE *data)
{
  int res;

  d8(("proto_cmd_param_get_def: id=%ld", (LONG)id));
  res = proto_atom_write_word(proto, PROTO_CMD_PARAM_SET_ID, id);
  d8r((" res=%ld\n", (LONG)res));

  if(res != PROTO_RET_OK) {
    return res;
  }

  d8r(("set_val:[%ld]", (LONG)size));
  res = proto_atom_write_block(proto, PROTO_CMD_PARAM_SET_VAL, data, size);
  d8r((" res=%ld\n", (LONG)res));
  return res;
}

// ----- prefs -----

int proto_cmd_prefs_reset(proto_handle_t *proto)
{
  int res;

  d8(("proto_cmd_prefs_reset:"));
  res = proto_atom_action(proto, PROTO_CMD_PREFS_RESET);
  d8r((" res=%ld\n", (LONG)res));
  return res;
}

int proto_cmd_prefs_load(proto_handle_t *proto, UWORD *status)
{
  int res;

  d8(("proto_cmd_prefs_load:"));
  res = proto_atom_read_word(proto, PROTO_CMD_PREFS_LOAD, status);
  d8r((" status=%04lx res=%ld\n", (ULONG)*status, (LONG)res));
  return res;
}

int proto_cmd_prefs_save(proto_handle_t *proto, UWORD *status)
{
  int res;

  d8(("proto_cmd_prefs_save:"));
  res = proto_atom_read_word(proto, PROTO_CMD_PREFS_SAVE, status);
  d8r((" status=%04lx res=%ld\n", (ULONG)*status, (LONG)res));
  return res;
}

// ----- RX/TX -----

int proto_cmd_send_frame(proto_handle_t *proto, UBYTE *buf, UWORD num_bytes, UWORD *status)
{
  int res;

  d8(("proto_cmd_send_frame: size=%lu", (ULONG)num_bytes));

  res = proto_atom_write_word(proto, PROTO_CMD_TX_SIZE, num_bytes);
  d8r((", size: res=%ld", (LONG)res));
  if(res != PROTO_RET_OK) {
    return res;
  }

  /* align size to be even */
  if(num_bytes & 1) {
    num_bytes++;
  }

  res = proto_atom_write_block(proto, PROTO_CMD_TX_BUF, buf, num_bytes);
  d8r((", block: size=%lu res=%ld", (ULONG)num_bytes, (LONG)res));
  if(res != PROTO_RET_OK) {
    return res;
  }

  res = proto_atom_read_word(proto, PROTO_CMD_TX_RESULT, status);
  d8r((", status=%lx res=%ld\n", (ULONG)*status, (LONG)res));
  if(res != PROTO_RET_OK) {
    return res;
  }

  return PROTO_RET_OK;
}

int proto_cmd_recv_frame(proto_handle_t *proto, UBYTE *buf, UWORD max_bytes, UWORD *num_bytes, UWORD *status)
{
  int res;

  d8(("proto_cmd_recv_frame: max=%lu", (ULONG)max_bytes));

  res = proto_atom_read_word(proto, PROTO_CMD_RX_SIZE, num_bytes);
  d8r((", size=%ld res=%ld", (LONG)*num_bytes, (LONG)res));
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
    d8r((", block: size=%lu res=%ld", (ULONG)rx_bytes, (LONG)res));
    if(res != PROTO_RET_OK) {
      return res;
    }
  }

  res = proto_atom_read_word(proto, PROTO_CMD_RX_RESULT, status);
  d8r((", status=%lx res=%ld\n", (ULONG)*status, (LONG)res));
  if(res != PROTO_RET_OK) {
    return res;
  }

  return PROTO_RET_OK;
}

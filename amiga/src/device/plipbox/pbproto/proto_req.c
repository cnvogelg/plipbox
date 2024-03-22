#define __NOLIBBASE__
#include <proto/exec.h>

#include "compiler.h"
#include "debug.h"

#include "proto_cmd.h"
#include "proto_req.h"
#include "req_shared.h"

int proto_req_get_cur_mac(proto_handle_t *proto, mac_t mac)
{
  int res;
  proto_cmd_req_t req = {
    .command = REQ_MAC_GET_CUR,
    .in_size = 0,
    .out_size = MAC_SIZE,
    .out_buf = mac
  };

  d8(("proto_req_get_cur_mac:"));
  res = proto_cmd_request(proto, &req);
  d8r((" res=%ld %02lx:%02lx:%02lx:%02lx:%02lx:%02lx\n", (LONG)res,
       (ULONG)mac[0], (ULONG)mac[1], (ULONG)mac[2],
       (ULONG)mac[3], (ULONG)mac[4], (ULONG)mac[5]));
  return res;
}

int proto_req_get_def_mac(proto_handle_t *proto, mac_t mac)
{
  int res;
  proto_cmd_req_t req = {
    .command = REQ_MAC_GET_DEF,
    .in_size = 0,
    .out_size = MAC_SIZE,
    .out_buf = mac
  };

  d8(("proto_req_get_def_mac:"));
  res = proto_cmd_request(proto, &req);
  d8r((" res=%ld %02lx:%02lx:%02lx:%02lx:%02lx:%02lx\n", (LONG)res,
       (ULONG)mac[0], (ULONG)mac[1], (ULONG)mac[2],
       (ULONG)mac[3], (ULONG)mac[4], (ULONG)mac[5]));
  return res;
}

int proto_req_set_cur_mac(proto_handle_t *proto, mac_t mac)
{
  int res;
  proto_cmd_req_t req = {
    .command = REQ_MAC_SET_CUR,
    .in_size = MAC_SIZE,
    .in_buf = mac,
    .out_size = 0
  };

  d8(("proto_req_set_cur_mac:"));
  res = proto_cmd_request(proto, &req);
  d8r((" res=%ld %02lx:%02lx:%02lx:%02lx:%02lx:%02lx\n", (LONG)res,
       (ULONG)mac[0], (ULONG)mac[1], (ULONG)mac[2],
       (ULONG)mac[3], (ULONG)mac[4], (ULONG)mac[5]));
  return res;
}

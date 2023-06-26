#ifndef CONFIG_H
#define CONFIG_H

#define COMMON_TEMPLATE "NOSPECIALSTATS/S,PRIORITY=PRI/K/N,BPS/K/N,MTU/K/N,"

struct CommonConfig {
   ULONG  nospecialstats;
   LONG  *priority;
   ULONG *bps;
   ULONG *mtu;
};

#endif

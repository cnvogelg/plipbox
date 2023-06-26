
#include <exec/resident.h>

#include "global.h"
#include "device.h"

static REGARGS LONG DevNull(void)
{
  return 0;
}

static const APTR LibVectors[] =
{
  (APTR)DevOpen,
  (APTR)DevClose,
  (APTR)DevExpunge,
  (APTR)DevNull,
  (APTR)DevBeginIO,
  (APTR)DevAbortIO,
  (APTR)-1 \
};

static const ULONG LibInitTab[] =
{
  sizeof(struct PLIPBase),
  (ULONG)LibVectors,
  (ULONG)NULL,
  (ULONG)DevInit
};

static const char DeviceName[] = DEVICE_NAME;
static const char DeviceId[]   = "Id: " DEVICE_ID "\r\n";

static const struct Resident ROMTag =
{
  RTC_MATCHWORD,
  (struct Resident *)&ROMTag,
  (struct Resident *)&ROMTag + 1,
  RTF_AUTOINIT,
  DEVICE_VERSION,
  NT_DEVICE,
  0, /* prio */
  (APTR)DeviceName,
  (APTR)DeviceId,
  (APTR)LibInitTab
};

void device_init(struct Library *base)
{
  base->lib_Node.ln_Type = NT_LIBRARY;
  base->lib_Node.ln_Pri  = 0;
  base->lib_Node.ln_Name = (char *)ROMTag.rt_Name;
  base->lib_Flags        = LIBF_CHANGED | LIBF_SUMUSED;
  base->lib_Version      = DEVICE_VERSION;
  base->lib_Revision     = DEVICE_REVISION;
  base->lib_IdString     = (char *)ROMTag.rt_IdString;
}

#ifndef DEVICE_H
#define DEVICE_H

PUBLIC ASM SAVEDS struct Device *DevInit(REG(d0,BASEPTR), REG(a0,BPTR seglist), REG(a6,struct Library *_SysBase));
PUBLIC ASM SAVEDS LONG DevOpen(REG(a1,struct IOSana2Req *ios2), REG(d0,ULONG unit), REG(d1,ULONG flags), REG(a6,BASEPTR));
PUBLIC ASM SAVEDS BPTR DevExpunge(REG(a6,BASEPTR));
PUBLIC ASM SAVEDS BPTR DevClose( REG(a1,struct IOSana2Req *ior), REG(a6,BASEPTR));
PUBLIC VOID DevTermIO(BASEPTR, struct IOSana2Req *ios2);
PUBLIC ASM SAVEDS VOID DevBeginIO(REG(a1,struct IOSana2Req *ios2), REG(a6,BASEPTR));
PUBLIC ASM SAVEDS LONG DevAbortIO(REG(a1,struct IOSana2Req *ior), REG(a6,BASEPTR));

extern void device_init(struct Library *base);

#endif

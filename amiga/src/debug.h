#ifndef __DEBUG_H
#define __DEBUG_H

#if (DEBUG != 0)
extern void KPrintF(char *, ...), KGetChar(void);
#endif

#if (DEBUG & 1)
#  define d(x) do { KPrintF("%s:%s:%ld:",__FILE__,__FUNC__,__LINE__); KPrintF x; } while(0)
#else
#  define d(x)
#endif

#if (DEBUG & 2)
#  define d2(x) do { KPrintF("%s:%s:%ld:",__FILE__,__FUNC__,__LINE__); KPrintF x; } while(0)
#else
#  define d2(x)
#endif

#if (DEBUG & 4)
#  define d4(x) do { KPrintF("%s:%s:%ld:",__FILE__,__FUNC__,__LINE__); KPrintF x; } while(0)
#else
#  define d4(x)
#endif

#if (DEBUG & 8)
#  define d8(x) do { KPrintF("%s:%s:%ld:",__FILE__,__FUNC__,__LINE__); KPrintF x; } while(0)
#else
#  define d8(x)
#endif

#endif


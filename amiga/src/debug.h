#ifndef DEBUG_H
#define DEBUG_H

#if (DEBUG != 0)
extern void KPrintF(char *, ...), KGetChar(void);
#endif

#if (DEBUG & 1)
#ifdef __FUNC__
#define d(x) do { KPrintF("%s:%s:%ld:",__FILE__,__FUNC__,__LINE__); KPrintF x; } while(0)
#else
#define d(x) do { KPrintF("%s:%ld:",__FILE__,__LINE__); KPrintF x; } while(0)
#endif
#else
#define d(x)
#endif

#if (DEBUG & 2)
#ifdef __FUNC__
#define d2(x) do { KPrintF("%s:%s:%ld:",__FILE__,__FUNC__,__LINE__); KPrintF x; } while(0)
#else
#define d2(x) do { KPrintF("%s:%ld:",__FILE__,__LINE__); KPrintF x; } while(0)
#endif
#else
#define d2(x)
#endif

#if (DEBUG & 4)
#ifdef __FUNC__
#define d4(x) do { KPrintF("%s:%s:%ld:",__FILE__,__FUNC__,__LINE__); KPrintF x; } while(0)
#else
#define d4(x) do { KPrintF("%s:%ld:",__FILE__,__LINE__); KPrintF x; } while(0)
#endif
#else
#define d4(x)
#endif

#if (DEBUG & 8)
#ifdef __FUNC__
#define d8(x) do { KPrintF("%s:%s:%ld:",__FILE__,__FUNC__,__LINE__); KPrintF x; } while(0)
#else
#define d8(x) do { KPrintF("%s:%ld:",__FILE__,__LINE__); KPrintF x; } while(0)
#endif
#else
#define d8(x)
#endif

#endif


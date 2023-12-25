#ifndef DEBUG_H
#define DEBUG_H

#if (DEBUG != 0)
extern void KPrintF(char *, ...), KGetChar(void);
#endif

#if (DEBUG & 1)
#ifdef __FUNC__
#define d(x)                                             \
  do                                                     \
  {                                                      \
    KPrintF("%s:%s:%ld:", __FILE__, __FUNC__, __LINE__); \
    KPrintF x;                                           \
  } while (0)
#else
#define d(x)                                \
  do                                        \
  {                                         \
    KPrintF("%s:%ld:", __FILE__, __LINE__); \
    KPrintF x;                              \
  } while (0)
#endif
#define dr(x)  \
  do           \
  {            \
    KPrintF x; \
  } while (0)
#else
#define d(x)
#define dr(x)
#endif

#if (DEBUG & 2)
#ifdef __FUNC__
#define d2(x)                                            \
  do                                                     \
  {                                                      \
    KPrintF("%s:%s:%ld:", __FILE__, __FUNC__, __LINE__); \
    KPrintF x;                                           \
  } while (0)
#else
#define d2(x)                               \
  do                                        \
  {                                         \
    KPrintF("%s:%ld:", __FILE__, __LINE__); \
    KPrintF x;                              \
  } while (0)
#endif
#define d2r(x) \
  do           \
  {            \
    KPrintF x; \
  } while (0)
#else
#define d2(x)
#define d2r(x)
#endif

#if (DEBUG & 4)
#ifdef __FUNC__
#define d4(x)                                            \
  do                                                     \
  {                                                      \
    KPrintF("%s:%s:%ld:", __FILE__, __FUNC__, __LINE__); \
    KPrintF x;                                           \
  } while (0)
#else
#define d4(x)                               \
  do                                        \
  {                                         \
    KPrintF("%s:%ld:", __FILE__, __LINE__); \
    KPrintF x;                              \
  } while (0)
#endif
#define d4r(x) \
  do           \
  {            \
    KPrintF x; \
  } while (0)
#else
#define d4(x)
#define d4r(x)
#endif

#if (DEBUG & 8)
#ifdef __FUNC__
#define d8(x)                                            \
  do                                                     \
  {                                                      \
    KPrintF("%s:%s:%ld:", __FILE__, __FUNC__, __LINE__); \
    KPrintF x;                                           \
  } while (0)
#else
#define d8(x)                               \
  do                                        \
  {                                         \
    KPrintF("%s:%ld:", __FILE__, __LINE__); \
    KPrintF x;                              \
  } while (0)
#endif
#define d8r(x) \
  do           \
  {            \
    KPrintF x; \
  } while (0)
#else
#define d8(x)
#define d8r(x)
#endif

#endif

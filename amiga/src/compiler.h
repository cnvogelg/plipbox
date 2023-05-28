#ifndef __COMPILER_H
#define __COMPILER_H

#ifdef __SASC
#define ASM     __asm              /* define registers for function args */
#define REG(r,t) register __ ## r t /* specify a register in arglist */
#define INLINE  __inline           /* inline this function */
#define REGARGS __regargs          /* pass args to this function in regs */
#define SAVEDS  __saveds           /* setup data segment reg. on entry */
#define FAR     __far              /* reference this object in far mode */
#define PUBLIC                     /* define a globally visible function */
#define PRIVATE static             /* define a locally visible function */
#else
#ifdef __VBCC__
#define ASM
#define REG(r,t) __reg( #r ) t
#define INLINE  inline
#define SAVEDS __saveds
#define FAR    __far
#define PUBLIC  extern
#define PRIVATE static
#else
#error Please define the above macros for your compiler
#endif

#endif


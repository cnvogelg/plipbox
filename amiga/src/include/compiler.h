#ifndef COMPILER_H
#define COMPILER_H

#ifdef __SASC
#define ASM __asm                  /* define registers for function args */
#define REG(r, t) register __##r t /* specify a register in arglist */
#define INLINE __inline            /* inline this function */
#define REGARGS __regargs          /* pass args to this function in regs */
#define SAVEDS __saveds            /* setup data segment reg. on entry */
#define FAR __far                  /* reference this object in far mode */

#define min __builtin_min

#else
#ifdef __VBCC__
#define ASM
#define REG(r, t) __reg(#r) t
#define INLINE inline
#define REGARGS
#define SAVEDS __saveds
#define FAR __far

#define min(a, b) ((a < b) ? (a) : (b))

#else
#ifdef __GNUC__
#define ASM
#define REG(r, t) t __asm(#r)
#define INLINE __inline __attribute__((always_inline))
#define REGARGS
#ifdef BASEREL
#define SAVEDS __saveds
#else
#define SAVEDS
#endif
#define FAR __far

#define min(a, b) ((a < b) ? (a) : (b))

#else
#error Please define the above macros for your compiler
#endif
#endif
#endif

#endif

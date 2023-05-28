#ifndef __COMPILER_H
#define __COMPILER_H

#ifdef __SASC
/* Implementation notes:
**
** SAS/C complains if a module contains only "static" and "extern" functions
** that there would be no exported symbol. Therefore, the PUBLIC macro
** is defined empty for SAS/C but should be "extern" if possible (for
** instance with GNU CC).
*/
#  define ASM     __asm              /* define registers for function args */
#  define REG(x)  register __ ## x   /* specify a register in arglist */
#  define INLINE  __inline           /* inline this function */
#  define REGARGS __regargs          /* pass args to this function in regs */
#  define SAVEDS  __saveds           /* setup data segment reg. on entry */
#  define FAR     __far              /* reference this object in far mode */
#  define PUBLIC                     /* define a globally visible function */
#  define PRIVATE static             /* define a locally visible function */
#else
#  error Please define the above macros for your compiler
#endif

#endif


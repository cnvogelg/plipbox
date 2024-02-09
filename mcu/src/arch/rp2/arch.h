#ifndef ARCH_H
#define ARCH_H

#include <stddef.h>
#include "nopgm.h"

#define INLINE        static inline
#define FORCE_INLINE  __attribute__((always_inline)) static inline
#define FAST_FUNC(x)  __not_in_flash_func(x)

#endif

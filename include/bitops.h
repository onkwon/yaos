#ifndef __BITOPS_H__
#define __BITOPS_H__

#include <asm/bitops.h>
#include <types.h>

static inline int fls(int v)
{
        return WORD_BITS - __clz(v);
}

#endif /* __BITOPS_H__ */

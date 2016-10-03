#ifndef __BITOPS_H__
#define __BITOPS_H__

#include <asm/bitops.h>
#include <types.h>

static inline int fls(int x)
{
        return WORD_BITS - __clz(x);
}

static inline int ffs(int x)
{
	return fls(x & -x);
}

static inline int log2(int x)
{
	if (!x)
		return INF;

	int sign = 1;

	if (x < 0) {
		sign = -sign;
		x = -x;
	}

	return (fls(x) - 1) * sign;
}

#endif /* __BITOPS_H__ */

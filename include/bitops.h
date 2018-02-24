#ifndef __BITOPS_H__
#define __BITOPS_H__

#include <asm/bitops.h>
#include <types.h>
#include <error.h>

static inline int fls(int x)
{
        return WORD_BITS - __clz(x);
}

static inline int ffs(int x)
{
	/* mask the least significant bit only */
	return fls(x & -x);
}

static inline int digits(const unsigned int n)
{
	assert(sizeof(n) == 4);

	const int tbl[] = {
		10, 10, 9, 9, 9, 8, 8, 8, 7, 7, 7, 7, 6, 6, 6, 5,
		5, 5, 4, 4, 4, 4, 3, 3, 3, 2, 2, 2, 1, 1, 1, 1,
		0 };
	const unsigned int base[] = {
		0,
		10,
		100,
		1000,
		10000,
		100000,
		1000000,
		10000000,
		100000000,
		1000000000 };
	int digits = tbl[__clz(n)];

	if (base[digits] <= n)
		digits++;

	return digits;
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

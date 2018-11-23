#ifndef __YAOS_TYPES_H__
#define __YAOS_TYPES_H__

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef volatile uintptr_t reg_t;

typedef enum {
	SLEEP_NAP	= 1,
	SLEEP_DEEP	= 2,
	SLEEP_BLACKOUT	= 3,
} sleep_t;

#define WORD_SIZE			sizeof(uintptr_t)
#define WORD_BITS			(WORD_SIZE << 3)

#define MASK(v, mask)			((v) & (mask))
#define MASK_RESET(v, mask)		((v) & ~(mask))
#define MASK_SET(v, mask)		((v) | (mask))

#define BASE(x, a)			((x) & ~(typeof(x)(a) - 1UL))
#define ALIGN(x, a)			BASE((x) + ((typeof(x))(a) - 1UL), a)

#define stringify(x)			#x
#define def2str(x)			stringify(x)

#define sbi(v, bit)			(v |= (1U << (bit)))
#define cbi(v, bit)			(v &= ~(1U << (bit)))
#define gbi(v, bit)			(((v) >> (bit)) & 1)

#define container_of(ptr, type, member) \
	((type *)((char *)(ptr) - (char *)&((type *)0)->member))

#if 0
#ifndef max
#define max(a, b)			({ \
		__typeof__(a) _a = (a); \
		__typeof__(b) _b = (b); \
		_a > _b ? _a : _b; \
})
#endif
#ifndef min
#define min(a, b)			({ \
		__typeof__(a) _a = (a); \
		__typeof__(b) _b = (b); \
		_a < _b ? _a : _b; \
})
#endif
#endif

static inline bool is_pow2(const unsigned int x)
{
	return !(x & (x - 1));
}

#endif /* __YAOS_TYPES_H__ */

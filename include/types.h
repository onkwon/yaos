#ifndef __YAOS_TYPES_H__
#define __YAOS_TYPES_H__

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limits.h>

/** Register type */
typedef volatile uintptr_t reg_t;

/** Sleep type */
typedef enum {
	/** Only the core stopped while all peripherals continue to run */
	SLEEP_NAP	= 1,
	/** The core and peripherals stop to run including external clock while
	 * still retaining the SRAM and register contents */
	SLEEP_DEEP	= 2,
	/** RTC and watchdog are the only ones continue to run. All the
	 * contents of SRAM or the registers are not preserved */
	SLEEP_BLACKOUT	= 3,
} sleep_t;

#define WORD_SIZE			sizeof(uintptr_t)
#define WORD_BITS			(WORD_SIZE * CHAR_BIT)

#define MASK(v, mask)			((v) & (mask))
#define MASK_RESET(v, mask)		((v) & ~(mask))
#define MASK_SET(v, mask)		((v) | (mask))

/** Align down */
#define BASE(x, a)			((x) & ~((typeof(x))(a) - 1UL))
/** Align up */
#define ALIGN(x, a)			BASE((x) + ((typeof(x))(a) - 1UL), a)

#define stringify(x)			#x
#define def2str(x)			stringify(x)

#define sbi(v, bit)			(v |= (1U << (bit)))
#define cbi(v, bit)			(v &= ~(1U << (bit)))
#define gbi(v, bit)			(((v) >> (bit)) & 1)

#define container_of(ptr, type, member) \
	((type *)((char *)(ptr) - (char *)&((type *)0)->member))

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

#define KHZ				1000UL
#define MHZ				(KHZ * KHZ)

/** Check if integral power of two */
static inline bool is_pow2(const uintptr_t x)
{
	return !(x & (x - 1));
}

#endif /* __YAOS_TYPES_H__ */

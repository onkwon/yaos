/**
 * @file atomic.h
 * @brief The file contains atomic primitives
 * @author Kyunghwan Kwon
 */

#ifndef __YAOS_ARMv7M_ATOMIC_H__
#define __YAOS_ARMv7M_ATOMIC_H__

#include <stdint.h>

#if !defined(TEST)
#define __ldrex(addr) __extension__ ({				\
	uintptr_t __result = 0;					\
	__asm__ __volatile__("ldrex %0, [%1]"			\
		: "=r"(__result)				\
		: "r"(addr)					\
		: "cc", "memory");				\
	__result;						\
})

/* return 0 if success */
#define __strex(value, addr) __extension__ ({			\
	uintptr_t __result = 0;					\
	__asm__ __volatile__("strex %0, %2, [%1]"		\
		: "=r"(__result)				\
		: "r"(addr), "r"(value)				\
		: "cc", "memory");				\
	__result;						\
})

#define __clrex()						\
	__asm__ __volatile__("clrex" ::: "cc", "memory")
#else /* defined(TEST) */
#define __ldrex(addr)		(*(volatile uintptr_t *)(addr))
#define __strex(val, addr) __extension__ ({			\
	uintptr_t __result = 0;					\
	*(volatile uintptr_t *)(addr) = (uintptr_t)(val);	\
	__result;						\
})
#define __clrex()
#endif

#define ldrex(addr)		__ldrex(addr)
#define strex(value, addr)	__strex(value, addr)

/**
 * Fetch and add
 *
 * @param p Pointer to data
 * @param inc Increment to be added
 * @return Original value at :c:data:`p`
 */
static inline uintptr_t atomic_faa(uintptr_t *p, uintptr_t inc)
{
	uintptr_t val;

	do {
		val = ldrex(p);
	} while (strex(val + inc, p));

	return val;
}

/**
 * Load linked
 *
 * @p Pointer to data
 * @return 0 on success
 */
static inline uintptr_t atomic_ll(void *p)
{
	return ldrex(p);
}

/**
 * Store conditional
 *
 * @param p Pointer to data
 * @param newval Value to store
 * @return 0 on success
 */
static inline uintptr_t atomic_sc(void *p, uintptr_t newval)
{
	return strex(newval, p);
}

#endif /* __YAOS_ARMv7M_ATOMIC_H__ */

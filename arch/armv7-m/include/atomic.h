/**
 * @file atomic.h
 * @brief The file contains atomic primitives
 * @author Kyunghwan Kwon
 */

#ifndef __YAOS_ARMv7M_ATOMIC_H__
#define __YAOS_ARMv7M_ATOMIC_H__

#include <stdint.h>

#if !defined(TEST)
#define __ldrex(addr) __extension__ ({			\
	uintptr_t __result = 0;				\
	__asm__ __volatile__("ldrex %0, [%1]"		\
		: "=r"(__result)			\
		: "r"(addr)				\
		: "cc", "memory");			\
	__result;					\
})

/* return 0 if success */
#define __strex(value, addr) __extension__ ({		\
	uintptr_t __result = 0;				\
	__asm__ __volatile__("strex %0, %2, [%1]"	\
		: "=r"(__result)			\
		: "r"(addr), "r"(value)			\
		: "cc", "memory");			\
	__result;					\
})

#define __clrex()					\
	__asm__ __volatile__("clrex" ::: "cc", "memory")
#else /* defined(TEST) */
#define __ldrex(addr)		(*(addr))
#define __clrex()
static inline int __strex(uintptr_t value, uintptr_t *addr)
{
	*addr = value;
	return 0;
}
#endif

#define ldrex(addr)		__ldrex(addr)
#define strex(value, addr)	__strex(value, addr)

/**
 * @brief Fetch and Add
 * @param *p a pointer to the variable to add
 * @param a add a to *p
 * @return returns *p + a
 */
static inline intptr_t atomic_faa(intptr_t *p, intptr_t a)
{
	intptr_t val;

	do {
		val = ldrex(p) + a;
	} while (strex(val, p));

	return val;
}

static inline uintptr_t atomic_ll(uintptr_t *p)
{
	return ldrex(p);
}

/**
 * @brief store conditional
 * @param *p
 * @param newval
 * @return returns 0 if succeed
 */
static inline uintptr_t atomic_sc(uintptr_t *p, uintptr_t newval)
{
	return strex(newval, p);
}

#endif /* __YAOS_ARMv7M_ATOMIC_H__ */

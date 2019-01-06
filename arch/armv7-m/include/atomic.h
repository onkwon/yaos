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
		: "=&r"(__result)				\
		: "r"(addr), "r"(value)				\
		: "cc", "memory");				\
	__result;						\
})
#define __ldrexb(addr) __extension__ ({				\
	uint8_t __result = 0;					\
	__asm__ __volatile__("ldrexb %0, [%1]"			\
		: "=r"(__result)				\
		: "r"(addr)					\
		: "cc", "memory");				\
	__result;						\
})
#define __strexb(value, addr) __extension__ ({			\
	uint8_t __result = 0;					\
	__asm__ __volatile__("strexb %0, %2, [%1]"		\
		: "=&r"(__result)				\
		: "r"(addr), "r"(value)				\
		: "cc", "memory");				\
	__result;						\
})
#define __ldrexh(addr) __extension__ ({				\
	uint16_t __result = 0;					\
	__asm__ __volatile__("ldrexh %0, [%1]"			\
		: "=r"(__result)				\
		: "r"(addr)					\
		: "cc", "memory");				\
	__result;						\
})
#define __strexh(value, addr) __extension__ ({			\
	uint16_t __result = 0;					\
	__asm__ __volatile__("strexh %0, %2, [%1]"		\
		: "=&r"(__result)				\
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
#define __ldrexb(addr)		(*(volatile uint8_t *)(addr))
#define __strexb(val, addr) __extension__ ({			\
	uint8_t __result = 0;					\
	*(volatile uint8_t *)(addr) = (uint8_t)(val);	\
	__result;						\
})
#define __ldrexh(addr)		(*(volatile uint16_t *)(addr))
#define __strexh(val, addr) __extension__ ({			\
	uint16_t __result = 0;					\
	*(volatile uint16_t *)(addr) = (uint16_t)(val);	\
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
static inline int atomic_faa(int *p, int inc)
{
	int val;

	do {
		val = __ldrex(p);
	} while (__strex(val + inc, p));

	return val;
}

/**
 * Fetch and add byte
 *
 * @param p Pointer to data
 * @param inc Increment to be added
 * @return Original value at :c:data:`p`
 */
static inline int8_t atomic_faab(int8_t *p, int8_t inc)
{
	int8_t val;

	do {
		val = __ldrexb(p);
	} while (__strexb(val + inc, p));

	return val;
}

/**
 * Fetch and add half word
 *
 * @param p Pointer to data
 * @param inc Increment to be added
 * @return Original value at :c:data:`p`
 */
static inline int16_t atomic_faah(int16_t *p, int16_t inc)
{
	int16_t val;

	do {
		val = __ldrexh(p);
	} while (__strexh(val + inc, p));

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
	return __ldrex(p);
}
/**
 * Store conditional
 *
 * @param p Pointer to data
 * @param newval Value to store
 * @return 0 on success
 */
static inline uintptr_t atomic_sc(uintptr_t newval, void *p)
{
	return __strex(newval, p);
}
/**
 * Load linked byte
 *
 * @p Pointer to data
 * @return 0 on success
 */
static inline uint8_t atomic_llb(void *p)
{
	return __ldrexb(p);
}
/**
 * Store conditional byte
 *
 * @param p Pointer to data
 * @param newval Value to store
 * @return 0 on success
 */
static inline uint8_t atomic_scb(uint8_t newval, void *p)
{
	return __strexb(newval, p);
}
/**
 * Load linked half-word
 *
 * @p Pointer to data
 * @return 0 on success
 */
static inline uint16_t atomic_llh(void *p)
{
	return __ldrexh(p);
}
/**
 * Store conditional half-word
 *
 * @param p Pointer to data
 * @param newval Value to store
 * @return 0 on success
 */
static inline uint16_t atomic_sch(uint16_t newval, void *p)
{
	return __strexh(newval, p);
}

#if 0
#include <stdbool.h>

/** Atomic compare and set
 *
 * @param p  Pointer to data
 * @param oldval Value to swap out
 * @param newval Value to swap in
 * @return true on success or false
 */
static inline bool atomic_cas(void *p, uintptr_t oldval, uintptr_t newval)
{
#if 0
	uintptr_t val = atomic_ll(p);

	if (val != oldval)
		return false;
#else
	oldval = atomic_ll(p);
#endif

	return !atomic_sc(newval, p);
}
#endif

#endif /* __YAOS_ARMv7M_ATOMIC_H__ */

#ifndef __STM32_LOCK_H__
#define __STM32_LOCK_H__

/* return 0 if success */
static inline int atomic_set(int *p, int v)
{
	int result;

	__asm__ __volatile__(
			"ldrex	%1, [%0]	\n\t"
			"strex	%1, %2, [%0]	\n\t"
			: "+r"(p), "=&r"(result)
			: "r"(v)
			: "memory");

	return result;
}

#endif /* __STM32_LOCK_H__ */

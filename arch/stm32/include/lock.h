#ifndef __STM32_LOCK_H__
#define __STM32_LOCK_H__

/* The result is 0 if successful */
static inline int set_atomic(int *p, int v)
{
	int result;

	__asm__ __volatile__(
			"ldrex	%1, [%0]	\n\t"
			"strex	%1, %2, [%0]	\n\t"
			: "+r"(p), "=&r"(result)
			: "r"(v));

	return result;
}

int atomic_dec(void *s);
int atomic_inc(void *s);

#endif /* __STM32_LOCK_H__ */

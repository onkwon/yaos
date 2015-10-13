#ifndef __ARMv7A_LOCK_H__
#define __ARMv7A_LOCK_H__

/*
#define read_lock(count)	\
	volatile int *p = &count; \
	__asm__ __volatile__( \
		"try:"	"ldrex		r12, [%0]	\n\t" \
			"cmp		r12, #0		\n\t" \
			"addgt		r12, r12, #1	\n\t" \
			"strexgt	r11, r12, [%0]	\n\t" \
			"cmpgt		r11, #0		\n\t" \
			"bne		try		\n\t" \
			: "+r"(p) \
			:: "r11", "r12", "memory")
*/

/*
#define spin_lock(count)	\
	__asm__ __volatile__( \
		"try:"	"ldrex		r12, [%0]	\n\t" \
			"cmp		r12, #0		\n\t" \
			"subgt		r12, r12, #1	\n\t" \
			"strexgt	r12, r12, [%0]	\n\t" \
			"cmpgt		r12, #0		\n\t" \
			"bne		try		\n\t" \
			: "+r"(&count) \
			:: "r12", "memory")
*/

/* return 0 if success */
static inline int atomic_set(int *p, int v)
{
	*p = v;
	return 0;
	int result;

	__asm__ __volatile__(
			"ldrex	%1, [%0]	\n\t"
			"strex	%1, %2, [%0]	\n\t"
			: "+r"(p), "=&r"(result)
			: "r"(v)
			: "memory");

	return result;
}

#endif /* __ARMv7A_LOCK_H__ */

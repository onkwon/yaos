#ifndef __ARMv7A_LOCK_H__
#define __ARMv7A_LOCK_H__

/*
#define spin_lock(count)					\
	__asm__ __volatile__(					\
		"try:"	"ldrex		r12, [%0]	\n\t"	\
			"cmp		r12, #0		\n\t"	\
			"subgt		r12, r12, #1	\n\t"	\
			"strexgt	r12, r12, [%0]	\n\t"	\
			"cmpgt		r12, #0		\n\t"	\
			"bne		try		\n\t"	\
			: "+r"(&count)				\
			:: "r12", "memory")

#define read_lock(count)					\
	volatile int *p = &count;				\
	__asm__ __volatile__(					\
		"try:"	"ldrex		r12, [%0]	\n\t"	\
			"cmp		r12, #0		\n\t"	\
			"addgt		r12, r12, #1	\n\t"	\
			"strexgt	r11, r12, [%0]	\n\t"	\
			"cmpgt		r11, #0		\n\t"	\
			"bne		try		\n\t"	\
			: "+r"(p)				\
			:: "r11", "r12", "memory")

#define __ldrex(addr)				({	\
	unsigned int __result = 0;			\
	__asm__ __volatile__("ldrex %0, [%1]"		\
		: "=r"(__result)			\
		: "r"(addr)				\
		: "memory");				\
	__result;					\
})

#define __strex(value, addr)			({	\
	unsigned int __result = 0;			\
	__asm__ __volatile__("strex %0, %2, [%1]"	\
		: "=r"(result)				\
		: "r"(addr), "r"(value)			\
		: "memory");				\
	__result;					\
})

static inline int atomic_set(int *p, int v)
{
	int result;

	__ldrex(p);
	result = __strex(v, p);

	return result;
}
*/

#define __ldrex(addr)				({	\
	unsigned int __result = 0;			\
	__asm__ __volatile__("ldrex %0, [%1]"		\
		: "=r"(__result)			\
		: "r"(addr)				\
		: "memory");				\
	__result;					\
})

/* return 0 if success */
#define __strex(value, addr)			({	\
	unsigned int __result = 0;			\
	__asm__ __volatile__("strex %0, %2, [%1]"	\
		: "=&r"(__result)			\
		: "r"(addr), "r"(value)			\
		: "memory");				\
	__result;					\
})

#endif /* __ARMv7A_LOCK_H__ */

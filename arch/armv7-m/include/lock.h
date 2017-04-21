#ifndef __ARMv7M_LOCK_H__
#define __ARMv7M_LOCK_H__

#if 1
#define __ldrex(addr)				({	\
	unsigned int __result = 0;			\
	__asm__ __volatile__("ldrex %0, [%1]"		\
		: "=r"(__result)			\
		: "r"(addr)				\
		: "cc", "memory");			\
	__result;					\
})

/* return 0 if success */
#define __strex(value, addr)			({	\
	unsigned int __result = 0;			\
	__asm__ __volatile__("strex %0, %2, [%1]"	\
		: "=r"(__result)			\
		: "r"(addr), "r"(value)			\
		: "cc", "memory");			\
	__result;					\
})

#define __clrex()					\
	__asm__ __volatile__("clrex" ::: "cc", "memory")
#else
int __ldrex(void *addr);
int __strex(int val, void *addr);
#endif

static inline void atomic_sub(int i, lock_t *counter)
{
	int count, result;

	__asm__ __volatile__(
		"1:"	"ldrex	%0, [%2]		\n\t"
			"sub	%0, %0, %3		\n\t"
			"strex	%1, %0, [%2]		\n\t"
			"teq	%1, #0			\n\t"
			"bne	1b			\n\t"
			: "=&r"(count), "=&r"(result)
			: "r"(counter), "Ir"(i)
			: "cc");
}

static inline void atomic_add(int i, lock_t *counter)
{
	int count, result;

	__asm__ __volatile__(
		"1:"	"ldrex	%0, [%2]		\n\t"
			"add	%0, %0, %3		\n\t"
			"strex	%1, %0, [%2]		\n\t"
			"teq	%1, #0			\n\t"
			"bne	1b			\n\t"
			: "=&r"(count), "=&r"(result)
			: "r"(counter), "Ir"(i)
			: "cc");
}

static inline void read_lock_spinning(lock_t *lock)
{
	int count, result;

	__asm__ __volatile__(
		"1:"	"ldrex	%0, [%2]		\n\t"
			"cmp	%0, #0			\n\t"
			"ble	1b			\n\t"
			"add	%0, %0, #1		\n\t"
			"strex	%1, %0, [%2]		\n\t"
			"cmp	%1, #0			\n\t"
			"bne	1b			\n\t"
			"dmb				\n\t"
			: "=&r"(count), "=&r"(result)
			: "r"(lock)
			: "cc", "memory");
}

static inline void write_lock_spinning(lock_t *lock)
{
	int count, result;

	__asm__ __volatile__(
		"1:"	"ldrex	%0, [%2]		\n\t"
			"cmp	%0, %3			\n\t"
			"bne	1b			\n\t"
			"sub	%0, %0, #1		\n\t"
			"strex	%1, %0, [%2]		\n\t"
			"cmp	%1, #0			\n\t"
			"bne	1b			\n\t"
			"dmb				\n\t"
			: "=&r"(count), "=&r"(result)
			: "r"(lock), "I"(UNLOCKED)
			: "cc", "memory");
}

void __semaphore_dec(struct semaphore *sem, int ms);
int __semaphore_dec_wait(struct semaphore *sem, int ms);
void __semaphore_inc(struct semaphore *sem);
void __lock_atomic(lock_t *counter);
void __unlock_atomic(lock_t *counter);

#endif /* __ARMv7M_LOCK_H__ */

#ifndef __ARMv7M_LOCK_H__
#define __ARMv7M_LOCK_H__

#define __ldrex(addr)				({	\
	unsigned int __result = 0;			\
	__asm__ __volatile__("ldrex %0, [%1]"		\
		: "=r"(__result)			\
		: "r"(addr)				\
		: "cc");				\
	__result;					\
})

/* return 0 if success */
#define __strex(value, addr)			({	\
	unsigned int __result = 0;			\
	__asm__ __volatile__("strex %0, %2, [%1]"	\
		: "=r"(__result)			\
		: "r"(addr), "r"(value)			\
		: "cc");				\
	__result;					\
})

static inline void semaphore_dec(struct semaphore *sem)
{
	int count, result;

	__asm__ __volatile__(
		"1:"	"ldrex	%0, [%2]		\n\t"
			"cmp	%0, #0			\n\t"
			"bgt	2f			\n\t"
			"mov	r0, %3			\n\t"
			"bl	sleep_in_waitqueue	\n\t"
			"b	1b			\n\t"
		"2:"	"sub	%0, #1			\n\t"
			"strex	%1, %0, [%2]		\n\t"
			"cmp	%1, #0			\n\t"
			"bne	1b			\n\t"
			"dmb				\n\t"
			: "=&r"(count), "=&r"(result)
			: "r"(&sem->count), "r"(&sem->wq)
			: "r0", "lr", "cc", "memory");
}

static inline void semaphore_inc(struct semaphore *sem)
{
	int count, result;

	__asm__ __volatile__(
		"1:"	"ldrex	%0, [%2]		\n\t"
			"add	%0, #1			\n\t"
			"strex	%1, %0, [%2]		\n\t"
			"cmp	%1, #0			\n\t"
			"bne	1b			\n\t"
			"cmp	%0, #0			\n\t"
			"dmb				\n\t"
			"itt	gt			\n\t"
			"movgt	r0, %3			\n\t"
			"blgt	shake_waitqueue_out	\n\t"
			: "=&r"(count), "=&r"(result)
			: "r"(&sem->count), "r"(&sem->wq)
			: "r0", "lr", "cc", "memory");
}

static inline void lock_dec_spinning(lock_t *counter)
{
	int count, result;

	__asm__ __volatile__(
		"1:"	"ldrex	%0, [%2]		\n\t"
			"cmp	%0, #0			\n\t"
			"ble	1b			\n\t"
			"sub	%0, %0, #1		\n\t"
			"strex	%1, %0, [%2]		\n\t"
			"cmp	%1, #0			\n\t"
			"bne	1b			\n\t"
			"dmb				\n\t"
			: "=&r"(count), "=&r"(result)
			: "r"(counter)
			: "cc", "memory");
}

static inline void lock_inc_spinning(lock_t *counter)
{
	int count, result;

	__asm__ __volatile__(
		"1:"	"ldrex	%0, [%2]		\n\t"
			"add	%0, %0, #1		\n\t"
			"strex	%1, %0, [%2]		\n\t"
			"cmp	%1, #0			\n\t"
			"bne	1b			\n\t"
			"dmb				\n\t"
			: "=&r"(count), "=&r"(result)
			: "r"(counter)
			: "cc", "memory");
}

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

#endif /* __ARMv7M_LOCK_H__ */

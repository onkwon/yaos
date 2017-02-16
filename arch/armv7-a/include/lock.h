#ifndef __ARMv7A_LOCK_H__
#define __ARMv7A_LOCK_H__

/* FIXME: the instructions, ldrex and strex, don't work
 * It seems like the instructions, ldrex and strex, don't work before some
 * condition match? Should MMU be turned on to make it work? */
#if 0
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
			"mcr	p15, 0, r0, c7, c10, 5	\n\t"
			: "=&r"(count), "=&r"(result)
			: "r"(&sem->counter), "r"(&sem->wq)
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
			"mcr	p15, 0, r0, c7, c10, 5	\n\t"
			"itt	gt			\n\t"
			"movgt	r0, %3			\n\t"
			"blgt	shake_waitqueue_out	\n\t"
			: "=&r"(count), "=&r"(result)
			: "r"(&sem->counter), "r"(&sem->wq)
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
			"mcr	p15, 0, r0, c7, c10, 5	\n\t"
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
			"mcr	p15, 0, r0, c7, c10, 5	\n\t"
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
			"mcr	p15, 0, r0, c7, c10, 5	\n\t"
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
			"mcr	p15, 0, r0, c7, c10, 5	\n\t"
			: "=&r"(count), "=&r"(result)
			: "r"(lock), "I"(UNLOCKED)
			: "cc", "memory");
}
#else
static inline void semaphore_dec(struct semaphore *sem)
{
	extern void sleep_in_waitqueue(struct waitqueue_head *q);

	while (sem->counter <= 0) {
		sleep_in_waitqueue(&sem->wq);
	}

	sem->counter = sem->counter - 1;
}

static inline void semaphore_inc(struct semaphore *sem)
{
	extern void shake_waitqueue_out(struct waitqueue_head *q);

	sem->counter = sem->counter + 1;

	if (sem->counter > 0) {
		shake_waitqueue_out(&sem->wq);
	}
}

static inline void lock_dec_spinning(lock_t *counter)
{
	while (*counter <= 0) ;

	*counter = *counter - 1;
}

static inline void lock_inc_spinning(lock_t *counter)
{
	*counter = *counter + 1;
}

static inline void atomic_sub(int i, lock_t *counter)
{
	*counter = *counter - i;
}

static inline void atomic_add(int i, lock_t *counter)
{
	*counter = *counter + i;
}

static inline void read_lock_spinning(lock_t *counter)
{
	while (*counter <= 0) ;

	*counter = *counter + 1;
}

static inline void write_lock_spinning(lock_t *counter)
{
	while (*counter != UNLOCKED) ;

	*counter = *counter - 1;
}
#endif

#endif /* __ARMv7A_LOCK_H__ */

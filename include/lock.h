#ifndef __LOCK_H__
#define __LOCK_H__

#define preempt_disable()
#define preempt_enable()
#define preempt_count()

#define sei()		__asm__ __volatile__("cpsie i")
#define cli()		__asm__ __volatile__("cpsid i")

#define irq_save(flags) \
	__asm__ __volatile__("mrs %0, primask" : "=r"(flags))
#define irq_restore(flags) \
	__asm__ __volatile__("msr primask, %0" :: "r"(flags))

struct semaphore {
	volatile int count;
	/* wait queue */
};

#define semaphore_new(name, count) \
	struct semaphore name = { count }

static inline void semaphore_init(struct semaphore *s, int count)
{
	s->count = count;
}

static inline void semaphore_down(struct semaphore *s)
{
	while (s->count <= 0) {
		/* go sleep */
	}

	s->count -= 1;
}

static inline void semaphore_up(struct semaphore *s)
{
	s->count += 1;

	if (s->count > 0) {
		/* wake up tasks in the wait queue */
	}
}

static inline void semaphore_down_atomic(struct semaphore *s, unsigned long *flags)
{
	irq_save(*flags);

waiting4key:
	while (s->count <= 0) ;

	cli();
	/* in case an interrupt occures between cli() and exiting from loop */
	if (s->count <= 0) {
		DBUG(("critical region got polluted.\n"));
		irq_restore(*flags);
		goto waiting4key;
	}

	s->count -= 1;
}

static inline void semaphore_up_atomic(struct semaphore *s, unsigned long *flags)
{
	s->count += 1;

	irq_restore(*flags);
}

#define DEFINE_MUTEX(name)		semaphore_new(name, 1)
#define mutex_lock(s)			semaphore_down(s)
#define mutex_unlock(s)			semaphore_up(s)

#define DEFINE_SPINLOCK(name)		semaphore_new(name, 1)
#define spinlock_irqsave(s, f)		semaphore_down_atomic(s, f)
#define spinlock_irqrestore(s, f)	semaphore_up_atomic(s, f)

/*
static inline int set_atomw(int var, int val)
{
	int res;

	__asm__ __volatile__("ldrex %1, %0	\n\t"
			     "strex %1, %2, %0	\n\t"
			     : "+m" (var), "=l" (res) : "l" (val));

	return res;
}
*/

#endif /* __LOCK_H__ */

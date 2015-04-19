#ifndef __LOCK_H__
#define __LOCK_H__

#define preempt_disable()
#define preempt_enable()
#define preempt_count()

#ifdef MACHINE
#include <asm/lock.h>
#endif

struct semaphore {
	volatile int count;
	/* wait queue */
};

#define semaphore_new(name, count) \
	struct semaphore name = { count }

#define semaphore_init(name, v)		(name.count = v)

#define semaphore_down(s) do { \
	while (s.count <= 0) { \
		/* go sleep */ \
	} \
} while (set_atomic((int *)&s.count, s.count-1))

#define semaphore_up(s) do { \
	while (set_atomic((int *)&s.count, s.count+1)) ; \
	if (s.count > 0) { \
		/* wake up tasks in the wait queue */ \
	} \
} while (0)

#define DEFINE_MUTEX(name)		semaphore_new(name, 1)

#define mutex_lock(s)			semaphore_down(s)
#define mutex_unlock(s)			semaphore_up(s)

#define DEFINE_SPINLOCK(name)		semaphore_new(name, 1)

#define spin_lock(s) do { \
	while (s.count <= 0) ; \
} while (set_atomic((int *)&s.count, s.count-1))

#define spin_unlock(s) \
	while (set_atomic((int *)&s.count, s.count+1))

#include <io.h>

#define semaphore_down_atomic(s, f) do { \
	irq_save(f); \
	cli(); \
	spin_lock(s); \
} while (0)

#define semaphore_up_atomic(s, f) do { \
	spin_unlock(s); \
	irq_restore(f); \
} while (0)

#define spinlock_irqsave(s, f)		semaphore_down_atomic(s, f)
#define spinlock_irqrestore(s, f)	semaphore_up_atomic(s, f)

#endif /* __LOCK_H__ */

#ifndef __YAOS_LOCK_H__
#define __YAOS_LOCK_H__

#include "io.h"
#include "kernel/interrupt.h"
#include "list.h"
#include <stdint.h>

#define DEFINE_LOCK(name)	struct lock name = { 0, 0, { NULL } }

struct lock {
	int ticket;
	int turn;
	struct list waitq; // FIXME: the data type must be lock-free
};

static inline void spin_lock(struct lock * const lock)
{
	int myturn = atomic_faa(&lock->ticket, 1);
	while (ACCESS_ONCE(lock->turn) != myturn) ;
}

static inline void spin_unlock(struct lock * const lock)
{
	dmb();
	ACCESS_ONCE(lock->turn) += 1;
}

#if defined(CONFIG_SMP)
# error "No support for SMP"
#else
# define spin_lock_isr(x)
# define spin_unlock_isr(x)
#endif

#if !defined(TEST)
static inline void spin_lock_irqsave(struct lock * const lock, uintptr_t *irqflag)
{
	irq_save(*irqflag);
	local_irq_disable();
#if defined(CONFIG_SMP)
	spin_lock(lock);
#else
	(void)lock;
#endif
}

static inline void spin_unlock_irqrestore(struct lock * const lock, uintptr_t irqflag)
{
#if defined(CONFIG_SMP)
	spin_unlock(lock);
#else
	(void)lock;
#endif
	irq_restore(irqflag);
}

// TODO: replace it with real mutex implemeting waitqueue
static inline void mutex_lock(struct lock * const lock)
{
	spin_lock(lock);
}

static inline void mutex_unlock(struct lock * const lock)
{
	spin_unlock(lock);
}
#else // defined(TEST)
# define spin_lock_irqsave(lock, flag)
# define spin_unlock_irqrestore(lock, flag)
# define mutex_lock(lock)
# define mutex_unlock(lock)
#endif

void lock_init(struct lock * const lock);

#if 0
int semaphore_dec(struct semaphore *sem, int timeout_ms)
{
	counter -= 1;
	if (counter < 0)
		block();
}

void semaphore_inc(struct semaphore *sem)
{
	counter += 1;
	if (counter > 0)
		wakeup();
}
#endif

#endif /* __YAOS_LOCK_H__ */

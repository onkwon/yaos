#ifndef __YAOS_LOCK_H__
#define __YAOS_LOCK_H__

#include "io.h"
#include "list.h"

#include "kernel/interrupt.h"

#include <stdint.h>

#define DEFINE_LOCK(name)	\
	struct lock name = { { INIT_LISTQ_HEAD((name).waitq_head) }, {0} }
#define DEFINE_MUTEX(name)	\
	struct lock name = { { INIT_LISTQ_HEAD((name).waitq_head) }, {1} }

/**
 * The number of threads that simultaneously accesses the same resouce must
 * not exceed (2^8 - 1) as the data type of `ticket` is `int8_t`.
 */
typedef struct lock {
	union {
		struct listq_head waitq_head; // TODO: better to be lock-free data type
		uintptr_t irqflag;
	};

	union {
		int16_t counter;
		struct {
			int8_t ticket;
			volatile int8_t turn;
		};
	};
} lock_t;

typedef lock_t sem_t;

static inline void spin_lock(struct lock * const lock)
{
	int8_t myturn;

	dmb();
	myturn = atomic_faab(&lock->ticket, 1);
	while (lock->turn != myturn) ;
}

static inline void spin_unlock(struct lock * const lock)
{
	dmb();
	// guaranteed only the winner can write at a time
	lock->turn = (uint8_t)(lock->turn + 1);
}

#if defined(CONFIG_SMP)
# error "No support for SMP"
#else
# define spin_lock_critical(x)
# define spin_unlock_critical(x)
#endif

#if !defined(TEST)
static inline void spin_lock_irqsave(struct lock * const lock)
{
	irq_save(lock->irqflag);
	local_irq_disable();
#if defined(CONFIG_SMP)
	spin_lock(lock);
#else
	(void)lock;
#endif
}

static inline void spin_unlock_irqrestore(struct lock * const lock)
{
#if defined(CONFIG_SMP)
	spin_unlock(lock);
#else
	(void)lock;
#endif
	irq_restore(lock->irqflag);
	listq_init(&lock->waitq_head);
}

static inline void mutex_lock_critical(struct lock * const lock)
{
	spin_lock_irqsave(lock);
}

static inline void mutex_unlock_critical(struct lock * const lock)
{
	spin_unlock_irqrestore(lock);
}

void mutex_lock(struct lock * const lock);
void mutex_unlock(struct lock * const lock);
/** If the value of the semaphore is negative, the calling task blocks; one
 * of the blocked tasks wakes up when another task calls sem_post */
int sem_wait(struct lock * const lock);
/** Increments the value of the semaphore and wakes up a blocked task waiting
 * on the semaphore, if any. */
int sem_post(struct lock * const lock);

#else // defined(TEST)
# define spin_lock_irqsave(lock)
# define spin_unlock_irqrestore(lock)
# define mutex_lock(lock)
# define mutex_unlock(lock)
# define mutex_lock_atomic(lock)
# define mutex_unlock_atomic(lock)
# define sem_wait(sem)
# define sem_post(sem)
#endif

void lock_init(struct lock * const lock);
void mutex_init(struct lock * const lock);
/** Initialize a semaphore
 *
 * @param lock points to a semaphore object to initialize
 * @param shared not used
 * @param value is an initial value to set the semaphore to
 * @return 0 on success
 */
int sem_init(struct lock * const lock, int shared, uint16_t value);

#endif /* __YAOS_LOCK_H__ */

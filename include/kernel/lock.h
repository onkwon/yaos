#ifndef __LOCK_H__
#define __LOCK_H__

#include <types.h>

#ifdef MACHINE
#include <asm/lock.h>
#endif

#define UNLOCKED			1
#define DEFINE_LOCK(name)		lock_t name = UNLOCKED
#define INIT_LOCK(name)			(name = UNLOCKED)
#define is_locked(count)		(count <= 0)

#define up(count)			atomic_set((int *)&count, count + 1)
#define down(count)			atomic_set((int *)&count, count - 1)

/* spinlock */
#ifdef CONFIG_SMP
#define DEFINE_SPINLOCK(name)		DEFINE_LOCK(name)
#define spin_lock(count)		do { \
	while (is_locked(count)) ; \
} while (down(count))
#define spin_unlock(count)		while (up(count))
#else
#define DEFINE_SPINLOCK(name)
#define spin_lock(count)
#define spin_unlock(count)
#endif /* CONFIG_SMP */

#define spin_lock_irqsave(lock, flag)	do { \
	irq_save(flag); \
	local_irq_disable(); \
	spin_lock(lock); \
} while (0)

#define spin_unlock_irqrestore(lock, flag) do { \
	spin_unlock(lock); \
	irq_restore(flag); \
} while (0)

/* semaphore */
struct semaphore {
	lock_t count;
	/* can't use `waitqueue_head_t` because of circular dependency.
	 * move lock_t typedef into types.h? */
	lock_t wait_lock;
	struct list wait_list;
};

#define DEFINE_SEMAPHORE(name, v) \
	struct semaphore name = { \
		.count = v, \
		.wait_lock = UNLOCKED, \
		.wait_list = INIT_LIST_HEAD((name).wait_list), \
	}

#define INIT_SEMAPHORE(name, v)	name = (struct semaphore){ \
	.count = v, \
	.wait_lock = UNLOCKED, \
	.wait_list = INIT_LIST_HEAD((name).wait_list), \
}

#include <kernel/waitqueue.h>
#include <kernel/sched.h>

#define semaphore_down(s) { \
	DEFINE_WAIT(wait); \
	unsigned irqflag; \
	do { \
		while (is_locked(s.count)) { \
			spin_lock_irqsave(s.wait_lock, irqflag); \
			if (list_empty(&wait.link)) \
				list_add(&wait.link, s.wait_list.prev); \
			set_task_state(current, TASK_WAITING); \
			spin_unlock_irqrestore(s.wait_lock, irqflag); \
			schedule(); \
		} \
	} while (atomic_set((int *)&s.count, s.count-1)); \
}

#define semaphore_up(s) do { \
	struct task *task; \
	unsigned irqflag; \
	while (atomic_set((int *)&s.count, s.count+1)) ; \
	if (!is_locked(s.count)) { \
		spin_lock_irqsave(s.wait_lock, irqflag); \
		if (s.wait_list.next != &s.wait_list) { \
			task = get_container_of(s.wait_list.next, \
					struct waitqueue, link)->task; \
			set_task_state(task, TASK_RUNNING); \
			runqueue_add(task); \
			list_del(s.wait_list.next); \
		} \
		spin_unlock_irqrestore(s.wait_lock, irqflag); \
	} \
} while (0)

/* mutex */
typedef struct semaphore mutex_t;

#define DEFINE_MUTEX(name)		DEFINE_SEMAPHORE(name, 1)
#define INIT_MUTEX(name)		INIT_SEMAPHORE(name, 1)
#define mutex_lock(count)		semaphore_down(count)
#define mutex_unlock(count)		semaphore_up(count)

/* reader-writer spin lock */
#define DEFINE_RWLOCK(name)		DEFINE_LOCK(name)
#define read_lock(count)		do { \
	while (is_locked(count)) ; \
} while (up(count))
#define read_unlock(count)		while (down(count))
#define write_lock(count)		do { \
	while (count != UNLOCKED) ; \
} while (down(count))
#define write_unlock(count)		up(count)

#endif /* __LOCK_H__ */

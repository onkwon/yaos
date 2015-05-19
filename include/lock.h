#ifndef __LOCK_H__
#define __LOCK_H__

#define preempt_disable()
#define preempt_enable()
#define preempt_count()

#ifdef MACHINE
#include <asm/lock.h>
#endif

typedef volatile int spinlock_t;

#define SPINLOCK_UNLOCKED		(1)
#define DEFINE_SPINLOCK(name)		spinlock_t name = SPINLOCK_UNLOCKED
#define spinlock_init(name)		(name = SPINLOCK_UNLOCKED)

#define spin_lock(count) do { \
	while (count <= 0) ; \
} while (set_atomic((int *)&count, count-1))
#define spin_unlock(count) \
	while (set_atomic((int *)&count, count+1))
#define is_spin_locked(count)		(count <= 0)

#include <io.h>

#define spin_lock_irqsave(lock, f) do { \
	irq_save(f); \
	local_irq_disable(); \
	spin_lock(lock); \
} while (0)
#define spin_unlock_irqrestore(lock, f) do { \
	spin_unlock(lock); \
	irq_restore(f); \
} while (0)

struct semaphore {
	volatile int count;

	/* can't use `waitqueue_head_t` because of circular dependency.
	 * move spinlock_t typedef into types.h? */
	spinlock_t  wait_lock;
	struct list_t wait_list;
};

typedef struct semaphore mutex_t;

#define semaphore_new(name, v) \
	struct semaphore name = { \
		.count = v, \
		.wait_lock = SPINLOCK_UNLOCKED, \
		.wait_list = LIST_HEAD_INIT((name).wait_list), \
	}

#define semaphore_init(name, v)		(name.count = v)

#include <waitqueue.h>
#include <kernel/sched.h>

#define semaphore_down(s) { \
	DEFINE_WAIT(__wait); \
	unsigned __irq_flag; \
	do { \
		while (s.count <= 0) { \
			spin_lock_irqsave(s.wait_lock, __irq_flag); \
			if (list_empty(&__wait.link)) \
				list_add(&__wait.link, s.wait_list.prev); \
			set_task_state(current, TASK_WAITING); \
			spin_unlock_irqrestore(s.wait_lock, __irq_flag); \
			schedule(); \
		} \
	} while (set_atomic((int *)&s.count, s.count-1)); \
}

#define semaphore_up(s) do { \
	struct task_t *__task; \
	unsigned __irq_flag; \
	while (set_atomic((int *)&s.count, s.count+1)) ; \
	if (s.count > 0) { \
		spin_lock_irqsave(s.wait_lock, __irq_flag); \
		if (s.wait_list.next != &s.wait_list) { \
			__task = get_container_of(s.wait_list.next, \
					struct waitqueue_t, link)->task; \
			set_task_state(__task, TASK_RUNNING); \
			runqueue_add(__task); \
			list_del(s.wait_list.next); \
		} \
		spin_unlock_irqrestore(s.wait_lock, __irq_flag); \
	} \
} while (0)

#define DEFINE_MUTEX(name)		semaphore_new(name, 1)
#define mutex_lock(s)			semaphore_down(s)
#define mutex_unlock(s)			semaphore_up(s)

#endif /* __LOCK_H__ */

#include <kernel/softirq.h>
#include <kernel/syscall.h>
#include <error.h>

struct softirq softirq;
struct task *softirqd;

static DEFINE_MUTEX(req_lock);

unsigned int request_softirq(void (*func)())
{
	if (func == NULL)
		return -ERR_PARAM;

	unsigned int i;

	mutex_lock(req_lock);
	for (i = 0; i < SOFTIRQ_MAX; i++) {
		if ((softirq.bitmap & (1 << i)) == 0) {
			softirq.bitmap |= 1 << i;
			softirq.action[i] = func;
			break;
		}
	}
	mutex_unlock(req_lock);

	return i;
}

static inline unsigned int softirq_pending()
{
	unsigned int pending, irqflag;

	spin_lock_irqsave(softirq.wlock, irqflag);
	pending = softirq.pending;
	softirq.pending = 0;
	spin_unlock_irqrestore(softirq.wlock, irqflag);

	return pending;
}

static inline void (**get_actions())()
{
	return softirq.action;
}

static void softirq_handler()
{
	unsigned int pending;
	void (*(*action))();

	while (1) {
		pending = softirq_pending();
		action = get_actions();

		while (pending) {
			if (pending & 1)
				(*action)();

			pending >>= 1;
			action++;
		}

		sys_yield();
	}
}

#include <kernel/init.h>

int __init softirq_init()
{
	softirq.pending = 0;
	softirq.bitmap = 0;
	lock_init(&softirq.wlock);

	if ((softirqd = make(TASK_KERNEL | STACK_SHARED, softirq_handler,
					&init)) == NULL)
		return -ERR_ALLOC;

	set_task_pri(softirqd, HIGH_PRIORITY);

	return 0;
}

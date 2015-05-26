#include <kernel/softirq.h>
#include <kernel/syscall.h>
#include <error.h>

struct softirq_t softirq;
struct task_t *softirqd;

static DEFINE_MUTEX(registration);

unsigned int register_softirq(void (*func)())
{
	if (func == NULL)
		return -ERR_PARAM;

	unsigned int i;

	mutex_lock(registration);
	for (i = 0; i < SOFTIRQ_MAX; i++) {
		if ((softirq.bitmap & (1 << i)) == 0) {
			softirq.bitmap |= 1 << i;
			softirq.call[i] = func;
			break;
		}
	}
	mutex_unlock(registration);

	return i;
}

static void softirq_handler()
{
	unsigned int pending, irqflag;
	void (*(*call))();

	while (1) {
		spin_lock_irqsave(softirq.wlock, irqflag);
		pending = softirq.pending;
		softirq.pending = 0;
		spin_unlock_irqrestore(softirq.wlock, irqflag);

		call = softirq.call;

		while (pending) {
			if (pending & 1)
				(*call)();

			pending >>= 1;
			call++;
		}

		yield();
	}
}

#include <kernel/init.h>

int __init softirq_init()
{
	softirq.pending = 0;
	softirq.bitmap = 0;
	INIT_LOCK(softirq.wlock);

	if ((softirqd = make(TASK_KERNEL, softirq_handler, init.mm.kernel,
					STACK_SHARE)) == NULL)
		return -ERR_ALLOC;

	return 0;
}

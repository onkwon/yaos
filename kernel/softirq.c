/*
 * "[...] Sincerity (comprising truth-to-experience, honesty towards the self,
 * and the capacity for human empathy and compassion) is a quality which
 * resides within the laguage of literature. It isn't a fact or an intention
 * behind the work [...]"
 *
 *             - An introduction to Literary and Cultural Theory, Peter Barry
 *
 *
 *                                                   o8o
 *                                                   `"'
 *     oooo    ooo  .oooo.    .ooooo.   .oooo.o     oooo   .ooooo.
 *      `88.  .8'  `P  )88b  d88' `88b d88(  "8     `888  d88' `88b
 *       `88..8'    .oP"888  888   888 `"Y88b.       888  888   888
 *        `888'    d8(  888  888   888 o.  )88b .o.  888  888   888
 *         .8'     `Y888""8o `Y8bod8P' 8""888P' Y8P o888o `Y8bod8P'
 *     .o..P'
 *     `Y8P'                   Kyunghwan Kwon <kwon@yaos.io>
 *
 *  Welcome aboard!
 */

#include <kernel/softirq.h>
#include <kernel/syscall.h>
#include <error.h>

struct softirq softirq;
struct task *softirqd;

static DEFINE_MUTEX(req_lock);

static inline unsigned int softirq_pending()
{
	unsigned int pending;
	int irqflag;

	disable_irq(irqflag);
	assert(!is_locked(softirq.wlock));
	lock_atomic(&softirq.wlock);

	pending = softirq.pending;
	softirq.pending = 0;

	unlock_atomic(&softirq.wlock);
	enable_irq(irqflag);

	return pending;
}

static inline void *getpool()
{
	return softirq.pool;
}

static void softirq_handler()
{
	unsigned int pending;
	struct __softirq *pool;
#ifdef CONFIG_SOFTIRQ_THREAD
	struct task *thread;
#endif

endless:
	pending = softirq_pending();
	pool = getpool();

	while (pending) {
#ifdef CONFIG_SOFTIRQ_THREAD
		if ((pending & 1) &&
				((thread = make(get_task_flags(current),
						STACK_SIZE_DEFAULT,
						pool->action, current)))) {
			put_arguments(thread, pool->args, NULL, NULL, NULL);
			set_task_pri(thread, pool->priority);
			go_run(thread);
#else
		if (pending & 1) {
			set_task_pri(current, pool->priority);
			pool->action(pool->args);
			set_task_pri(current, RT_HIGHEST_PRIORITY);
#endif
		}

		pending >>= 1;
		pool++;
	}

	sys_yield();

	goto endless;
}

unsigned int request_softirq(void (*func)(), int pri)
{
	if (func == NULL)
		return -EINVAL;

	unsigned int i;

	mutex_lock(&req_lock);
	for (i = 0; i < SOFTIRQ_MAX; i++) {
		if ((softirq.bitmap & (1 << i)) == 0) {
			softirq.bitmap |= 1 << i;
			softirq.pool[i].action = func;
			softirq.pool[i].args = NULL;
			softirq.pool[i].priority = pri;
			break;
		}
	}
	mutex_unlock(&req_lock);

	return i;
}

#include <kernel/init.h>

int __init softirq_init()
{
	int i;

	softirq.pending = 0;
	softirq.bitmap = 0;
	lock_init(&softirq.wlock);

	for (i = 0; i < SOFTIRQ_MAX; i++)
		softirq.pool[i].overrun = 0;

	if ((softirqd = make(TASK_KERNEL | STACK_SHARED, STACK_SIZE_DEFAULT,
					softirq_handler, &init)) == NULL)
		return -ENOMEM;

	softirqd->name = "softirqd";
	set_task_pri(softirqd, RT_HIGHEST_PRIORITY);

	return 0;
}

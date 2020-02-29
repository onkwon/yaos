#include "kernel/sched.h"
#include "kernel/task.h"
#include "kernel/systick.h"
#include "pbrr.h"
#include "syslog.h"

#include <assert.h>

static struct scheduler pbrr;

static inline const uintptr_t *read_watermark(const void * const start,
		const void * const end, uintptr_t sig)
{
	const uintptr_t *p = start;

	while ((uintptr_t)p < (uintptr_t)end) {
		if (*p != sig)
			break;
		p++;
	}

	if (((uintptr_t)p > (uintptr_t)start)
			&& (*--p == sig))
		return p;

	return NULL;
}

static inline void update_runtime(struct task *next, struct task *prev)
{
#if defined(CONFIG_TASK_EXECUTION_TIME)
	uint64_t now = get_systick64_core();

	prev->sum_exec_runtime += now - prev->exec_start;
	next->exec_start = now;
#else
	(void)next;
	(void)prev;
#endif
}

int runqueue_add_core(void *task)
{
	struct task *p = task;

	assert(p && p->addr && p->stack.base && p->stack.limit &&
			p->kstack.base && p->kstack.limit &&
			p->heap.base && p->heap.limit);

	if (p->sched == NULL)
		p->sched = &pbrr;

	return p->sched->add(p->sched, p);
}

/* TODO: implement */
int runqueue_del_core(void *task)
{
	struct task *p = task;
	(void)p;

	return 0;
}

int sched_yield(void)
{
	resched();

	return 0;
}

void schedule(void)
{
#if defined(CONFIG_MEM_WATERMARK)
	current->stack.watermark = read_watermark(current->stack.base,
			current->stack.p, STACK_WATERMARK);
	current->kstack.watermark = read_watermark(current->kstack.base,
			current->kstack.p, STACK_WATERMARK);
	int left = (uintptr_t)current->stack.watermark -
		(uintptr_t)current->stack.base;
	if (left <= 0)
		debug("%s stack left %d", current->name, left);
#endif
	//debug("stack %lx k %lx", (unsigned long)current->stack.p, (unsigned long)current->kstack.p);

	struct task *prev;

	prev = current;

#if defined(CONFIG_REALTIME)
	pbrr_enqueue(&pbrr, current);
	current = pbrr_dequeue(&pbrr);
#endif
	if (!current)
		current = &init_task;

	update_runtime(current, prev);
}

void sched_init(void)
{
	pbrr_init(&pbrr);
}

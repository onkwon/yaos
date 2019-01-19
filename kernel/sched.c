#include "kernel/sched.h"
#include "kernel/task.h"
#include "syslog.h"

static inline uintptr_t *take_watermark(void * const start,
		const void * const end, uintptr_t sig)
{
	uintptr_t *p = start;

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

void schedule(void)
{
#if defined(CONFIG_MEM_WATERMARK)
	current->stack.watermark = take_watermark(current->stack.base,
			current->stack.p, STACK_WATERMARK);
	current->kstack.watermark = take_watermark(current->kstack.base,
			current->kstack.p, STACK_WATERMARK);
	//debug("stack margin left %ld", current->stack.watermark - current->stack.base);
#endif
	//debug("stack %lx k %lx", (unsigned long)current->stack.p, (unsigned long)current->kstack.p);
}

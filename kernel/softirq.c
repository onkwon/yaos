#include <kernel/softirq.h>
#include <error.h>

struct softirq_t softirq_pool;

unsigned int register_softirq(struct task_t *task)
{
	if (task == NULL)
		return -ERR_PARAM;

	unsigned int i;

	for (i = 0; i < SOFTIRQ_MAX; i++) {
		if ((softirq_pool.bitmap & (1 << i)) == 0) {
			softirq_pool.bitmap |= 1 << i;
			break;
		}
	}

	return i;
}

void softirq_init()
{
	softirq_pool.pending = 0;
	softirq_pool.bitmap = 0;
}

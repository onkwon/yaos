#ifndef __TASK_H__
#define __TASK_H__

#define STACK_SIZE_DEFAULT	1024

#define NORMAL_PRIORITY		1
#define REALTIME_PRIORITY	2

#include <types.h>

struct task_t {
	int flags;       	/* priority */
	unsigned long primask;

	unsigned *stack; 	/* stack pointer */
	int stack_size;  	/* stack size */

	void *addr;      	/* address */

	struct list_t rq; 	/* runqueue linked list */
} __attribute__((packed));

#define REGISTER_TASK(func, size, pri) \
	static struct task_t task_##func \
	__attribute__((section(".user_task_list"), used)) = { \
		.flags      = pri, \
		.primask    = 0, \
		.stack      = 0, \
		.stack_size = size, \
		.addr       = func, \
		.rq         = {NULL, NULL}, \
	}

extern struct task_t *current;

#endif /* __TASK_H__ */

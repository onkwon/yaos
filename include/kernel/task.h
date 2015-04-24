#ifndef __TASK_H__
#define __TASK_H__

#define DEFAULT_STACK_SIZE	1024

/* task->state:
 *   31 ... 16 | 15 14 13 12 11 12 10 09 08 | 07 06 05 04 03 02 01 00
 *  -----------|----------------------------|-------------------------
 *   reserved  |            state           |        priorty
 */
#define TASK_STATE_BIT		8
#define TASK_STATE_MASK		0x0000ff00
#define TASK_PRIORITY_MASK	0x000000ff

#define TASK_RUNNING		1
#define TASK_WAITING		2

#define set_task_state(p, s) \
	( ((struct task_t *)p)->state = \
	  (((struct task_t *)p)->state & ~TASK_STATE_MASK) | \
	  (s << TASK_STATE_BIT) )
#define reset_task_state(p, s) \
	( ((struct task_t *)p)->state &= ~(s << TASK_STATE_BIT) )
#define get_task_state(p) \
	( (((struct task_t *)p)->state & TASK_STATE_MASK) >> TASK_STATE_BIT )

/* The lower number, the higher priority.
 *
 *  realtime |  normal
 * ----------|------------
 *  0 ~ 100  | 101 ~ 120
 *                 `-----> 110 = default priority
 */
#define RT_LEAST_PRIORITY	100
#define LEAST_PRIORITY		(RT_LEAST_PRIORITY + 20)
#define DEFAULT_PRIORITY	(RT_LEAST_PRIORITY + 10)

#define GET_PRIORITY(p)		(((struct task_t *)p)->state & TASK_PRIORITY_MASK)
#define IS_TASK_REALTIME(p)	(GET_PRIORITY(p) <= RT_LEAST_PRIORITY)

#include <types.h>

struct sched_entity {
	uint64_t vruntime;
	uint64_t exec_start;
	uint64_t sum_exec_runtime;
} __attribute__((packed));

struct task_t {
	int state;       	/* priority */
	unsigned long primask;

	unsigned *stack; 	/* stack end, the lowest address */
	unsigned *sp; 		/* stack pointer */
	int stack_size;  	/* stack size */

	void *addr;      	/* address */

	struct list_t rq; 	/* runqueue linked list */

	struct sched_entity se;
} __attribute__((packed));

#define REGISTER_TASK(func, size, pri) \
	static struct task_t task_##func \
	__attribute__((section(".user_task_list"), used)) = { \
		.state      = pri, \
		.primask    = 0, \
		.stack      = 0, \
		.sp         = 0, \
		.stack_size = size, \
		.addr       = func, \
		.rq         = {NULL, NULL}, \
		.se         = {0, 0, 0}, \
	}

extern struct task_t *current;

#endif /* __TASK_H__ */

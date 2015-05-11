#ifndef __TASK_H__
#define __TASK_H__

#define DEFAULT_STACK_SIZE	1024

/* task->state:
 *   31 ... 16 | 15 14 13 12 11 12 10 09 08 | 07 06 05 04 03 02 01 00
 *  -----------|----------------------------|-------------------------
 *   reserved  |          priority          |         state
 */
#define TASK_RUNNING		1
#define TASK_WAITING		2
#define TASK_KERNEL		4

#define TASK_STATE_BIT		0
#define TASK_PRIORITY_BIT	8
#define TASK_RSVD_BIT		16
#define TASK_STATE_MASK		((1 << TASK_PRIORITY_BIT) - 1)
#define TASK_PRIORITY_MASK	(((1 << TASK_RSVD_BIT) - 1) & ~TASK_STATE_MASK)

#define set_task_state(p, s)	\
	( ((struct task_t *)(p))->state = \
	  (((struct task_t *)(p))->state & ~TASK_STATE_MASK) | \
	  ((s) << TASK_STATE_BIT) )
#define reset_task_state(p, s)	\
	( ((struct task_t *)(p))->state &= ~((s) << TASK_STATE_BIT) )
#define get_task_state(p)	\
	( (((struct task_t *)(p))->state & TASK_STATE_MASK) >> TASK_STATE_BIT )

/* The lower number, the higher priority.
 *
 *  realtime |  normal
 * ----------|----------
 *   0 ~ 10  | 11 ~ 255
 *                `-----> 132 = default priority
 */
#define RT_LEAST_PRIORITY	10
#define LEAST_PRIORITY		(RT_LEAST_PRIORITY + 245)
#define DEFAULT_PRIORITY	(RT_LEAST_PRIORITY + 122)

#define SET_PRIORITY(pri)	((pri) << TASK_PRIORITY_BIT)
#define set_task_priority(p, s)	\
	( ((struct task_t *)(p))->state = \
	  (((struct task_t *)(p))->state & ~TASK_PRIORITY_MASK) | \
	  SET_PRIORITY(s) )
#define get_task_priority(p)	\
	( (((struct task_t *)(p))->state & TASK_PRIORITY_MASK) >> \
	  TASK_PRIORITY_BIT )
#define is_task_realtime(p)	(get_task_priority(p) <= RT_LEAST_PRIORITY)

#include <types.h>

struct sched_entity {
	uint64_t vruntime;
	uint64_t exec_start;
	uint64_t sum_exec_runtime;
} __attribute__((packed));

struct task_t {
	/* `state` must be the first element in the structure, being located at
	 * the start address as it is used for sanity check in initialization */
	long state;       	/* include priority */
	unsigned long primask;

	unsigned long *stack; 	/* stack end, the lowest address */
	unsigned long *sp;	/* stack pointer */
	long stack_size;  	/* stack size */

	void *addr;      	/* address */

	struct list_t rq; 	/* runqueue linked list */

	struct sched_entity se;
} __attribute__((packed));

#define REGISTER_TASK(func, size, pri) \
	static struct task_t task_##func \
	__attribute__((section(".user_task_list"), used)) = { \
		.state      = SET_PRIORITY(pri), \
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

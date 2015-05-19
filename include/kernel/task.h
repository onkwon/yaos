#ifndef __TASK_H__
#define __TASK_H__

#define DEFAULT_STACK_SIZE	1024 /* bytes */
#define KERNEL_STACK_SIZE	256  /* words = 1024 bytes */

#define get_kernel_stack(addr)	((unsigned long)(addr) & ~(KERNEL_STACK_SIZE-1))

/* task->state:
 *   31 ... 16 | 15 14 13 12 11 12 10 09 08 | 07 06 05 04 03 02 01 00
 *  -----------|----------------------------|-------------------------
 *   reserved  |          priority          |         state
 */
#define TASK_STOPPED		1
#define TASK_RUNNING		2
#define TASK_WAITING		4

#define TASK_KERNEL		8

#define TASK_STATE_BIT		0
#define TASK_PRIORITY_BIT	8
#define TASK_RSVD_BIT		16

#define TASK_STATE_MASK		((1 << TASK_PRIORITY_BIT) - 1)
#define TASK_PRIORITY_MASK	(((1 << TASK_RSVD_BIT) - 1) & ~TASK_STATE_MASK)

#define TASK_STATE_SET(s)	((s) << TASK_STATE_BIT)
/* task type bit TASK_KERNEL must not be concerned with task state transition */
#define set_task_state(p, s)	\
	( ((struct task_t *)(p))->state = \
	  (((struct task_t *)(p))->state & ~(TASK_STATE_MASK & ~TASK_KERNEL)) | \
	  TASK_STATE_SET(s) )
#define reset_task_state(p, s)	\
	( ((struct task_t *)(p))->state &= ~TASK_STATE_SET(s) )
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

#define SCHED_ENTITY_INIT	(struct sched_entity){ 0, 0, 0 }

struct task_t {
	/* `state` must be the first element in the structure, being located at
	 * the start address as it is used for sanity check in initialization */
	long state;       	/* include priority */
	unsigned long primask;

	struct {
		unsigned long *sp;	/* user stack pointer */
		unsigned long *brk;	/* user stack boundary */
		unsigned long size;	/* user stack size */

		/* heap takes place at the bottom of stack memory and expands
		 * up to brk. so "heap size = brk - heap" while "stack top =
		 * heap + size". but be aware that both of heap and brk can be
		 * changed dynamically. */
		unsigned long *heap;

		unsigned long *kernel;	/* kernel stack */
	} stack;

	void *addr;      	/* address */

	struct task_t *parent;
	struct list_t children;
	struct list_t sibling;

	struct list_t rq; 	/* runqueue linked list */

	struct sched_entity se;
} __attribute__((packed));

#define REGISTER_TASK(func, size, pri) \
	static struct task_t task_##func \
	__attribute__((section(".user_task_list"), used)) = { \
		.state    = SET_PRIORITY(pri) | TASK_STATE_SET(TASK_STOPPED), \
		.primask  = 0, \
		.stack    = { NULL, NULL, size, NULL, NULL }, \
		.addr     = func, \
		.parent   = &init, \
		.children = LIST_HEAD_INIT(task_##func.children), \
		.sibling  = LIST_HEAD_INIT(task_##func.sibling), \
		.rq       = LIST_HEAD_INIT(task_##func.rq), \
		.se       = SCHED_ENTITY_INIT, \
	}

#define TASK_INIT(p, address) { \
	set_task_state(&p, TASK_STOPPED); \
	set_task_priority(&p, DEFAULT_PRIORITY); \
	p.stack.size = DEFAULT_STACK_SIZE; \
	alloc_stack(&p); \
	p.addr = address; \
	p.parent = &init; \
	LIST_LINK_INIT(&p.children); \
	LIST_LINK_INIT(&p.sibling); \
	LIST_LINK_INIT(&p.rq); \
	p.se = SCHED_ENTITY_INIT; \
	set_task_context_core(&p); \
}

extern struct task_t *current;
extern struct task_t init;

struct task_t *find_task(unsigned long id, struct task_t *head);
unsigned long *alloc_stack(struct task_t *p);

#endif /* __TASK_H__ */

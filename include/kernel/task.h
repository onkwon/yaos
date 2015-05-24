#ifndef __TASK_H__
#define __TASK_H__

#define STACK_SIZE		1024 /* bytes */
#define KERNEL_STACK_SIZE	1024 /* bytes */
#define HEAP_SIZE		(STACK_SIZE >> 2) /* one fourth out of stack */

#define STACK_SEPARATE		1
#define STACK_SHARE		2

struct mm_t {
	unsigned int *base;
	unsigned int *sp;
	unsigned int *heap;
	/* heap is located in the bottom of stack memory and expands up to brk.
	 * and to obtain stack bottom, "bottom = sp & ~(stack size - 1)" while
	 * "top = bottom + stack size - 1". so "heap size = brk - bottom". but
	 * be aware that both of heap and brk can be changed dynamically. */

	unsigned int *kernel;	/* kernel stack */
};

/* task->state:
 *   31 ... 16 | 15 14 13 12 11 12 10 09 08 | 07 06 05 04 03 02 01 00
 *  -----------|----------------------------|-------------------------
 *   reserved  |          priority          |         state
 */
#define TASK_STOPPED		0x01 /* state */
#define TASK_RUNNING		0x02
#define TASK_WAITING		0x04
#define TASK_KERNEL		0x08 /* type */
#define TASK_USER		0x00
#define TASK_REGISTERED		0x80

#define TASK_STATE_BIT		0
#define TASK_PRIORITY_BIT	8
#define TASK_RSVD_BIT		16

#define TASK_STATE_MASK		((1 << TASK_PRIORITY_BIT) - 1)
#define TASK_PRIORITY_MASK	(((1 << TASK_RSVD_BIT) - 1) & ~TASK_STATE_MASK)

#define TASK_STATE(s)		((s) << TASK_STATE_BIT)
/* task type bit TASK_KERNEL must not be concerned with task state transition */
#define set_task_state(p, s)	\
	( ((struct task_t *)(p))->state = \
	  (((struct task_t *)(p))->state & ~(TASK_STATE_MASK & ~TASK_KERNEL)) | \
	  TASK_STATE(s) )
#define reset_task_state(p, s)	\
	( ((struct task_t *)(p))->state &= ~TASK_STATE(s) )
#define get_task_state(p)	\
	( (((struct task_t *)(p))->state & TASK_STATE_MASK) >> TASK_STATE_BIT )
#define set_task_type(p, s)	set_task_state(p, s)
#define get_task_type(p)	get_task_state(p)

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
#include <kernel/sched.h>

struct task_t {
	/* `state` must be the first element in the structure, being located at
	 * the start address as it is used for sanity check in initialization */
	unsigned int state;	/* inclusive of priority */
	void *addr;		/* address */
	unsigned long irqflag;

	struct mm_t mm;

	struct task_t *parent;
	struct list_t children;
	struct list_t sibling;

	struct list_t rq;	/* runqueue linked list */

	struct sched_entity se;
} __attribute__((packed));

#define REGISTER_TASK(func, type, pri) \
	static struct task_t task_##func \
	__attribute__((section(".user_task_list"), used)) = { \
		.state = TASK_REGISTERED | type | SET_PRIORITY(pri), \
		.addr  = func, \
	}

extern struct task_t *current;
extern struct task_t init;

struct task_t *make(unsigned int flags, void (*func)(), void *ref, int option);
void kill(struct task_t *task);
void set_task_dressed(struct task_t *task, unsigned int flags, void *addr);
int alloc_mm(struct task_t *p, void *ref, int option);

struct task_t *find_task(unsigned long id, struct task_t *head);

#endif /* __TASK_H__ */

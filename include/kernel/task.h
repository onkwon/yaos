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

/* type */
#define TASK_USER		0x00
#define TASK_KERNEL		0x01

#define set_task_type(p, v)	((p)->flags = v)
#define get_task_type(p)	((p)->flags)

/* state */
#define TASK_RUNNING		0x00
#define TASK_STOPPED		0x01
#define TASK_WAITING		0x02
#define TASK_SLEEPING		0x04
#define TASK_REGISTERED		0x80

#define set_task_state(p, s)	((p)->state = s)
#define get_task_state(p)	((p)->state)

/* priority */
/* the lower number, the higher priority.
 *
 *  realtime |  normal
 * ----------|----------
 *   0 ~ 10  | 11 ~ 255
 *                `-----> 132 = default priority
 */
#define RT_LEAST_PRIORITY	10
#define LEAST_PRIORITY		(RT_LEAST_PRIORITY + 245)
#define DEFAULT_PRIORITY	(RT_LEAST_PRIORITY + 122)

#define set_task_pri(p, v)	((p)->pri = v)
#define get_task_pri(p)		((p)->pri)
#define is_realtime(p)		(get_task_pri(p) <= RT_LEAST_PRIORITY)

#include <types.h>
#include <kernel/sched.h>

struct task_t {
	/* `state` must be the first element in the structure as it is used
	 * for sanity check in initialization */
	unsigned int state;
	unsigned int flags;
	unsigned int pri;
	void *addr;
	unsigned int irqflag;

	struct mm_t mm;

	struct task_t *parent;
	struct list_t children;
	struct list_t sibling;

	struct list_t rq;

	struct sched_entity se;
} __attribute__((packed));

#define REGISTER_TASK(f, t, p) \
	static struct task_t task_##func \
	__attribute__((section(".user_task_list"), used)) = { \
		.state = TASK_REGISTERED, \
		.flags = t, \
		.pri   = p, \
		.addr  = f, \
	}

extern struct task_t *current;
extern struct task_t init;

struct task_t *make(unsigned int flags, void (*func)(), void *ref, int option);
void kill(struct task_t *task);
void set_task_dressed(struct task_t *task, unsigned int flags, void *addr);
int alloc_mm(struct task_t *p, void *ref, int option);

struct task_t *find_task(unsigned int id, struct task_t *head);

#endif /* __TASK_H__ */

#ifndef __TASK_H__
#define __TASK_H__

#define STACK_SIZE_DEFAULT		1024
#define STACK_SIZE_MIN			1024

/* TODO: separate heap from stack */
#define HEAP_SIZE_DEFAULT		256

#define STACK_SIZE			1024 /* bytes, kernel stack */
#define USER_SPACE_SIZE			1024 /* bytes */
#define HEAP_SIZE			(USER_SPACE_SIZE >> 2) /* one fourth */
#define USER_STACK_SIZE			(USER_SPACE_SIZE - HEAP_SIZE)

#define STACK_SENTINEL			0xdeafc0de

#include <types.h>
#include <lib/firstfit.h>

struct mm {
	unsigned int *base;
	unsigned int *sp;
	union {
		unsigned int *heap;
		heap_t heaphead;
	};
	/* heap is located in the bottom of stack memory and expands up to brk.
	 * and to obtain stack bottom, "bottom = sp & ~(stack size - 1)" while
	 * "top = bottom + stack size - 1". so "heap size = brk - bottom". but
	 * be aware that both of heap and brk can be changed dynamically. */

	/* kernel stack */
	struct {
		unsigned int *base;
		unsigned int *sp;
	} kernel;
};

/* type & flag */
#define TF_USER				0x00
#define TF_KERNEL			0x01
#define TF_STATIC			0x02
#define TF_SYSCALL			0x04
#define TF_CLONED			0x08
#define TF_HANDLER			0x10
#define TF_PRIVILEGED			0x20
#define TF_SHARED			0x80 /* share stack */

#define TASK_USER			(TF_USER)
#define TASK_KERNEL			(TF_KERNEL | TF_PRIVILEGED)
#define TASK_STATIC			(TF_STATIC)
#define TASK_SYSCALL			(TF_SYSCALL)
#define TASK_CLONED			(TF_CLONED)
#define TASK_PRIVILEGED			(TF_PRIVILEGED)
#define STACK_SHARED			(TF_SHARED)
#define TASK_HANDLER			(TF_HANDLER | TASK_KERNEL)

#define set_task_flags(p, v)		((p)->flags = v)
#define get_task_flags(p)		((p)->flags)
#define set_task_type(p, v)		set_task_flags(p, v)
#define get_task_type(p)		get_task_flags(p)

/* state */
#define TASK_RUNNING			0x00
#define TASK_STOPPED			0x01
#define TASK_WAITING			0x02
#define TASK_SLEEPING			0x04
#define TASK_ZOMBIE			0x08

#define set_task_state(p, s)		((p)->state = s)
#define get_task_state(p)		((p)->state)

/* priority */
/* the lower number, the higher priority.
 *
 *  realtime |  normal
 * ----------|----------
 *   0 ~ 10  | 11 ~ 255
 *                `-----> 132 = default priority
 */
#define RT_PRIORITY			10
#define LOWEST_PRIORITY			(RT_PRIORITY + 245)
#define DEFAULT_PRIORITY		(RT_PRIORITY + 122)
#ifdef CONFIG_REALTIME
#define HIGHEST_PRIORITY		0
#else
#define HIGHEST_PRIORITY		(RT_PRIORITY + 1)
#endif

#define set_task_pri(p, v)		((p)->pri = v)
#define get_task_pri(p)			((p)->pri)
#define is_task_realtime(p)		(get_task_pri(p) <= RT_PRIORITY)

#include <kernel/sched.h>
#include <kernel/lock.h>

struct task {
	/* `state` must be the first element in the structure as it is used
	 * for sanity check at the initialization */
	unsigned int state;
	unsigned int flags;
	unsigned int pri;
	void *addr;

	union {
		unsigned int irqflag;
		size_t size; /* initial stack size */
	};

	struct mm mm;

	struct task *parent;
	struct links children;
	struct links sibling;

	struct links rq;

	struct sched_entity se;

	lock_t lock;

	void *args;
};

#define REGISTER_TASK(func, f, p, s) \
	static struct task task_##func \
	__attribute__((section(".user_task_list"), aligned(4), used)) = { \
		.state = TASK_ZOMBIE, \
		.flags = TASK_STATIC | f, \
		.pri   = p, \
		.addr  = func, \
		.size  = s, \
	}

extern struct task *current;
extern struct task init;

struct task *make(unsigned int flags, size_t size, void *addr, void *ref);
int clone(unsigned int flags, void *ref);
void set_task_dressed(struct task *task, unsigned int flags, void *addr);
int alloc_mm(struct task *new, size_t size, unsigned int flags, void *ref);
void wrapper();
void go_run_atomic(struct task *task);
unsigned int kill_zombie();

struct task *find_task(unsigned int addr, struct task *head);

int sys_kill(unsigned int tid);
int sys_fork();

/* It used to return `(int)(p)`, task address, but it could probably lead
 * problem when the address is higher than 0x80000000 resulting in negative
 * value. We don't really make use of tid at the moment which is just task
 * address. So return just 1. */
#define get_task_tid(p)			1

#endif /* __TASK_H__ */

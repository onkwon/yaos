#ifndef __TASK_H__
#define __TASK_H__

#define STACK_SIZE_DEFAULT		1024 /* bytes */
#define STACK_SIZE_MIN			384 /* bytes */

/* TODO: add the functionality of resizing heap size dynamically */
#define HEAP_SIZE_DEFAULT		128 /* bytes */

#define STACK_SENTINEL			0xdeafc0de

#include <types.h>
#include <lib/firstfit.h>

struct mm {
	/* stack */
	unsigned int *base;
	unsigned int *sp;

	/* heap */
	unsigned int *heap;
	heap_t heaphead;

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
#define TF_ATOMIC			0x40
#define TF_SHARED			0x80 /* share the kernel stack */

#define TASK_USER			(TF_USER)
#define TASK_KERNEL			(TF_KERNEL | TF_PRIVILEGED)
#define TASK_STATIC			(TF_STATIC)
#define TASK_SYSCALL			(TF_SYSCALL)
#define TASK_CLONED			(TF_CLONED)
#define TASK_PRIVILEGED			(TF_PRIVILEGED)
#define STACK_SHARED			(TF_SHARED)
#define TASK_HANDLER			(TF_HANDLER | TASK_KERNEL | TF_ATOMIC)

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
	const char *name;

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
		.name  = #func, \
	}

extern struct task *current;
extern struct task init;

struct task *make(unsigned int flags, size_t size, void *addr, void *ref);
int clone(unsigned int flags, void *ref);
void set_task_dressed(struct task *task, unsigned int flags, void *addr);
int alloc_mm(struct task *new, size_t size, unsigned int flags, void *ref);
void wrapper();
unsigned int kill_zombie();

struct task *find_task(unsigned int addr, struct task *head);

void sys_kill_core(struct task *target, struct task *killer);
int sys_kill(void *task);
int sys_fork();

extern void runqueue_add_core(struct task *new);
extern void runqueue_del_core(struct task *task);
extern void runqueue_add(struct task *new);
extern void runqueue_del(struct task *task);

static inline void go_run_atomic(struct task *task)
{
	if (get_task_state(task)) {
		set_task_state(task, TASK_RUNNING);
		runqueue_add_core(task);
	}
}

static inline void go_run_atomic_if(struct task *task, unsigned int flag)
{
	if (get_task_state(task) == flag) {
		set_task_state(task, TASK_RUNNING);
		runqueue_add_core(task);
	}
}

static inline void go_run(struct task *task)
{
	if (get_task_state(task)) {
		set_task_state(task, TASK_RUNNING);
		runqueue_add(task);
	}
}

static inline void go_run_if(struct task *task, unsigned int flag)
{
	if (get_task_state(task) == flag) {
		set_task_state(task, TASK_RUNNING);
		runqueue_add(task);
	}
}

void syscall_delegate_return(struct task *task, int ret);

static inline void syscall_delegate(struct task *org, struct task *delegate)
{
	set_task_state(org, TASK_WAITING);
	go_run_atomic(delegate);

	extern void resched();
	resched();
}

/* It used to return `(int)(p)`, task address, but it could probably lead
 * problem when the address is higher than 0x80000000 resulting in negative
 * value. We don't really make use of tid at the moment which is just task
 * address. So return just 1. */
#define get_task_tid(p)			1

#endif /* __TASK_H__ */

#ifndef __YAOS_TASK_H__
#define __YAOS_TASK_H__

#include "list.h"
#include "compiler.h"
#include "kernel/lock.h"

#define STACK_ALIGNMENT			8 /* bytes */
#define STACK_SIZE_DEFAULT		2048 /* bytes */
#define STACK_SIZE_MIN			2048 /* bytes */

#define HEAP_SIZE_DEFAULT		128 /* bytes */
#define HEAP_SIZE_MIN			128 /* bytes */

#define STACK_SENTINEL			0xdeadc0deUL
#define STACK_WATERMARK			0x5a5a5a5aUL

/** Task type & flag */
enum {
	TF_USER			= 0x0000,
	TF_KERNEL		= 0x0001,
	TF_STATIC		= 0x0002,
	TF_SYSCALL		= 0x0004,
	TF_CLONED		= 0x0008,
	TF_HANDLER		= 0x0010,
	TF_PRIVILEGED		= 0x0020,
	TF_ATOMIC		= 0x0040,
	TF_SHARED		= 0x0080, /* kernel stack sharing */
	TF_TRANSIT		= 0x0100,

	TASK_USER		= TF_USER,
	TASK_KERNEL		= TF_KERNEL | TF_PRIVILEGED,
	TASK_STATIC		= TF_STATIC,
	TASK_SYSCALL		= TF_SYSCALL,
	TASK_CLONED		= TF_CLONED,
	TASK_PRIVILEGED		= TF_PRIVILEGED,
	TASK_HANDLER		= TF_HANDLER | TASK_KERNEL | TF_ATOMIC,
};

/** Task state */
enum {
	TASK_RUNNING		= 0x00,
	TASK_STOPPED		= 0x01,
	TASK_WAITING		= 0x02,
	TASK_SLEEPING		= 0x04,
	TASK_ZOMBIE		= 0x08,
	TASK_BACKGROUND		= 0x10,
};

enum task_priority {
	TASK_PRIORITY_LOW,
	/* TASK_PRIORITY_NORMAL, */
	/* TASK_PRIORITY_HIGH, */
	TASK_PRIORITY_MAX,
	TASK_PRIORITY_DEFAULT = TASK_PRIORITY_LOW,
};

struct mm {
	const void *base;
	union {
		const void *limit;
		void *p;
	};
#if defined(CONFIG_MEM_WATERMARK)
	const void *watermark;
#endif
};

struct task {
	unsigned long state; /* keep in the first place, used for sanity check
				in the initialization */
	unsigned long flags; /* keep the postion, used in assembly as offset */
	int pri;
	const void *addr;
	const char *name;

	struct mm stack;
	struct mm kstack;
	struct mm heap;

	union {
		uintptr_t irqflag;
		size_t size; /* initial stack size */
	};

	const struct task *parent;
	struct list children;
	struct list sibling;

	struct scheduler *sched;

#if defined(CONFIG_TASK_EXECUTION_TIME)
	uint64_t sum_exec_runtime;
	uint64_t exec_start;
#endif

	lock_t lock;
};

#define set_task_flags(p, v)		((p)->flags = v)
#define get_task_flags(p)		(ACCESS_ONCE((p)->flags))
#define set_task_state(p, s)		((p)->state = s)
#define get_task_state(p)		(ACCESS_ONCE((p)->state))
#define set_task_pri(p, v)		((p)->pri = v)
#define get_task_pri(p)			(ACCESS_ONCE((p)->pri))

#define REGISTER_TASK(func, f, p, s) \
	static struct task task_##func \
	__attribute__((section(".user_task_list"), aligned(4), used)) = { \
		.state = TASK_STOPPED, \
		.flags = TASK_STATIC | (f), \
		.pri   = p, \
		.addr  = func, \
		.size  = s, \
		.name  = #func, \
	}

struct task *current, init_task;

void task_init(void);
void idle_task(void);

#endif /* __YAOS_TASK_H__ */

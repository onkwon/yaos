#ifndef __WAITQUEUE_H__
#define __WAITQUEUE_H__

#include <types.h>

struct waitqueue_head {
	lock_t lock;
	struct links list;
};

#define INIT_WAIT_HEAD(name)	{ UNLOCKED, INIT_LIST_HEAD((name).list) }
#define DEFINE_WAIT_HEAD(name)	\
	struct waitqueue_head name = INIT_WAIT_HEAD(name)
#define WQ_INIT(name)		\
	name = (struct waitqueue_head)INIT_WAIT_HEAD(name)

struct waitqueue {
	struct task *task;
	struct links list;
};

#define WQ_EXCLUSIVE	1
#define WQ_ALL		0x7fffffff

#define DEFINE_WAIT(name) \
	struct waitqueue name = { \
		.task = current, \
		.list = INIT_LIST_HEAD(name.list), \
	}

void wq_wait(struct waitqueue_head *q);
void wq_wake(struct waitqueue_head *q, int nr_task);

#endif /* __WAITQUEUE_H__ */

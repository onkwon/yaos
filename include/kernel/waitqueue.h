#ifndef __WAITQUEUE_H__
#define __WAITQUEUE_H__

#include <types.h>

struct waitqueue_head {
	lock_t lock;
	struct list list;
};

#define DEFINE_WAIT_HEAD(name) \
	struct waitqueue_head name = { \
		.lock = UNLOCKED, \
		.list = INIT_LIST_HEAD(name.list), \
	}

struct waitqueue {
	struct task *task;
	struct list link;
};

#define WQ_EXCLUSIVE	1
#define WQ_ALL		0x7fffffff

#define DEFINE_WAIT(name) \
	struct waitqueue name = { \
		.task = current, \
		.link = INIT_LIST_HEAD(name.link), \
	}

extern inline void wq_wait(struct waitqueue_head *q, struct waitqueue *wait);
extern inline void wq_wake(struct waitqueue_head *head, int nr_task);

#endif /* __WAITQUEUE_H__ */

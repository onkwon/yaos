#ifndef __WAIT_H__
#define __WAIT_H__

#include <types.h>
#include <lock.h>

struct waitqueue_head_t {
	spinlock_t lock;
	struct list_t list;
};

struct waitqueue_t {
	struct task_t *task;
	struct list_t link;
};

#define WQ_EXCLUSIVE	1
#define WQ_ALL		0x7fffffff

#define DEFINE_WAIT(name) \
	struct waitqueue_t name = { \
		.task = current, \
		.link = LIST_HEAD_INIT((name).link), \
	}

extern inline void wait_in(struct waitqueue_head_t *q, struct waitqueue_t *wait);
extern inline void wake_up(struct waitqueue_head_t *head, int nr_task);

#endif /* __WAIT_H__ */

#ifndef __TIMER_H__
#define __TIMER_H__

#include <types.h>

struct timer {
	struct list list; /* keep this first */

	unsigned int expires;
	unsigned int data; /* reference to argument of event() */
	void (*event)(unsigned int data);

	/* security issue can be arise manupulating the element of task
	 * in time between add_timer() and run_timer()
	 * because the data is owned by user while kernel accepts
	 * any address of task without verification */
	struct task *task;
};

struct timer_queue {
	unsigned int nr;
	unsigned int next;
	struct list list;
	lock_t lock;
};

int add_timer(struct timer *new);
int timer_init();

int sys_timer_create(struct timer *new);

void sleep(unsigned int sec);
void msleep(unsigned int ms);

void set_timeout(unsigned int *tv, unsigned int ms);
int is_timeout(unsigned int goal);

#endif /* __TIMER_H__ */

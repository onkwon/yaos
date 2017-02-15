#ifndef __KERNEL_TIMER_H__
#define __KERNEL_TIMER_H__

#include <types.h>

struct ktimer {
	struct links list; /* keep this first */

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
	struct links list;
	lock_t lock;
};

int add_timer(struct ktimer *new);
int timer_init();
unsigned int get_timer_nr();

int sys_timer_create(struct ktimer *new);

void sleep(unsigned int sec);
void msleep(unsigned int ms);

void set_timeout(unsigned int *tv, unsigned int ms);
int is_timeout(unsigned int goal);

void udelay(unsigned int us);
void mdelay(unsigned int ms);

#endif /* __KERNEL_TIMER_H__ */

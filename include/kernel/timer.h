#ifndef __KERNEL_TIMER_H__
#define __KERNEL_TIMER_H__

#include <types.h>
#include <kernel/lock.h>

struct ktimer {
	struct link list; /* keep this first */

	unsigned int expires;
	void *data;
	void (*event)(void *data);

	struct task *task;
};

struct timer_queue {
	unsigned int nr;
	unsigned int next;
	struct link list;
	mutex_t mutex;
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

#ifndef __KERNEL_TIMER_H__
#define __KERNEL_TIMER_H__

#include <types.h>
#include <kernel/lock.h>

struct ktimer {
	struct link list; /* keep this first */
	struct link link; /* link to task's timer list */

	unsigned int expires;
	void *data;
	void (*event)(struct ktimer *timer);

	struct task *task;
};

struct timer_queue {
	unsigned int nr;
	unsigned int next;
	struct link list;
	mutex_t mutex;
};

int add_timer(int ms, void (*func)(struct ktimer *timer));
int __add_timer(struct ktimer *new);
void __del_timer_if_match(struct task *task, void *addr);
unsigned int get_timer_nr();
struct ktimer *get_timer_nearest();

int sys_timer_create(struct ktimer *new);

void sleep(unsigned int sec);
void msleep(unsigned int ms);

void set_timeout(unsigned int *tv, unsigned int tick);
int is_timeout(unsigned int goal);

void udelay(unsigned int us);
void mdelay(unsigned int ms);

#endif /* __KERNEL_TIMER_H__ */

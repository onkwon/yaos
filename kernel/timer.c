#include "kernel/timer.h"
#include "kernel/systick.h"
#include "kernel/task.h"
#include "kernel/sched.h"
#include "kernel/syscall.h"
#include "list.h"
#include "heap.h"
#include "syslog.h"

#include <errno.h>

typedef struct ktimer {
	uint8_t run; /* 0 when it's done or removed otherwise counted down
			each time run. in case of TIMER_MAX_RERUN it reruns
			forever, not counting down */
	uint16_t round;
	uint32_t interval;
	uint32_t expires; /* for latency compensation */
	void (*cb)(void);
	void *task;

	struct list q;
} ktimer_t;

static struct list timers[TIMER_NR_SLOTS];
static int overflow;

static inline int calc_left(uint32_t goal, uint32_t now)
{
	int left;

	if (goal < now) {
		left = (uint32_t)-1 - now + goal;
		//debug("overflow: %d, now %lu, goal %lu", left, now, goal);
	} else {
		left = goal - now;
	}

	return left;
}

static inline int get_slot(int left, uint32_t now)
{
	return (now + left - 1) % TIMER_NR_SLOTS;
}

static inline uint16_t get_round(int left)
{
	return (left - 1) / TIMER_NR_SLOTS;
}

static inline int update_timer(ktimer_t *timer, uint32_t interval_ticks, uint32_t now)
{
	uint32_t goal;
	int next_slot, left;

	goal = now + interval_ticks;
	left = min((uint32_t)calc_left(goal, now), interval_ticks);
	next_slot = get_slot(left, now);

	timer->round = get_round(left);
	timer->interval = interval_ticks;
	timer->expires = goal;

	assert(!(timer->run == 0));
	//debug("interval %lu, now %lu goal %lu left %d", timer->interval, now, timer->expires, left);

	return next_slot;
}

static inline void insert(ktimer_t *timer, int slot)
{
	assert(&timer->q != timers[slot].next);
	// TODO: make it ordered list. it takes O(N) now
	list_add(&timer->q, &timers[slot]);
}

static inline void delete(ktimer_t *timer, void *slot)
{
	// TODO: optimize O(N)
	int res = list_del(&timer->q, slot);
	assert(res == 0);
}

#if defined(TEST)
void timer_handler(uint32_t now)
#else
static void timer_handler(uint32_t now)
#endif
{
	int current_slot = now % TIMER_NR_SLOTS;

	struct list *slot = &timers[current_slot];
	struct list *node = slot->next;

	while (node) {
		ktimer_t *timer = container_of(node, ktimer_t, q);
		int left = calc_left(timer->expires, now);
		uint16_t round = get_round(left);

		node = node->next;

		if (timer->run == 0) {
			debug("delete timer: %p", timer);
			delete(timer, slot);
			free(timer);
			continue;
		} else if (left >= TIMER_NR_SLOTS && round) {
			//return; // TODO: return if ordered
			continue;
		}

		delete(timer, slot);
		/* TODO: remove security vulnerability adjusting task priority */
		timer->cb();

		if (timer->run != TIMER_REPEAT)
			timer->run--;
		if (timer->run) {
			insert(timer, update_timer(timer, timer->interval,
						timer->expires));
		}
	}
}

static void timer_process(void)
{
	while (1) {
		uint32_t now = get_systick();
		int cnt = overflow;

		for (int i = cnt; i; i--) {
			timer_handler(now - i);
		}

		atomic_faa(&overflow, -cnt);

		set_task_state(current, TASK_WAITING);
		yield();
	}
}
REGISTER_TASK(timer_process, TASK_KERNEL | TF_MANUAL, TASK_PRIORITY_HIGHEST,
		STACK_SIZE_DEFAULT);

/* TODO: call only when a timer to run exists, reducing system timer interrupt
 * overhead which results in power saving */
int timer_run(void)
{
	atomic_faa(&overflow, 1);

	if (get_task_state(&task_timer_process) == TASK_RUNNING) {
		/* NOTE: timers registered are not being processed in time
		 * because it exceeds number of timers that cpu power can
		 * handle at a time. do either of reducing workload or
		 * increasing cpu power. slower systick or higher cpu clock
		 * would help with it. */
		return -EAGAIN;
	}

	set_task_state(&task_timer_process, TASK_RUNNING);
	runqueue_add(&task_timer_process);

	return 0;
}

/**
 * @return timer ID, a memory pointer to allocated timer in fact
*/
int timer_create_core(uint32_t interval_ticks, void (*cb)(void), uint8_t run)
{
	ktimer_t *new;

	if (interval_ticks == 0 || run == 0 || !cb)
		return -EINVAL;

	if ((new = malloc(sizeof(*new))) == NULL)
		return -ENOMEM;

	int next_slot;

	new->run = run;
	new->cb = cb;
	next_slot = update_timer(new, interval_ticks, get_systick());
	new->task = current;
	list_init(&new->q);

	insert(new, next_slot);

	return (int)new;
}

/* TODO: implement */
int timer_delete_core(int timerid)
{
	return timerid;
}

/* TODO: implement */
uint32_t timer_nearest(void)
{
	return 0;
}

void timer_init(void)
{
	for (unsigned int i = 0; i < TIMER_NR_SLOTS; i++) {
		list_init(&timers[i]);
	}
}

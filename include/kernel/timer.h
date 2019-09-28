#ifndef __YAOS_TIMER_H__
#define __YAOS_TIMER_H__

#include "list.h"
#include <stdint.h>

#define TIMER_NR_SLOTS				10
#define TIMER_MAX_RERUN				0xffU

typedef struct ktimer {
	uint8_t run; /* 0 when it's done or removed otherwise counted down
			each time run. in case of TIMER_MAX_RERUN it reruns
			forever, not counting down */
	uint16_t round;
	uint32_t interval;
	uint32_t goal; /* for latency compensation */
	void (*cb)(void);
	void *task;

	struct list q;
} ktimer_t;

int timer_create_core(uint32_t interval_ticks, void (*cb)(void), uint8_t run);
int timer_delete_core(int timerid);
int timer_run(void);
void timer_init(void);

#endif /* __YAOS_TIMER_H__ */

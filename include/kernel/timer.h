#ifndef __YAOS_TIMER_H__
#define __YAOS_TIMER_H__

#include <stdint.h>

#define TIMER_NR_SLOTS				10
#define TIMER_REPEAT				0xffU
#define TIMER_MAX_RERUN				(TIMER_REPEAT - 1)

int timer_create_core(uint32_t interval_ticks, void (*cb)(void), uint8_t run);
int timer_delete_core(int timerid);
int timer_run(void);
void timer_init(void);

#if defined(TEST)
void timer_handler(uint32_t now);
#endif

#endif /* __YAOS_TIMER_H__ */

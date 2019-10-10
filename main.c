#include "syslog.h"

#include "drivers/gpio.h"
#include "arch/mach/board/pinmap.h"
#include "kernel/systick.h"
#include "kernel/timer.h"

#define BTN_HOLD_MIN_MSEC		100
#define BTN_HOLD_LONG_MSEC		5000

#define PIN_TEST			PIN_DEBUG

#include <stdlib.h>
static void gpio_interrupt_callback(const int pin)
{
	(void)pin;

	static bool pin_level_saved = true;
	static unsigned long stamp_saved;
	static unsigned long high, low;

	unsigned long stamp = TICKS_TO_MSEC(get_systick());

	if (gpio_get(/*pin*/PIN_TEST))
		high++;
	else
		low++;

	if (abs(stamp - stamp_saved) < BTN_HOLD_MIN_MSEC) /* debouncing */
		return;

	bool pin_level = high > low;
	debug("H %lu L %lu", high, low);

	if (pin_level_saved && !pin_level) { /* pressed */
		debug("P[%d] stamp %lu, saved %lu = %ld", pin_level,
				stamp, stamp_saved, stamp - stamp_saved);
		//debug("Button pressed %lu %ld", stamp, stamp - stamp_saved);
	} else { /* relesed */
		unsigned long elapsed = stamp - stamp_saved;
		if (elapsed >= BTN_HOLD_LONG_MSEC) {
		} else {
		}
		//debug("Button released %lu %lu %ld", stamp, elapsed, stamp - stamp_saved);
		debug("R[%d] stamp %lu, saved %lu = %ld", pin_level,
				stamp, stamp_saved, stamp - stamp_saved);
	}

	stamp_saved = stamp;
	pin_level_saved = pin_level;
	high = low = 0;
}

int main(void)
{
	debug("bare metal example");

	gpio_init(PIN_STATUS_LED, GPIO_MODE_OUTPUT, NULL);
	gpio_init(PIN_DEBUG,
			GPIO_MODE_INPUT | GPIO_CONF_PULLUP | GPIO_INT_FALLING,
			gpio_interrupt_callback);

	int led_state = 0;

	while (1) {
		gpio_put(PIN_STATUS_LED, led_state & 1);
		led_state++;
		debug("%d", led_state);

		msleep(500);
	}
}
#if defined(CONFIG_SCHEDULER)
#include "kernel/task.h"
REGISTER_TASK(main, TASK_KERNEL, TASK_PRIORITY_NORMAL, STACK_SIZE_DEFAULT);
#endif

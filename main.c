#include "syslog.h"

#include "drivers/gpio.h"
#include "arch/mach/board/pinmap.h"
#include "kernel/syscall.h"
#include "kernel/systick.h"
#include "kernel/timer.h"

#define BTN_HOLD_MIN_MSEC		100
#define BTN_HOLD_LONG_MSEC		5000

#define PIN_TEST			PIN_DEBUG

#define SOFTWARE_DEBOUNCING

#if defined(SOFTWARE_DEBOUNCING)
#define POLLING_PERIOD_MSEC		5

static bool button_poll(void)
{
	static uint16_t state = 0;
	bool pin_level;

	pin_level = gpio_get(/*pin*/PIN_TEST);

	state = (uint16_t)((state << 1) | !pin_level | 0xe000);

	if (state == 0xf000)
		return true;

	return false;
}

static void button_timer(void *arg)
{
	if (button_poll())
		debug("CHANGED");

	(void)arg;
}
#else
static void gpio_interrupt_callback(const int pin)
{
	(void)pin;

	static bool in_transition;
	static unsigned long stamp_saved;
	unsigned long stamp, elapsed;
	bool pin_level;

	stamp = TICKS_TO_MSEC(get_systick());
	pin_level = gpio_get(/*pin*/PIN_TEST);

	if (stamp < stamp_saved) { /* overflow */
		elapsed = (unsigned long)-1 - stamp_saved + stamp;
	} else {
		elapsed = stamp - stamp_saved;
	}

	if (in_transition && elapsed < BTN_HOLD_MIN_MSEC) /* debouncing */
		return;

	in_transition ^= true;
	stamp_saved = stamp;

	debug("pin level changed to %d", pin_level);
}
#endif

int main(void)
{
	debug("bare metal example");

	gpio_init(PIN_STATUS_LED, GPIO_MODE_OUTPUT, NULL);
#if defined(SOFTWARE_DEBOUNCING)
	gpio_init(PIN_DEBUG, GPIO_MODE_INPUT | GPIO_CONF_PULLUP, NULL);
	timer_create(MSEC_TO_TICKS(POLLING_PERIOD_MSEC), button_timer, TIMER_REPEAT);
#else
	gpio_init(PIN_DEBUG,
			GPIO_MODE_INPUT | GPIO_CONF_PULLUP | GPIO_INT_FALLING,
			gpio_interrupt_callback);
#endif

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

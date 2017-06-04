#include <kernel/module.h>
#include <pinmap.h>

REGISTER_DEVICE(uart, "uart", 6);
REGISTER_DEVICE(gpio, "led", PIN_LED_GREEN);
REGISTER_DEVICE(gpio, "led", PIN_LED_ORANGE);
REGISTER_DEVICE(gpio, "led", PIN_LED_RED);
REGISTER_DEVICE(gpio, "led", PIN_LED_BLUE);

void clock_init()
{
	extern void SystemClock_Config(void);
	SystemClock_Config();
}

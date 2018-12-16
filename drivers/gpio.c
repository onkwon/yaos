#include "drivers/gpio.h"

#if 0
static bitmap_t gpio_state;
isr dictionary;

static void __attribute__((weak)) ISR_gpio(int nvector)
{
	uint16_t pin;

#ifndef CONFIG_COMMON_IRQ_FRAMEWORK
	nvector = get_active_irq();
#endif

	pin = hw_gpio_event_source_get(nvector);
	hw_gpio_event_clear(pin);
}
#endif

int gpio_init(const uint16_t npin, const uint32_t flags)
{
	return __gpio_init(npin, flags);
}

void gpio_fini(const uint16_t npin)
{
	__gpio_fini(npin);
}

void gpio_put(const uint16_t npin, const int val)
{
	__gpio_put(npin, val);
}

uint16_t gpio_get(const uint16_t npin)
{
	return __gpio_get(npin);
}

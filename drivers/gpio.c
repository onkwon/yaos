#include "drivers/gpio.h"

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


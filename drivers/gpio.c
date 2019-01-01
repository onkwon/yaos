#include "drivers/gpio.h"
#include "bitmap.h"
#include "dict.h"
#include "assert.h"
#include "syslog.h"
#include "kernel/interrupt.h"
#include "kernel/lock.h"
#include <stdbool.h>

#define SLOT_MAX			((GPIO_MAX) / 16U) // 6.25% of GPIO_MAX

static DEFINE_BITMAP(_gpiomap, GPIO_MAX);
static DEFINE_DICTIONARY_TABLE(_cb_dict, SLOT_MAX);
static DEFINE_DICTIONARY(_cb_dict, SLOT_MAX); // isr callback dictionary for each gpio
static DEFINE_LOCK(_cb_dict_lock);

static void ISR_gpio(int vector)
{
	void (*f)(const int pin) = NULL;
	uintptr_t addr;
	uint16_t pin;

	vector = get_active_irq_from_isr(vector);
	pin = hw_gpio_get_event_source(vector);
	assert(pin < GPIO_MAX);

#if 0 // TODO: Find out which port involved to the event on stm32
	if (!bitmap_get(_gpiomap, pin)) {
		syslog("WARN: gpio(%u) not initialized", pin);
		goto out;
	}
#endif

	spin_lock_isr(&_cb_dict_lock);
	int t = idict_get(&_cb_dict, pin, &addr);
	spin_unlock_isr(&_cb_dict_lock);

	if (0 != t) {
		syslog("WARN: no isr registered for gpio(%u)", pin);
		goto out;
	}

	if ((f = (void (*)(int))addr))
		f(pin);
out:
	hw_gpio_clear_event(pin);
}

int gpio_init(const uint16_t pin, const uint32_t flags, void (*f)(const int))
{
	uintptr_t irqflag;
	int rc = -EEXIST;

	if (bitmap_get(_gpiomap, pin))
		goto out;

	if (0 > (rc = hw_gpio_init(pin, flags))) // error
		goto out;

	if (0 < rc) { // interrupt enabled
		spin_lock_irqsave(&_cb_dict_lock, &irqflag);
		if (0 == idict_add(&_cb_dict, rc - 1, (uintptr_t)f))
			rc = 0;
		spin_unlock_irqrestore(&_cb_dict_lock, irqflag);
	}

	bitmap_set(_gpiomap, pin);
out:
	return rc;
}

void gpio_fini(const uint16_t pin)
{
	uintptr_t irqflag;

	hw_gpio_fini(pin);

	spin_lock_irqsave(&_cb_dict_lock, &irqflag);
	idict_del(&_cb_dict, pin);
	spin_unlock_irqrestore(&_cb_dict_lock, irqflag);

	bitmap_clear(_gpiomap, pin);
}

void gpio_put(const uint16_t pin, const int val)
{
	if (bitmap_get(_gpiomap, pin))
		hw_gpio_put(pin, val);
}

int gpio_get(const uint16_t pin)
{
	int rc = -ENOENT;

	if (bitmap_get(_gpiomap, pin))
		rc = hw_gpio_get(pin);

	return rc;
}

#include "kernel/init.h"

static void __init gpio_driver_init(void)
{
	hw_gpio_driver_init(ISR_gpio);
}
REGISTER_INIT(gpio_driver_init, 10);

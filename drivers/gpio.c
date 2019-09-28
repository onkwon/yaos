#include "drivers/gpio.h"

#include "bitmap.h"
#include "dict.h"
#include "syslog.h"

#include "kernel/interrupt.h"
#include "kernel/lock.h"

#include <stdbool.h>
#include <assert.h>

#define SLOT_MAX			((GPIO_MAX) / 16U) // 6.25% of GPIO_MAX

static DEFINE_BITMAP(gpiomap, GPIO_MAX);
static DEFINE_DICTIONARY_TABLE(cb_dict, SLOT_MAX);
static DEFINE_DICTIONARY(cb_dict, SLOT_MAX); // isr callback dictionary for each gpio
static DEFINE_LOCK(cb_dict_lock);

static void ISR_gpio(int vector)
{
	void (*cb)(const int pin) = NULL;
	uintptr_t addr;
	uint16_t pin;

	vector = get_active_irq_from_isr(vector);
	pin = hw_gpio_get_event_source(vector);
	assert(pin < GPIO_MAX);

#if 0 // TODO: Find out which port involved to the event on stm32
	if (!bitmap_get(gpiomap, pin)) {
		warn("gpio(%u) not initialized", pin);
		goto out;
	}
#endif

	spin_lock_critical(&cb_dict_lock);
	int t = dict_get(&cb_dict, pin, &addr);
	spin_unlock_critical(&cb_dict_lock);

	if (t != 0) {
		warn("no isr registered for gpio(%u)", pin);
		goto out;
	}

	if ((cb = (void (*)(int))addr))
		cb(pin);
out:
	hw_gpio_clear_event(pin);
}

int gpio_init(const uint16_t pin, const uint32_t flags, void (*cb)(const int))
{
	int rc = -EEXIST;

	if (bitmap_get(gpiomap, pin))
		goto out;

	if ((rc = hw_gpio_init(pin, flags)) < 0) // error
		goto out;

	if (rc > 0) { // interrupt enabled
		spin_lock_irqsave(&cb_dict_lock);
		if (dict_add(&cb_dict, rc - 1, (uintptr_t)cb) == 0)
			rc = 0;
		spin_unlock_irqrestore(&cb_dict_lock);
	}

	bitmap_set(gpiomap, pin);
out:
	return rc;
}

void gpio_fini(const uint16_t pin)
{
	if (!bitmap_get(gpiomap, pin))
		return;

	hw_gpio_fini(pin);

	spin_lock_irqsave(&cb_dict_lock);
	dict_del(&cb_dict, pin);
	spin_unlock_irqrestore(&cb_dict_lock);

	bitmap_clear(gpiomap, pin);
}

void gpio_put(const uint16_t pin, const int val)
{
	if (bitmap_get(gpiomap, pin))
		hw_gpio_put(pin, val);
}

int gpio_get(const uint16_t pin)
{
	int rc = -ENOENT;

	if (bitmap_get(gpiomap, pin))
		rc = hw_gpio_get(pin);

	return rc;
}

#include "kernel/init.h"

static void __init gpio_driver_init(void)
{
	hw_gpio_driver_init(ISR_gpio);
}
REGISTER_INIT(gpio_driver_init, 10);

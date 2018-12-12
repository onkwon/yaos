#ifndef __YAOS_GPIO_H__
#define __YAOS_GPIO_H__

#include <stdint.h>

/** GPIO initialization options. Each options get combined by OR. For example
 * to initialize a gpio as an input with interrupt enabled at falling edge and
 * pull-up `gpio_init(pin_number, GPIO_MODE_INPUT | GPIO_CONF_PULLUP |
 * GPIO_INT_FALLING)`. */
enum gpio_mode {
	/** Input mode */
	GPIO_MODE_INPUT		= 0x0000,
	/** Output mode */
	GPIO_MODE_OUTPUT	= 0x0001,
	/** Alternative function mode */
	GPIO_MODE_ALT		= 0x0002,
	/** Analog mode */
	GPIO_MODE_ANALOG	= 0x0004,
	/** Mode mask */
	GPIO_MODE_MASK		= 0x0007,

	/** Pull-up */
	GPIO_CONF_PULLUP	= 0x0008,
	/** Pull-down */
	GPIO_CONF_PULLDOWN	= 0x0010,
	/** Opendrain */
	GPIO_CONF_OPENDRAIN	= 0x0020,
	/** Conf mask */
	GPIO_CONF_MASK		= 0x0038,

	/** Falling edge detected interrupt */
	GPIO_INT_FALLING	= 0x0100,
	/** Rising edge detected interrupt */
	GPIO_INT_RISING		= 0x0200,
	/** Level high detected interrupt */
	GPIO_INT_HIGH		= 0x0400,
	/** Level low detected interrupt */
	GPIO_INT_LOW		= 0x0800,
	/** Interrupt mask */
	GPIO_INT_MASK		= 0x0f00,

	/** Slow */
	GPIO_SPD_SLOW		= 0x1000,
	/** Mid */
	GPIO_SPD_MID		= 0x2000,
	/** Fast */
	GPIO_SPD_FAST		= 0x3000,
	/** Fastest */
	GPIO_SPD_FASTEST	= 0x4000,
	/** Speed mask */
	GPIO_SPD_MASK		= 0xf000,
};

#define GPIO_ALT_SHIFT			16

/** Select a gpio alternative function. :c:data:`n` must be `uint32_t` */
#define gpio_altfunc(n)			\
	(GPIO_MODE_ALT | ((n) << GPIO_ALT_SHIFT))
#define gpio_altfunc_get(flags)		((flags) >> GPIO_ALT_SHIFT)

/**
 * Initialize a gpio
 *
 * @param npin Number of pin
 * @param flags Refer to :c:type:`enum gpio_mode`
 * @return Logical interrupt vector number
 */
int gpio_init(const uint16_t npin, const uint32_t flags);
/**
 * Deinitialize a gpio
 *
 * @param npin Number of pin
 */
void gpio_fini(const uint16_t npin);
/**
 * Write logical value to GPIO output pin
 *
 * @param npin Number of pin
 * @param val Logical value to write
 */
void gpio_put(const uint16_t npin, const int val);
/**
 * Read logical value of GPIO input pin
 *
 * @param npin Number of pin
 * @return Current level of GPIO
 */
uint16_t gpio_get(const uint16_t npin);

#include "arch/mach/gpio.h"

#endif /* __YAOS_GPIO_H__ */

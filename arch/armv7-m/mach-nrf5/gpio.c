#include <gpio.h>

#include "nrf52.h"
#include "nrf_gpio.h"
#include <drivers/gpio.h>
#include <asm/interrupt.h>

static struct gpio {
	union {
		struct {
			unsigned int pin0: 1;
			unsigned int pin1: 1;
			unsigned int pin2: 1;
			unsigned int pin3: 1;
			unsigned int pin4: 1;
			unsigned int pin5: 1;
			unsigned int pin6: 1;
			unsigned int pin7: 1;
			unsigned int pin8: 1;
			unsigned int pin9: 1;
			unsigned int pin10: 1;
			unsigned int pin11: 1;
			unsigned int pin12: 1;
			unsigned int pin13: 1;
			unsigned int pin14: 1;
			unsigned int pin15: 1;
		};

		unsigned int pins;
	};
} state[NR_PORT];

static int nr_active; /* number of active pins */

void ret_from_gpio_int(unsigned int n)
{
	NRF_GPIOTE->EVENTS_PORT = 0;

#if 0
	int i;
	for (i = 0; i < 8; i++)
		NRF_GPIOTE->EVENTS_IN[i] = 0;
#endif
}

unsigned int gpio_get(unsigned int npin)
{
	return nrf_gpio_pin_out_read(npin);
	nrf_gpio_pin_read(npin);
	nrf_gpio_pin_sense_get(npin);
}

void gpio_put(unsigned int npin, int v)
{
	if (v)
		nrf_gpio_pin_set(npin);
	else
		nrf_gpio_pin_clear(npin);
}

int gpio_init(unsigned int pin, unsigned int flags)
{
	nrf_gpio_pin_pull_t conf;
	nrf_gpio_pin_sense_t intr;
	int vector = 0;

	if (state[0].pins & (1 << pin)) {
		error("already taken: %d", pin);
		return EEXIST;
	}

	if (flags & GPIO_MODE_OUTPUT) {
		nrf_gpio_cfg_output(pin);
	} else {
		switch (flags & GPIO_CONF_MASK) {
		case GPIO_CONF_PULLUP:
			conf = NRF_GPIO_PIN_PULLUP;
			break;
		case GPIO_CONF_PULLDOWN:
			conf = NRF_GPIO_PIN_PULLDOWN;
			break;
		default:
			conf = NRF_GPIO_PIN_NOPULL;
			break;
		}

		switch (flags & GPIO_INT_MASK) {
		case GPIO_INT_HIGH:
		case GPIO_INT_RISING:
			intr = NRF_GPIO_PIN_SENSE_HIGH;
			break;
		case GPIO_INT_LOW:
		case GPIO_INT_FALLING:
			intr = NRF_GPIO_PIN_SENSE_LOW;
			break;
		default:
			intr = NRF_GPIO_PIN_NOSENSE;
			break;
		}

		nrf_gpio_cfg_sense_input(pin, conf, intr);

		if (intr != NRF_GPIO_PIN_NOSENSE) {
			vector = NVECTOR_IRQ + 6; /* GPIOTE */
			nvic_set(vec2irq(vector), ON);

			NRF_GPIOTE->EVENTS_PORT = 0;
			NRF_GPIOTE->INTENSET = 1 << 31; /* PORT event */
		}
	}

	state[0].pins |= 1 << pin;
	nr_active++;

	return vector;
}

void gpio_reset(unsigned int pin)
{
}

unsigned int get_gpio_state(int port)
{
	if (port >= NR_PORT) {
		error("not supported port: %d", port);
		return -1;
	}

	return state[port].pins;
}

#include "nrf_delay.h"
void __nrf_delay_ms(int ms)
{
	nrf_delay_ms(ms);
}

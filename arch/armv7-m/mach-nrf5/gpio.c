#include <gpio.h>

#include "nrf52.h"
#include "nrf_gpio.h"
#include <drivers/gpio.h>
#include <asm/interrupt.h>

static struct gpio {
	unsigned int pins;
} state[NR_PORT];

static int nr_active; /* number of active pins */

static void (*isr_table[PINS_PER_PORT])(int nvector);

static int irq_register(int lvector, void (*handler)(int))
{
	int pin = get_secondary_vector(lvector);

	if (pin >= PINS_PER_PORT)
		return -ERANGE;

	/* NOTE: maybe you can make handler list so that multiple user handlers
	 * get called. we call only one here however. */
	isr_table[pin] = handler;

	return 0;
}

static void ISR_gpio(int nvector)
{
	unsigned int conf, sens, scan;
	int i;

#ifndef CONFIG_COMMON_IRQ_FRAMEWORK
	nvector = get_active_irq();
#endif

#ifdef NRF5_SUPPORT_GPIOTE
	if (!NRF_GPIOTE->EVENTS_PORT)
		goto event_in;
#endif
	scan = NRF_P0->IN;

	for (i = 0; i < PINS_PER_PORT; i++) {
		conf = NRF_P0->PIN_CNF[i];
		sens = (conf & (3 << 16)) >> 16;

		if (conf & (1 << 31)) /* if output */
			continue;
		if (!sens) /* if no sense */
			continue;
		if ((scan & (1 << i)) != ((3 - sens) << i)) /* if not match level */
			continue;

		break;
	}

	if (i < PINS_PER_PORT && isr_table[i])
		isr_table[i](nvector);

	NRF_GPIOTE->EVENTS_PORT = 0;
	return;

#ifdef NRF5_SUPPORT_GPIOTE
event_in:
	intr = NRF_GPIOTE->INTENSET;

	for (i = 0; i < 8; i++) {
		if (!(intr & (1 << i))) /* if not enabled */
			continue;
	}

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
		return -EEXIST;
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
			nvic_enable(vector, true);
			vector = mkvector(vector, pin);

			NRF_GPIOTE->EVENTS_PORT = 0;
			NRF_GPIOTE->INTENSET = 1 << 31; /* PORT event */
		}
	}

	state[0].pins |= 1 << pin;
	nr_active++;

	return vector;
}

void gpio_fini(unsigned int pin)
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

#include <kernel/init.h>

static void __init gpio_irq_init()
{
	int i;

	for (i = 0; i < PINS_PER_PORT; i++)
		isr_table[i] = NULL;

	register_isr(NVECTOR_IRQ + 6, ISR_gpio);
	register_isr_register(NVECTOR_IRQ + 6, irq_register, 0);
}
REGISTER_INIT(gpio_irq_init, 10);

#include "nrf_delay.h"
void __nrf_delay_ms(int ms)
{
	nrf_delay_ms(ms);
}

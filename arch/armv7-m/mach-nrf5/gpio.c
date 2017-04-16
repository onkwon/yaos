#include <gpio.h>

#include "nrf52.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include <kernel/gpio.h>
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

int reg2port(reg_t *reg)
{
	return 0;
}

void set_port_pin_conf(reg_t *reg, int pin, int mode)
{
}

void set_port_pin_conf_alt(reg_t *reg, int pin, int mode)
{
}

unsigned int gpio_get(unsigned int npin)
{
	//nrf_gpio_pin_read(npin);
	return nrf_gpio_pin_out_read(npin);
}

void gpio_put(unsigned int npin, int v)
{
	if (v)
		nrf_gpio_pin_set(npin);
	else
		nrf_gpio_pin_clear(npin);
}

int gpio_init(unsigned int npin, unsigned int flags)
{
	int vector;

	vector = NVECTOR_IRQ + 6; /* GPIOTE */

	if (flags & GPIO_MODE_OUTPUT) {
		nrf_gpio_cfg_output(npin);
	} else {
#if 0
		nrf_gpio_cfg_sense_input(uint32_t pin_number,
   	             nrf_gpio_pin_pull_t  pull_config,
		     nrf_gpio_pin_sense_t sense_config)
#endif
	}

	return vector;
}

void gpio_reset(unsigned int npin)
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

void __nrf_delay_ms(int ms)
{
	nrf_delay_ms(ms);
}

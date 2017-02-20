#include <gpio.h>
#include <types.h>
#include <error.h>
#include "exti.h"
#include "io.h"

DEFINE_SPINLOCK(gpio_irq_lock);
static DEFINE_SPINLOCK(gpio_init_lock);

#define pin2port(pin)		((pin) / PINS_PER_PORT)
#define pin2portpin(pin)	((pin) % PINS_PER_PORT)
#define port2reg(port)		((reg_t *)((((port) * WORD_SIZE) << 8) + PORTA))

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
	switch ((unsigned int)reg) {
	case PORTA: return 0;
	case PORTB: return 1;
	case PORTC: return 2;
	case PORTD: return 3;
	case PORTE: return 4;
	case PORTF: return 5;
	case PORTG: return 6;
	default:
		break;
	}

	return -ERR_RANGE;
}

void set_port_pin_conf(reg_t *reg, int pin, int mode)
{
	unsigned int idx, t, shift, mask;

#if (SOC == stm32f1)
	idx = pin / 8;
	shift = (pin % 8) * 4;
	mask = 0xf;
#else
	idx = pin / 16;
	shift = (pin % 16) * 2;
	mask = 0x3;
#endif

	t = reg[idx];
	t = MASK_RESET(t, mask << shift) | (mode << shift);
	reg[idx] = t;
}

void set_port_pin_conf_alt(reg_t *reg, int pin, int mode)
{
	unsigned int idx, t, shift, mask;

	idx = pin / 8;
	idx += 8; /* port base register + alt register offset(0x20) */
	shift = (pin % 8) * 4;
	mask = 0xf;

	t = reg[idx];
	t = MASK_RESET(t, mask << shift) | (mode << shift);
	reg[idx] = t;
}

unsigned int gpio_get(unsigned int index)
{
	unsigned int port, pin;
	reg_t *reg;

	if ((port = pin2port(index)) >= NR_PORT) {
		error("not supported port: %d", port);
		return -ERR_RANGE;
	}

	reg = port2reg(port);
	pin = pin2portpin(index);

	return (scan_port(reg) >> pin) & 1;
}

void gpio_put(unsigned int index, int v)
{
	unsigned int port, pin;
	reg_t *reg;

	if ((port = pin2port(index)) >= NR_PORT) {
		error("not supported port: %d", port);
		return;
	}

	reg = port2reg(port);
	pin = pin2portpin(index);

	write_port_pin(reg, pin, v & 1);
}

int gpio_init(unsigned int index, unsigned int flags)
{
	unsigned int port, pin, mode;
	int vector;
	unsigned int irqflag;
	reg_t *reg;

	if ((port = pin2port(index)) >= NR_PORT) {
		error("not supported port: %d", port);
		return 0;
	}

	vector = -1;
	mode = 0;
	pin = pin2portpin(index);
	reg = port2reg(port);


	spin_lock_irqsave(&gpio_init_lock, irqflag);

	if (state[port].pins & (1 << pin)) {
		error("already taken: %d", index);
		vector = 0;
		goto out;
	}

#if (SOC == stm32f1)
	__turn_apb2_clock(port + 2, ON);
#elif (SOC == stm32f3)
	__turn_ahb1_clock(port + 17, ON);
#elif (SOC == stm32f4)
	__turn_ahb1_clock(port, ON);
#endif

	/* default */
#if (SOC == stm32f3 || SOC == stm32f4)
	/* no pull-up, pull-down */
	reg[3] &= ~(3 << (pin * 2));
	/* very high speed I/O output speed */
	reg[2] |= 3 << (pin * 2);
	/* push-pull output */
	reg[1] &= ~(1 << pin);
#endif

	if (flags & GPIO_MODE_ALT) {
		mode = PIN_ALT;

#if (SOC == stm32f1)
		if (flags & GPIO_MODE_OUTPUT)
			mode |= PIN_OUTPUT;
#endif

#if (SOC == stm32f3 || SOC == stm32f4)
		if (pin / 8) {
			reg[9] = (reg[9] & ~(0xf << 4 * (pin % 8))) |
				((flags >> GPIO_ALT_SHIFT) << 4 * (pin % 8));
		} else {
			reg[8] = (reg[8] & ~(0xf << 4 * (pin % 8))) |
				((flags >> GPIO_ALT_SHIFT) << 4 * (pin % 8));
		}
#endif
	} else if (flags & GPIO_MODE_ANALOG) {
		mode = PIN_ANALOG;
	} else if (flags & GPIO_MODE_OUTPUT) {
		mode = PIN_OUTPUT;
	} else { /* GPIO_MODE_INPUT */
		mode = PIN_INPUT;
#if (SOC == stm32f1)
		mode |= PIN_FLOATING;
#endif
	}

	if (flags & GPIO_CONF_OPENDRAIN) {
#if (SOC == stm32f3 || SCO == stm32f4)
		reg[1] |= 1 << pin;
#elif (SOC == stm32f1)
		mode &= ~(PIN_FLOATING);
		if (flags & GPIO_MODE_ALT)
			mode |= PIN_ALT_OPENDRAIN;
		else
			mode |= PIN_OPENDRAIN;
#endif
	} else if (flags & GPIO_CONF_PULL_UP) {
#if (SOC == stm32f3 || SOC == stm32f4)
		reg[3] |= 1 << (pin * 2);
#elif (SOC == stm32f1)
		mode &= ~(PIN_FLOATING);
		mode |= PIN_PULL;
#endif
		write_port_pin(reg, pin, HIGH);
	} else if (flags & GPIO_CONF_PULL_DOWN) {
#if (SOC == stm32f3 || SOC == stm32f4)
		reg[3] |= 2 << (pin * 2);
#elif (SOC == stm32f1)
		mode &= ~(PIN_FLOATING);
		mode |= PIN_PULL;
#endif
		write_port_pin(reg, pin, LOW);
	}

	set_port_pin_conf(reg, pin, mode);

	if (flags & (GPIO_INT_FALLING | GPIO_INT_RISING)) {
#if (SOC == stm32f1)
		/* AFIO deals with Pin Remapping and EXTI */
		__turn_apb2_clock(0, ON);
#elif (SOC == stm32f3 || SOC == stm32f4)
		/* exti <- syscfg <- apb2 */
		__turn_apb2_clock(14, ON);
#endif
		EXTI_IMR |= 1 << pin;

		if (flags & GPIO_INT_FALLING)
			EXTI_FTSR |= 1 << pin;

		if (flags & GPIO_INT_RISING)
			EXTI_RTSR |= 1 << pin;

		switch (pin) {
		case 0 ... 4:
			nvic_set(pin + 6, ON);
			vector = pin + 22;
			break;
		case 5 ... 9:
			nvic_set(23, ON);
			vector = 39;
			break;
		case 10 ... 15:
			nvic_set(40, ON);
			vector = 56;
			break;
		default:
			break;
		}

		link_exti_to_nvic(port, pin);
	}

	state[port].pins |= 1 << pin;
	nr_active++;

out:
	spin_unlock_irqrestore(&gpio_init_lock, irqflag);

	return vector;
}

void gpio_reset(unsigned int index)
{
	unsigned int port, pin, irqflag;

	if ((port = pin2port(index)) >= NR_PORT) {
		error("not supported port: %d", port);
		return;
	}

	pin = pin2portpin(index);

	spin_lock_irqsave(&gpio_init_lock, irqflag);

	state[port].pins &= ~(1 << pin);
	nr_active--;
	assert(nr_active >= 0);

	barrier();
	if (!state[port].pins) {
#if (SOC == stm32f1)
		__turn_apb2_clock(port + 2, OFF);
#elif (SOC == stm32f3)
		__turn_ahb1_clock(port + 17, OFF);
#elif (SOC == stm32f4)
		__turn_ahb1_clock(port, OFF);
#endif
	}

	spin_unlock_irqrestore(&gpio_init_lock, irqflag);
}

unsigned int get_gpio_state(int port)
{
	if (port >= NR_PORT) {
		error("not supported port: %d", port);
		return -ERR_RANGE;
	}

	return state[port].pins;
}

#include <kernel/init.h>

static void __init port_init()
{
	/* FIXME: initializing of ports makes JTAG not working */
	return;
	__turn_port_clock((reg_t *)PORTA, ON);
	__turn_port_clock((reg_t *)PORTB, ON);
	__turn_port_clock((reg_t *)PORTC, ON);
	__turn_port_clock((reg_t *)PORTD, ON);
	__turn_port_clock((reg_t *)PORTE, ON);
	__turn_port_clock((reg_t *)PORTF, ON);

	unsigned int mode   = 0;
	unsigned int conf   = 0;
	unsigned int offset = 4;
#if (SOC == stm32f4)
	__turn_port_clock((reg_t *)PORTG, ON);
	__turn_port_clock((reg_t *)PORTH, ON);
	__turn_port_clock((reg_t *)PORTI, ON);

	mode   = 0xffffffff; /* analog mode */
	offset = 0xc;
#endif
	/* set pins to analog input(AIN) to reduce power consumption */
	*(reg_t *)PORTA = mode;
	*(reg_t *)(PORTA + offset) = conf;
	*(reg_t *)PORTB = mode;
	*(reg_t *)(PORTB + offset) = conf;
	*(reg_t *)PORTC = mode;
	*(reg_t *)(PORTC + offset) = conf;
	*(reg_t *)PORTD = mode;
	*(reg_t *)(PORTD + offset) = conf;
	*(reg_t *)PORTE = mode;
	*(reg_t *)(PORTE + offset) = conf;
	*(reg_t *)PORTF = mode;
	*(reg_t *)(PORTF + offset) = conf;
#if (SOC == stm32f4)
	*(reg_t *)PORTG = mode;
	*(reg_t *)(PORTG + offset) = conf;
	*(reg_t *)PORTH = mode;
	*(reg_t *)(PORTH + offset) = conf;
	*(reg_t *)PORTI = mode;
	*(reg_t *)(PORTI + offset) = conf;

	__turn_port_clock((reg_t *)PORTG, OFF);
	__turn_port_clock((reg_t *)PORTH, OFF);
	__turn_port_clock((reg_t *)PORTI, OFF);
#endif

	__turn_port_clock((reg_t *)PORTA, OFF);
	__turn_port_clock((reg_t *)PORTB, OFF);
	__turn_port_clock((reg_t *)PORTC, OFF);
	__turn_port_clock((reg_t *)PORTD, OFF);
	__turn_port_clock((reg_t *)PORTE, OFF);
	__turn_port_clock((reg_t *)PORTF, OFF);
}
REGISTER_INIT(port_init, 10);

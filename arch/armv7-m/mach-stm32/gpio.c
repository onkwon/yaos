/*
 * "[...] Sincerity (comprising truth-to-experience, honesty towards the self,
 * and the capacity for human empathy and compassion) is a quality which
 * resides within the laguage of literature. It isn't a fact or an intention
 * behind the work [...]"
 *
 *             - An introduction to Literary and Cultural Theory, Peter Barry
 *
 *
 *                                                   o8o
 *                                                   `"'
 *     oooo    ooo  .oooo.    .ooooo.   .oooo.o     oooo   .ooooo.
 *      `88.  .8'  `P  )88b  d88' `88b d88(  "8     `888  d88' `88b
 *       `88..8'    .oP"888  888   888 `"Y88b.       888  888   888
 *        `888'    d8(  888  888   888 o.  )88b .o.  888  888   888
 *         .8'     `Y888""8o `Y8bod8P' 8""888P' Y8P o888o `Y8bod8P'
 *     .o..P'
 *     `Y8P'                   Kyunghwan Kwon <kwon@yaos.io>
 *
 *  Welcome aboard!
 */

#include "include/gpio.h"
#include <types.h>
#include <error.h>
#include "include/exti.h"
#include "include/io.h"

static DEFINE_MUTEX(gpio_init_lock);

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
	unsigned int pending;
	int pin, mask;

#ifndef CONFIG_COMMON_IRQ_FRAMEWORK
	nvector = get_active_irq();
#endif
	pin = nvector;
	mask = 1;

	pin -= 22; /* EXTI0~4 */

	if (pin == 17) { /* EXTI5~9 */
		pin = 5;
		mask = 0x1f;
	} else if (pin == 34) { /* EXTI10~15 */
		pin = 10;
		mask = 0x3f;
	} else if (pin > 4) {
		error("unknown interrupt vector %x\n", nvector);
		return;
	}

	pending = get_exti_pending();

	while (mask) {
		if (pending & (1 << pin))
			break;
		mask >>= 1;
		pin++;
	}

	if (mask && isr_table[pin])
		isr_table[pin](nvector);

	clear_exti_pending(pin);
}

static inline int gpio2exti(int n)
{
	return pin2portpin(n);
}

static inline unsigned int scan_port(reg_t *reg)
{
	int idx = 4;
#if defined(stm32f1)
	idx = 2;
#endif
	return reg[idx];
}

static inline void write_port(reg_t *reg, unsigned int data)
{
	int idx = 5;
#if defined(stm32f1)
	idx = 3;
#endif
	reg[idx] = data;
}

static inline void write_port_pin(reg_t *reg, int pin, bool on)
{
	int idx = 6;
#if defined(stm32f1)
	idx = 4;
#endif
	reg[idx] = on? 1 << pin : 1 << (pin + 16);
}

static void set_port_pin_conf(reg_t *reg, int pin, int mode)
{
	unsigned int idx, t, shift, mask;

#if defined(stm32f1)
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

static void set_port_pin_conf_alt(reg_t *reg, int pin, int mode)
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

static inline int pin2vec(int pin)
{
	int nvector = 0;

	switch (pin) {
	case 0 ... 4:
		nvector = pin + 22;
		break;
	case 5 ... 9:
		nvector = 39;
		break;
	case 10 ... 15:
		nvector = 56;
		break;
	default:
		break;
	}

	return nvector;
}

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

	return -ERANGE;
}

unsigned int gpio_get(unsigned int index)
{
	unsigned int port, pin;
	reg_t *reg;

	if ((port = pin2port(index)) >= NR_PORT) {
		error("not supported port: %d", port);
		return -ERANGE;
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

#if defined(stm32f1)
int gpio_init(unsigned int index, unsigned int flags)
{
	unsigned int port, pin, mode;
	int lvector;
	reg_t *reg;

	if ((port = pin2port(index)) >= NR_PORT) {
		error("not supported port: %d", port);
		return -ERANGE;
	}

	lvector = 0;
	mode = 0;
	pin = pin2portpin(index);
	reg = port2reg(port);


	mutex_lock(&gpio_init_lock);

	if (state[port].pins & (1 << pin)) {
		error("already taken: %d", index);
		lvector = -EEXIST;
		goto out;
	}

	__turn_apb2_clock(port + 2, ON);

	if (flags & (GPIO_MODE_ALT | GPIO_MODE_OUTPUT)) {
		switch (flags & GPIO_SPD_MASK) {
		case GPIO_SPD_SLOW:
			mode |= PIN_OUTPUT_2MHZ;
			break;
		case GPIO_SPD_MID:
			mode |= PIN_OUTPUT_10MHZ;
			break;
		case GPIO_SPD_FAST:
		case GPIO_SPD_FASTEST:
			mode |= PIN_OUTPUT_50MHZ;
			break;
		default:
			if (flags & GPIO_MODE_OUTPUT)
				mode |= PIN_OUTPUT_2MHZ;
			/* or input */
			break;
		}
	}

	if (flags & GPIO_MODE_ALT)
		mode |= PIN_ALT;
	else if (flags & GPIO_MODE_ANALOG)
		mode |= PIN_ANALOG;
	else if (flags & GPIO_MODE_INPUT)
		mode |= PIN_INPUT | PIN_FLOATING;

	if (flags & GPIO_CONF_OPENDRAIN) {
		mode &= ~(PIN_FLOATING);
		if (flags & GPIO_MODE_ALT)
			mode |= PIN_ALT_OPENDRAIN;
		else
			mode |= PIN_OPENDRAIN;
	} else if (flags & GPIO_CONF_PULLUP) {
		mode &= ~(PIN_FLOATING);
		mode |= PIN_PULL;
		write_port_pin(reg, pin, HIGH);
	} else if (flags & GPIO_CONF_PULLDOWN) {
		mode &= ~(PIN_FLOATING);
		mode |= PIN_PULL;
		write_port_pin(reg, pin, LOW);
	}

	set_port_pin_conf(reg, pin, mode);

	if (flags & (GPIO_INT_FALLING | GPIO_INT_RISING)) {
		/* AFIO deals with pin remapping and EXTI */
		__turn_apb2_clock(0, ON);
		EXTI_IMR |= 1 << pin;

		if (flags & GPIO_INT_FALLING)
			EXTI_FTSR |= 1 << pin;

		if (flags & GPIO_INT_RISING)
			EXTI_RTSR |= 1 << pin;

		nvic_enable(pin2vec(pin), true);
		lvector = mkvector(pin2vec(pin), pin);

		exti_enable(index, ON);
	}

	state[port].pins |= 1 << pin;
	nr_active++;

out:
	mutex_unlock(&gpio_init_lock);

	return lvector;
}
#elif defined(stm32f3) || defined(stm32f4)
int gpio_init(unsigned int index, unsigned int flags)
{
	unsigned int port, pin, mode;
	int lvector;
	reg_t *reg;

	if ((port = pin2port(index)) >= NR_PORT) {
		error("not supported port: %d", port);
		return -ERANGE;
	}

	lvector = 0;
	mode = 0;
	pin = pin2portpin(index);
	reg = port2reg(port);


	mutex_lock(&gpio_init_lock);

	if (state[port].pins & (1 << pin)) {
		error("already taken: %d", index);
		lvector = -EEXIST;
		goto out;
	}

#if defined(stm32f3)
	__turn_ahb1_clock(port + 17, ON);
#elif defined(stm32f4)
	__turn_ahb1_clock(port, ON);
#else
#error undefined machine
#endif

	/* default */
	/* no pull-up, pull-down */
	reg[3] &= ~(3 << (pin * 2));
	/* low I/O output speed */
	reg[2] &= ~(3 << (pin * 2));
	/* push-pull output */
	reg[1] &= ~(1 << pin);

	if (flags & (GPIO_MODE_ALT | GPIO_MODE_OUTPUT)) {
		switch (flags & GPIO_SPD_MASK) {
		case GPIO_SPD_MID:
			reg[2] |= 1 << (pin * 2);
			break;
		case GPIO_SPD_FAST:
			reg[2] |= 2 << (pin * 2);
			break;
		case GPIO_SPD_FASTEST:
			reg[2] |= 3 << (pin * 2);
			break;
		default:
		case GPIO_SPD_SLOW:
			break;
		}
	}

	if (flags & GPIO_MODE_ALT) {
		mode |= PIN_ALT;
		set_port_pin_conf_alt(reg, pin, gpio_altfunc_get(flags));
	} else if (flags & GPIO_MODE_ANALOG) {
		mode |= PIN_ANALOG;
	} else if (flags & GPIO_MODE_OUTPUT) {
		mode |= PIN_OUTPUT;
	} else if (flags & GPIO_MODE_INPUT) {
		mode |= PIN_INPUT;
	}

	if (flags & GPIO_CONF_OPENDRAIN) {
		reg[1] |= 1 << pin;
	} else if (flags & GPIO_CONF_PULLUP) {
		reg[3] |= 1 << (pin * 2);
		write_port_pin(reg, pin, HIGH);
	} else if (flags & GPIO_CONF_PULLDOWN) {
		reg[3] |= 2 << (pin * 2);
		write_port_pin(reg, pin, LOW);
	}

	set_port_pin_conf(reg, pin, mode);

	if (flags & (GPIO_INT_FALLING | GPIO_INT_RISING)) {
		/* exti <- syscfg <- apb2 */
		__turn_apb2_clock(14, ON);
		EXTI_IMR |= 1 << pin;

		if (flags & GPIO_INT_FALLING)
			EXTI_FTSR |= 1 << pin;

		if (flags & GPIO_INT_RISING)
			EXTI_RTSR |= 1 << pin;

		nvic_enable(pin2vec(pin), true);
		lvector = mkvector(pin2vec(pin), pin);

		exti_enable(index, ON);
	}

	state[port].pins |= 1 << pin;
	nr_active++;

out:
	mutex_unlock(&gpio_init_lock);

	return lvector;
}
#endif

void gpio_fini(unsigned int index)
{
	unsigned int port, pin, lvector;

	if ((port = pin2port(index)) >= NR_PORT) {
		error("not supported port: %d", port);
		return;
	}

	pin = pin2portpin(index);

	mutex_lock(&gpio_init_lock);

	state[port].pins &= ~(1 << pin);
	nr_active--;
	assert(nr_active >= 0);

	barrier();
	if (!state[port].pins) {
#if defined(stm32f1)
		__turn_apb2_clock(port + 2, OFF);
#elif defined(stm32f3)
		__turn_ahb1_clock(port + 17, OFF);
#elif defined(stm32f4)
		__turn_ahb1_clock(port, OFF);
#else
#error undefined machine
#endif
	}

	exti_enable(index, OFF);
	lvector = mkvector(pin2vec(pin), pin);
	unregister_isr(lvector);

	mutex_unlock(&gpio_init_lock);
}

unsigned int get_gpio_state(int port)
{
	if (port >= NR_PORT) {
		error("not supported port: %d", port);
		return -ERANGE;
	}

	return state[port].pins;
}

#include <kernel/init.h>

static inline void gpio_irq_init()
{
	int i;

	for (i = 0; i < PINS_PER_PORT; i++)
		isr_table[i] = NULL;

	register_isr(22, ISR_gpio); /* EXTI0 */
	register_isr(23, ISR_gpio); /* EXTI1 */
	register_isr(24, ISR_gpio); /* EXTI2 */
	register_isr(25, ISR_gpio); /* EXTI3 */
	register_isr(26, ISR_gpio); /* EXTI4 */
	register_isr(39, ISR_gpio); /* EXTI5~9 */
	register_isr(56, ISR_gpio); /* EXTI10~15 */

	register_isr_register(22, irq_register, 0);
	register_isr_register(23, irq_register, 0);
	register_isr_register(24, irq_register, 0);
	register_isr_register(25, irq_register, 0);
	register_isr_register(26, irq_register, 0);
	register_isr_register(39, irq_register, 0);
	register_isr_register(56, irq_register, 0);
}

static void __init port_init()
{
	gpio_irq_init();

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
#ifdef stm32f4
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
#ifdef stm32f4
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

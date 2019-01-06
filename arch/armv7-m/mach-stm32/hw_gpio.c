#include "drivers/gpio.h"
#include "kernel/interrupt.h"
#include "syslog.h"
#include "io.h"

#include "include/hw_exti.h"
#include "include/hw_clock.h"

#include <errno.h>
#include <assert.h>

#include "arch/mach/board/hw.h"

#ifndef NR_PORT
#define NR_PORT			5U
#endif

static inline int gpio2exti(int n)
{
	return gpio_to_ppin(n);
}

static inline uintptr_t scan_port(reg_t *reg)
{
	uint8_t idx = 4;
#if defined(stm32f1)
	idx = 2;
#endif
	return reg[idx];
}

static inline void write_port(reg_t *reg, reg_t data)
{
	uint8_t idx = 5;
#if defined(stm32f1)
	idx = 3;
#endif
	reg[idx] = data;
}

static inline void write_port_pin(reg_t *reg, uint16_t pin, int val)
{
	uint8_t idx = 6;
#if defined(stm32f1)
	idx = 4;
#endif
	reg[idx] = (val == 1)? (1UL << pin) : (1UL << (pin + 16));
}

static inline int pin2vec(uint16_t pin)
{
	int nvector = 0;

	switch (pin) {
	case 0: case 1: case 2: case 3: case 4:
		nvector = pin + 22;
		break;
	case 5: case 6: case 7: case 8: case 9:
		nvector = 39;
		break;
	case 10: case 11: case 12: case 13: case 14: case 15:
		nvector = 56;
		break;
	default:
		break;
	}

	return nvector;
}

static void set_port_pin_conf(reg_t *reg, uint16_t pin, uint16_t mode)
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
	t = MASK_RESET(t, mask << shift) | ((unsigned int)mode << shift);
	reg[idx] = t;
}

void hw_gpio_clear_event(uint16_t pin)
{
	hw_exti_clear_pending(pin);
}

uint16_t hw_gpio_get_event_source(int vector)
{
	unsigned int pending;
	uint16_t pin, mask;

	pin = vector;
	mask = 1;

	pin -= 22; /* EXTI0~4 */

	if (pin == 17) { /* EXTI5~9 */
		pin = 5;
		mask = 0x1f;
	} else if (pin == 34) { /* EXTI10~15 */
		pin = 10;
		mask = 0x3f;
	} else if (pin > 4) {
		debug("unknown interrupt vector %x\n", vector);
		pin = -ENOENT;
		goto out;
	}

	pending = hw_exti_get_pending();

	while (mask) {
		if (pending & (1 << pin))
			break;
		mask >>= 1;
		pin++;
	}

	if (!mask)
		pin = -ERANGE;
out:
	return pin;
}

int hw_gpio_get(const uint16_t index)
{
	uint16_t port, pin;
	reg_t *reg;

	if ((port = gpio_to_port(index)) >= NR_PORT) {
		debug("not supported port: %d", port);
		return -ERANGE;
	}

	reg = gpio_to_reg(index);
	pin = gpio_to_ppin(index);

	return (int)((scan_port(reg) >> pin) & 1UL);
}

void hw_gpio_put(const uint16_t index, const int val)
{
	uint16_t port, pin;
	reg_t *reg;

	if ((port = gpio_to_port(index)) >= NR_PORT) {
		debug("not supported port: %d", port);
		return;
	}

	reg = gpio_to_reg(index);
	pin = gpio_to_ppin(index);

	write_port_pin(reg, pin, val);
}

#if defined(stm32f1)
int hw_gpio_init(const uint16_t index, const uint32_t flags)
{
	uint16_t port, pin, mode;
	int lvector;
	reg_t *reg;

	if ((port = gpio_to_port(index)) >= NR_PORT) {
		debug("not supported port: %d", port);
		return -ERANGE;
	}

	lvector = 0;
	mode = 0;
	pin = gpio_to_ppin(index);
	reg = gpio_to_reg(index);

	hw_clock_set_apb2(port + 2, true);

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
		write_port_pin(reg, pin, 1);
	} else if (flags & GPIO_CONF_PULLDOWN) {
		mode &= ~(PIN_FLOATING);
		mode |= PIN_PULL;
		write_port_pin(reg, pin, 0);
	}

	set_port_pin_conf(reg, pin, mode);

	if (flags & (GPIO_INT_FALLING | GPIO_INT_RISING)) {
		/* AFIO deals with pin remapping and EXTI */
		hw_clock_set_apb2(0, true);
		EXTI_IMR |= 1 << pin;

		if (flags & GPIO_INT_FALLING)
			EXTI_FTSR |= 1 << pin;

		if (flags & GPIO_INT_RISING)
			EXTI_RTSR |= 1 << pin;

		hw_irq_set(pin2vec(pin), true);
		//lvector = mkvector(pin2vec(pin), pin);
		lvector = (int)pin + 1;

		hw_exti_enable(index, true);
	}

	return lvector;
}
#elif defined(stm32f3) || defined(stm32f4)
static void set_port_pin_conf_alt(reg_t *reg, uint16_t pin, uint16_t mode)
{
	unsigned int idx, t, shift, mask;

	idx = pin / 8;
	idx += 8; /* port base register + alt register offset(0x20) */
	shift = (pin % 8) * 4;
	mask = 0xf;

	t = reg[idx];
	t = MASK_RESET(t, mask << shift) | ((unsigned int)mode << shift);
	reg[idx] = t;
}

int hw_gpio_init(const uint16_t index, const uint32_t flags)
{
	uint16_t port, pin, mode;
	int lvector;
	reg_t *reg;

	if ((port = gpio_to_port(index)) >= NR_PORT) {
		debug("not supported port: %d", port);
		return -ERANGE;
	}

	lvector = 0;
	mode = 0;
	pin = gpio_to_ppin(index);
	reg = gpio_to_reg(index);

#if defined(stm32f3)
	hw_clock_set_ahb1(port + 17, true);
#elif defined(stm32f4)
	hw_clock_set_ahb1(port, true);
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
		write_port_pin(reg, pin, 1);
	} else if (flags & GPIO_CONF_PULLDOWN) {
		reg[3] |= 2 << (pin * 2);
		write_port_pin(reg, pin, 0);
	}

	set_port_pin_conf(reg, pin, mode);

	if (flags & (GPIO_INT_FALLING | GPIO_INT_RISING)) {
		/* exti <- syscfg <- apb2 */
		hw_clock_set_apb2(14, true);
		EXTI_IMR |= 1 << pin;

		if (flags & GPIO_INT_FALLING)
			EXTI_FTSR |= 1 << pin;

		if (flags & GPIO_INT_RISING)
			EXTI_RTSR |= 1 << pin;

		hw_irq_set(pin2vec(pin), true);
		//lvector = mkvector(pin2vec(pin), pin);
		lvector = (int)pin + 1;

		hw_exti_enable(index, true);
	}

out:
	return lvector;
}
#endif

void hw_gpio_fini(const uint16_t index)
{
	uint16_t port, pin;
	unsigned int lvector;

	if ((port = gpio_to_port(index)) >= NR_PORT) {
		debug("not supported port: %d", port);
		return;
	}

	pin = gpio_to_ppin(index);

	barrier();
#if 0
	if (!state[port].pins) {
#if defined(stm32f1)
		__turn_apb2_clock(port + 2, false);
#elif defined(stm32f3)
		__turn_ahb1_clock(port + 17, false);
#elif defined(stm32f4)
		__turn_ahb1_clock(port, false);
#else
#error undefined machine
#endif
	}
#endif

	// FIXME: disable only if enable
	hw_exti_enable(index, false);
	lvector = mkvector(pin2vec(pin), pin);
	unregister_isr(lvector);
}

static inline void hw_gpio_irq_init(void (*f)(int))
{
	register_isr(22, f); /* EXTI0 */
	register_isr(23, f); /* EXTI1 */
	register_isr(24, f); /* EXTI2 */
	register_isr(25, f); /* EXTI3 */
	register_isr(26, f); /* EXTI4 */
	register_isr(39, f); /* EXTI5~9 */
	register_isr(56, f); /* EXTI10~15 */
}

void hw_gpio_driver_init(void (*f)(int))
{
	hw_gpio_irq_init(f);

	/* FIXME: initializing of ports makes JTAG not working */
	return;
	hw_clock_set_port((reg_t *)PORTA, true);
	hw_clock_set_port((reg_t *)PORTB, true);
	hw_clock_set_port((reg_t *)PORTC, true);
	hw_clock_set_port((reg_t *)PORTD, true);
	hw_clock_set_port((reg_t *)PORTE, true);
	hw_clock_set_port((reg_t *)PORTF, true);

	unsigned int mode   = 0;
	unsigned int conf   = 0;
	unsigned int offset = 4;
#ifdef stm32f4
	hw_clock_set_port((reg_t *)PORTG, true);
	hw_clock_set_port((reg_t *)PORTH, true);
	hw_clock_set_port((reg_t *)PORTI, true);

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

	hw_clock_set_port((reg_t *)PORTG, false);
	hw_clock_set_port((reg_t *)PORTH, false);
	hw_clock_set_port((reg_t *)PORTI, false);
#endif

	hw_clock_set_port((reg_t *)PORTA, false);
	hw_clock_set_port((reg_t *)PORTB, false);
	hw_clock_set_port((reg_t *)PORTC, false);
	hw_clock_set_port((reg_t *)PORTD, false);
	hw_clock_set_port((reg_t *)PORTE, false);
	hw_clock_set_port((reg_t *)PORTF, false);
}

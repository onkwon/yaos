#include "drivers/gpio.h"
#include "kernel/interrupt.h"
#include "log.h"
#include "io.h"

#include "include/hw_exti.h"
#include "include/clock.h"

#include <errno.h>
#include <assert.h>

#include "arch/mach/board/hw.h"

#ifndef NR_PORT
#define NR_PORT			5U
#endif

static struct gpio {
	union {
		struct {
			uint16_t pin0: 1;
			uint16_t pin1: 1;
			uint16_t pin2: 1;
			uint16_t pin3: 1;
			uint16_t pin4: 1;
			uint16_t pin5: 1;
			uint16_t pin6: 1;
			uint16_t pin7: 1;
			uint16_t pin8: 1;
			uint16_t pin9: 1;
			uint16_t pin10: 1;
			uint16_t pin11: 1;
			uint16_t pin12: 1;
			uint16_t pin13: 1;
			uint16_t pin14: 1;
			uint16_t pin15: 1;
		};

		uint16_t pins;
	};
} state[NR_PORT];

static int nr_active; /* number of active pins */

static void (*isr_table[PINS_PER_PORT])(int nvector);

static int irq_register(const int lvector, void (*handler)(const int))
{
	uint16_t pin = get_secondary_vector(lvector);

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
	uint16_t pin, mask;

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
		debug("unknown interrupt vector %x\n", nvector);
		return;
	}

	pending = hw_exti_get_pending();

	while (mask) {
		if (pending & (1 << pin))
			break;
		mask >>= 1;
		pin++;
	}

	if (mask && isr_table[pin])
		isr_table[pin](nvector);

	hw_exti_clear_pending(pin);
}

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

int __gpio_get(const uint16_t index)
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

void __gpio_put(const uint16_t index, const int val)
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
int __gpio_init(const uint16_t index, const uint32_t flags)
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

	if (state[port].pins & (1 << pin)) {
		debug("already taken: %d", index);
		lvector = -EEXIST;
		goto out;
	}

	__turn_apb2_clock(port + 2, true);

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
		__turn_apb2_clock(0, true);
		EXTI_IMR |= 1 << pin;

		if (flags & GPIO_INT_FALLING)
			EXTI_FTSR |= 1 << pin;

		if (flags & GPIO_INT_RISING)
			EXTI_RTSR |= 1 << pin;

		hw_irq_set(pin2vec(pin), true);
		lvector = mkvector(pin2vec(pin), pin);

		hw_exti_enable(index, true);
	}

	state[port].pins |= 1 << pin;
	nr_active++;

out:
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

int __gpio_init(const uint16_t index, const uint32_t flags)
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

	if (state[port].pins & (1 << pin)) {
		debug("already taken: %d", index);
		lvector = -EEXIST;
		goto out;
	}

#if defined(stm32f3)
	__turn_ahb1_clock(port + 17, true);
#elif defined(stm32f4)
	__turn_ahb1_clock(port, true);
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
		__turn_apb2_clock(14, true);
		EXTI_IMR |= 1 << pin;

		if (flags & GPIO_INT_FALLING)
			EXTI_FTSR |= 1 << pin;

		if (flags & GPIO_INT_RISING)
			EXTI_RTSR |= 1 << pin;

		hw_irq_set(pin2vec(pin), true);
		lvector = mkvector(pin2vec(pin), pin);

		hw_exti_enable(index, true);
	}

	state[port].pins |= 1 << pin;
	nr_active++;

out:
	return lvector;
}
#endif

void __gpio_fini(const uint16_t index)
{
	uint16_t port, pin;
	unsigned int lvector;

	if ((port = gpio_to_port(index)) >= NR_PORT) {
		debug("not supported port: %d", port);
		return;
	}

	pin = gpio_to_ppin(index);

	state[port].pins &= ~(1U << pin);
	nr_active--;
	assert(nr_active >= 0);

	barrier();
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

	hw_exti_enable(index, false);
	lvector = mkvector(pin2vec(pin), pin);
	unregister_isr(lvector);
}

uint16_t __gpio_get_status(uint8_t port)
{
	if (port >= NR_PORT) {
		debug("not supported port: %d", port);
		return -ERANGE;
	}

	return state[port].pins;
}

#include <kernel/init.h>

static inline void gpio_irq_init(void)
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

static void __init port_init(void)
{
	gpio_irq_init();

	/* FIXME: initializing of ports makes JTAG not working */
	return;
	__turn_port_clock((reg_t *)PORTA, true);
	__turn_port_clock((reg_t *)PORTB, true);
	__turn_port_clock((reg_t *)PORTC, true);
	__turn_port_clock((reg_t *)PORTD, true);
	__turn_port_clock((reg_t *)PORTE, true);
	__turn_port_clock((reg_t *)PORTF, true);

	unsigned int mode   = 0;
	unsigned int conf   = 0;
	unsigned int offset = 4;
#ifdef stm32f4
	__turn_port_clock((reg_t *)PORTG, true);
	__turn_port_clock((reg_t *)PORTH, true);
	__turn_port_clock((reg_t *)PORTI, true);

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

	__turn_port_clock((reg_t *)PORTG, false);
	__turn_port_clock((reg_t *)PORTH, false);
	__turn_port_clock((reg_t *)PORTI, false);
#endif

	__turn_port_clock((reg_t *)PORTA, false);
	__turn_port_clock((reg_t *)PORTB, false);
	__turn_port_clock((reg_t *)PORTC, false);
	__turn_port_clock((reg_t *)PORTD, false);
	__turn_port_clock((reg_t *)PORTE, false);
	__turn_port_clock((reg_t *)PORTF, false);
}
REGISTER_INIT(port_init, 10);

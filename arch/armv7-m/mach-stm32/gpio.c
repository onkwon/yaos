#include <gpio.h>
#include <types.h>
#include "exti.h"

#ifndef stm32f1
#define stm32f1	1
#define stm32f4	2
#endif

DEFINE_SPINLOCK(gpio_irq_lock);
static DEFINE_SPINLOCK(gpio_init_lock);

#define calc_port(i)		(i / PINS_PER_PORT)
#define calc_pin(i)		(i % PINS_PER_PORT)
#define calc_port_addr(p)	((((p) * WORD_SIZE) << 8) + PORTA)

unsigned int gpio_get(unsigned int index)
{
	unsigned int port, pin;

	port = calc_port_addr(calc_port(index));
	pin  = calc_pin(index);

	return (GET_PORT(port) >> pin) & 1;
}

void gpio_put(unsigned int index, int v)
{
	unsigned int port, pin;

	port = calc_port_addr(calc_port(index));
	pin  = calc_pin(index);

	PUT_PORT_PIN(port, pin, v & 1);
}

int gpio_init(unsigned int index, unsigned int flags)
{
	unsigned int port, pin, mode;
	int vector;
	unsigned int irqflag;

	vector = -1;
	mode = 0;
	port = calc_port(index);
	pin  = calc_pin(index);

	spin_lock_irqsave(&gpio_init_lock, irqflag);

#if (SOC == stm32f1)
	SET_CLOCK_APB2(ENABLE, port + 2);
#elif (SOC == stm32f4)
	SET_CLOCK_AHB1(ENABLE, port);
#endif

	port = calc_port_addr(port);

	/* default */
#if (SOC == stm32f4)
	/* no pull-up, pull-down */
	*(volatile unsigned int *)(port + 0xc) &= ~(3 << (pin * 2));
	/* very high speed I/O output speed */
	*(volatile unsigned int *)(port + 8) |= 3 << (pin * 2);
	/* push-pull output */
	*(volatile unsigned int *)(port + 4) &= ~(1 << pin);
#endif

	if (flags & GPIO_MODE_OUTPUT) {
		mode = PIN_OUTPUT;
	} else if (flags & GPIO_MODE_ALT) {
		mode = PIN_ALT;
	} else if (flags & GPIO_MODE_ANALOG) {
		mode = PIN_ANALOG;
	} else { /* GPIO_MODE_INPUT */
		mode = PIN_INPUT;
#if (SOC == stm32f1)
		mode |= PIN_FLOATING;
#endif
	}

	if (flags & GPIO_CONF_OPENDRAIN) {
#if (SOC == stm32f4)
		*(volatile unsigned int *)(port + 4) |= 1 << pin;
#elif (SOC == stm32f1)
		mode &= ~(PIN_FLOATING);
		if (flags & GPIO_MODE_ALT)
			mode |= PIN_ALT_OPENDRAIN;
		else
			mode |= PIN_OPENDRAIN;
#endif
	} else if (flags & GPIO_CONF_PULL_UP) {
#if (SOC == stm32f4)
		*(volatile unsigned int *)(port + 0xc) |= 1 << (pin * 2);
#elif (SOC == stm32f1)
		mode &= ~(PIN_FLOATING);
		mode |= PIN_PULL;
#endif
		PUT_PORT_PIN(port, pin, 1);
	} else if (flags & GPIO_CONF_PULL_DOWN) {
#if (SOC == stm32f4)
		*(volatile unsigned int *)(port + 0xc) |= 2 << (pin * 2);
#elif (SOC == stm32f1)
		mode &= ~(PIN_FLOATING);
		mode |= PIN_PULL;
#endif
		PUT_PORT_PIN(port, pin, 0);
	}

	SET_PORT_PIN(port, pin, mode);

	if (flags & (GPIO_INT_FALLING | GPIO_INT_RISING)) {
#if (SOC == stm32f1)
		/* AFIO deals with Pin Remapping and EXTI */
		SET_CLOCK_APB2(ENABLE, 0);
#elif (SOC == stm32f4)
		/* exti <- syscfg <- apb2 */
		SET_CLOCK_APB2(ENABLE, 14);
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

		link_exti_to_nvic(calc_port(index), pin);
	}

	spin_unlock_irqrestore(&gpio_init_lock, irqflag);

	return vector;
}

void gpio_close(unsigned int index)
{
}

#include <kernel/init.h>

static void __init port_init()
{
	SET_PORT_CLOCK(ENABLE, PORTA);
	SET_PORT_CLOCK(ENABLE, PORTB);
	SET_PORT_CLOCK(ENABLE, PORTC);
	SET_PORT_CLOCK(ENABLE, PORTD);
	SET_PORT_CLOCK(ENABLE, PORTE);
	SET_PORT_CLOCK(ENABLE, PORTF);
	SET_PORT_CLOCK(ENABLE, PORTG);

	unsigned int mode   = 0;
	unsigned int conf   = 0;
	unsigned int offset = 4;
#if (SOC == stm32f4)
	SET_PORT_CLOCK(ENABLE, PORTH);
	SET_PORT_CLOCK(ENABLE, PORTI);

	mode   = 0xffffffff; /* analog mode */
	offset = 0xc;
#endif
	/* set pins to analog input(AIN) to reduce power consumption */
	*(volatile unsigned int *)PORTA = mode;
	*(volatile unsigned int *)(PORTA + offset) = conf;
	*(volatile unsigned int *)PORTB = mode;
	*(volatile unsigned int *)(PORTB + offset) = conf;
	*(volatile unsigned int *)PORTC = mode;
	*(volatile unsigned int *)(PORTC + offset) = conf;
	*(volatile unsigned int *)PORTD = mode;
	*(volatile unsigned int *)(PORTD + offset) = conf;
	*(volatile unsigned int *)PORTE = mode;
	*(volatile unsigned int *)(PORTE + offset) = conf;
	*(volatile unsigned int *)PORTF = mode;
	*(volatile unsigned int *)(PORTF + offset) = conf;
	*(volatile unsigned int *)PORTG = mode;
	*(volatile unsigned int *)(PORTG + offset) = conf;
#if (SOC == stm32f4)
	*(volatile unsigned int *)PORTH = mode;
	*(volatile unsigned int *)(PORTH + offset) = conf;
	*(volatile unsigned int *)PORTI = mode;
	*(volatile unsigned int *)(PORTI + offset) = conf;

	SET_PORT_CLOCK(DISABLE, PORTH);
	SET_PORT_CLOCK(DISABLE, PORTI);
#endif

	SET_PORT_CLOCK(DISABLE, PORTA);
	SET_PORT_CLOCK(DISABLE, PORTB);
	SET_PORT_CLOCK(DISABLE, PORTC);
	SET_PORT_CLOCK(DISABLE, PORTD);
	SET_PORT_CLOCK(DISABLE, PORTE);
	SET_PORT_CLOCK(DISABLE, PORTF);
	SET_PORT_CLOCK(DISABLE, PORTG);
}
REGISTER_INIT(port_init, 10);

#include <gpio.h>
#include <types.h>

DEFINE_SPINLOCK(gpio_irq_lock);
DEFINE_SPINLOCK(gpio_init_lock);

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

	spin_lock_irqsave(gpio_init_lock, irqflag);

	SET_CLOCK_APB2(ENABLE, port + 2);

	port = calc_port_addr(port);

	if (flags & GPIO_MODE_OUTPUT) {
		mode |= PIN_OUTPUT_50MHZ;

		if (flags & GPIO_CONF_ALT) {
			SET_CLOCK_APB2(ENABLE, 0);
			mode |= PIN_ALT;
			if (flags & GPIO_CONF_OPENDRAIN)
				mode |= PIN_ALT_OPENDRAIN;
		} else { /* GPIO_CONF_GENERAL */
			if (flags & GPIO_CONF_OPENDRAIN)
				mode |= PIN_OPENDRAIN;
		}
	} else { /* GPIO_MODE_INPUT */
		mode |= PIN_INPUT;

		if (flags & GPIO_CONF_ANALOG) {
			mode |= PIN_ANALOG;
		} else if (flags & GPIO_CONF_FLOAT) {
			mode |= PIN_FLOATING;
		} else if (flags & GPIO_CONF_PULL) {
			mode |= PIN_PULL;
			PUT_PORT(port, 1 << pin);
		}
	}

	SET_PORT_PIN(port, pin, mode);

	if (flags & (GPIO_INT_FALLING | GPIO_INT_RISING)) {
		EXTI_IMR  |= 1 << pin;

		if (flags & GPIO_INT_FALLING)
			EXTI_FTSR |= 1 << pin;

		if (flags & GPIO_INT_RISING)
			EXTI_RTSR |= 1 << pin;

		switch (pin) {
		case 0 ... 4:
			SET_IRQ(ON, pin + 6);
			vector = pin + 22;
			break;
		case 5 ... 9:
			SET_IRQ(ON, 23);
			vector = 39;
			break;
		case 10 ... 15:
			SET_IRQ(ON, 40);
			vector = 56;
			break;
		default:
			break;
		}
	}

	spin_unlock_irqrestore(gpio_init_lock, irqflag);

	return vector;
}

void gpio_close(unsigned int index)
{
}

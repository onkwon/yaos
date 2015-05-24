#include <gpio.h>

int gpio_init(unsigned int index, unsigned int flags)
{
	unsigned int port, pin, mode = 0;
	int vector = -1;

	port = (index / PINS_PER_PORT) + 1;
	pin  = index % PINS_PER_PORT;

	SET_CLOCK_APB2(ENABLE, port << 2);

	port = ((port * sizeof(int)) << 10) + PORTA;

	if (flags & GPIO_MODE_OUTPUT) {
		if (flags & GPIO_CONF_ALT) {
			SET_CLOCK_APB2(ENABLE, 0);
		} else { /* GPIO_CONF_GENERAL */
		}

		if (flags & GPIO_CONF_OPENDRAIN) {
		} else { /* GPIO_CONF_PUSHPULL */
		}
	} else { /* GPIO_MODE_INPUT */
		mode |= PIN_INPUT;

		if (flags & GPIO_CONF_ANALOG) {
		} else if (flags & GPIO_CONF_FLOAT) {
		} else if (flags & GPIO_CONF_PULL) {
			PUT_PORT(port, 1 << pin);
			mode |= PIN_PULL;
		}
	}

	SET_PORT_PIN(port, pin, mode);

	if (flags & (GPIO_INT_FALLING | GPIO_INT_RISING)) {
		EXTI_IMR  |= 1 << pin;

		if (flags & GPIO_INT_FALLING) {
			SET_CLOCK_APB2(ENABLE, 0);
			EXTI_FTSR |= 1 << pin;
		}
		if (flags & GPIO_INT_RISING) {
			SET_CLOCK_APB2(ENABLE, 0);
			EXTI_RTSR |= 1 << pin;
		}

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

	return vector;
}

void gpio_close(unsigned int index)
{
}

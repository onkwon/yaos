#ifndef __STM32_GPIO_H__
#define __STM32_GPIO_H__

#define NR_PORT			7
#define PINS_PER_PORT		16

#define pin2port(pin)		((pin) / PINS_PER_PORT)
#define pin2portpin(pin)	((pin) % PINS_PER_PORT)
#define port2reg(port)		((reg_t *)((((port) * WORD_SIZE) << 8) + PORTA))

#include <io.h>
#include <kernel/lock.h>

/* FIXME: simultaneously generated interrupts between EXTI5~15 will be missed
 * those interrupts share an interrupt vector and we clear all that bits at
 * once below, we will miss except the first one. */
static inline void ret_from_exti(unsigned int n)
{
	int mask = 1;

	n -= 22; /* EXTI0~4 */

	if (n > 21) { /* EXTI10~15 */
		n = 10;
		mask = 0x3f;
	} else if (n > 4) { /* EXTI5~9 */
		n = 5;
		mask = 0x1f;
	}

	/* FIXME: handle EXTI in an isolated func considering sync, lock */
	EXTI_PR |= mask << n;
}

static inline int gpio2exti(int n)
{
	return pin2portpin(n);
}

static inline void ret_from_gpio_int(unsigned int n)
{
	unsigned int pin = n % PINS_PER_PORT;

	EXTI_PR |= 1 << pin;
}

static inline unsigned int scan_port(reg_t *reg)
{
	int idx = 4;
#if (SOC == stm32f1)
	idx = 2;
#endif
	return reg[idx];
}

static inline void write_port(reg_t *reg, unsigned int data)
{
	int idx = 5;
#if (SOC == stm32f1)
	idx = 3;
#endif
	reg[idx] = data;
}

static inline void write_port_pin(reg_t *reg, int pin, bool on)
{
	int idx = 6;
#if (SOC == stm32f1)
	idx = 4;
#endif
	reg[idx] = on? 1 << pin : 1 << (pin + 16);
}

void set_port_pin_conf(reg_t *reg, int pin, int mode);
void set_port_pin_conf_alt(reg_t *reg, int pin, int mode);
int reg2port(reg_t *reg);

#endif /* __STM32_GPIO_H__ */

#ifndef __STM32_GPIO_H__
#define __STM32_GPIO_H__

#define NR_PORT			7
#define PINS_PER_PORT		16

#include <io.h>
#include <kernel/lock.h>

static inline void ret_from_gpio_int(unsigned int n)
{
#ifdef CONFIG_SMP
	extern lock_t gpio_irq_lock;
#endif
	unsigned int irqflag;

	unsigned int pin = n % PINS_PER_PORT;

	spin_lock_irqsave(&gpio_irq_lock, irqflag);
	EXTI_PR |= 1 << pin;
	spin_unlock_irqrestore(&gpio_irq_lock, irqflag);
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

#ifndef __STM32_GPIO_H__
#define __STM32_GPIO_H__

#include <asm/hw.h>

#ifndef NR_PORT
#define NR_PORT			5
#endif
#define PINS_PER_PORT		16

#define pin2port(pin)		((pin) / PINS_PER_PORT)
#define pin2portpin(pin)	((pin) % PINS_PER_PORT)
#define port2reg(port)		((reg_t *)((((port) * WORD_SIZE) << 8) + PORTA))

#include <io.h>
#include <kernel/lock.h>

int reg2port(reg_t *reg);

#endif /* __STM32_GPIO_H__ */

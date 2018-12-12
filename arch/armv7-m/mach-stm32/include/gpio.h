#ifndef __YAOS_STM32_GPIO_H__
#define __YAOS_STM32_GPIO_H__

#include "types.h"
#include <stdint.h>

#define PINS_PER_PORT		16

#define gpio_to_port(pin)	((pin) / PINS_PER_PORT)
#define gpio_to_ppin(pin)	((pin) % PINS_PER_PORT)
#define gpio_to_reg(pin)	\
	((reg_t *)((((gpio_to_port(pin)) * WORD_SIZE) << 8) + PORTA))

int __gpio_get(const uint16_t index);
void __gpio_put(const uint16_t index, const int val);
int __gpio_init(const uint16_t index, const uint32_t flags);
void __gpio_fini(const uint16_t index);
uint16_t __gpio_get_status(const uint8_t port);

#endif /* __YAOS_STM32_GPIO_H__ */

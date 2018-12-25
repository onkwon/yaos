#ifndef __YAOS_STM32_GPIO_H__
#define __YAOS_STM32_GPIO_H__

#include "types.h"
#include <stdint.h>

#define PINS_PER_PORT		16

#define gpio_to_port(pin)	((pin) / PINS_PER_PORT)
#define gpio_to_ppin(pin)	((pin) % PINS_PER_PORT)
#define gpio_to_reg(pin)	\
	((reg_t *)((((gpio_to_port(pin)) * WORD_SIZE) << 8) + PORTA))

int hw_gpio_init(const uint16_t index, const uint32_t flags);
void hw_gpio_fini(const uint16_t index);
int hw_gpio_get(const uint16_t index);
void hw_gpio_put(const uint16_t index, const int val);
uint16_t hw_gpio_get_event_source(int vector);
void hw_gpio_clear_event(uint16_t pin);

void hw_gpio_driver_init(void (*f)(int));

#endif /* __YAOS_STM32_GPIO_H__ */

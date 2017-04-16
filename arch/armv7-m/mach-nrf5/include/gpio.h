#ifndef __NRF5_GPIO_H__
#define __NRF5_GPIO_H__

#include <error.h>
/* to not include nrf_assert.h */
#define NRF_ASSERT_H_
#define ASSERT		assert

#include <types.h>

#ifndef __IO_H__
extern size_t printk(const char *format, ...);
#endif

#define NR_PORT			1
#define PINS_PER_PORT		32

#define pin2port(pin)		((pin) / PINS_PER_PORT)
#define pin2portpin(pin)	((pin) % PINS_PER_PORT)
#define port2reg(port)		((reg_t *)((((port) * WORD_SIZE) << 8) + PORTA))

static inline void ret_from_exti(unsigned int n)
{
}

static inline int gpio2exti(int n)
{
	return 0;
}

static inline void ret_from_gpio_int(unsigned int n)
{
}

static inline unsigned int scan_port(reg_t *reg)
{
	return 0;
}

static inline void write_port(reg_t *reg, unsigned int data)
{
}

static inline void write_port_pin(reg_t *reg, int pin, bool on)
{
}

void set_port_pin_conf(reg_t *reg, int pin, int mode);
void set_port_pin_conf_alt(reg_t *reg, int pin, int mode);
int reg2port(reg_t *reg);

#endif /* __NRF5_GPIO_H__ */

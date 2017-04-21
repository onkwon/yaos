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

#endif /* __NRF5_GPIO_H__ */

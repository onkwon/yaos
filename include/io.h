#ifndef __IO_H__
#define __IO_H__

#include "types.h"
#ifndef __MACHINE_TYPE__
#include "stm32.h"
#endif

#define MASK(v, mask)			((v) & (mask))
#define MASK_RESET(v, mask)		((v) & ~(mask))
#define MASK_SET(v, mask)		((v) | (mask))

#define sbi(v, bit)			(v |= (1 << (bit)))
#define cbi(v, bit)			(v &= ~(1 << (bit)))
#define gbi(v, bit)			(((v) >> (bit)) & 1)

/* Use these macros where needs atomic operation. */
#define GET_BITBAND_ADDR(base, offset, bit) \
		((base) + ((offset) << 5) + ((bit) << 2))
#define GET_BITBAND(addr, bit) \
		GET_BITBAND_ADDR(((unsigned)(addr) & 0xf0000000) + 0x02000000, \
				((unsigned)(addr) & 0xfffff), bit)
#define BITBAND(addr, bit, v) \
		(*(volatile unsigned *)GET_BITBAND(addr, bit) = v)

/* RCC */
#define SET_CLOCK_APB2(on, pin)		BITBAND(&RCC_APB2ENR, pin, on)
#define SET_CLOCK_APB1(on, pin)		BITBAND(&RCC_APB1ENR, pin, on)
#define RESET_CLOCK_APB2(pin) { \
		BITBAND(&RCC_APB2RSTR, pin, ON); \
		BITBAND(&RCC_APB2RSTR, pin, OFF); \
	}
#define RESET_CLOCK_APB1(pin) { \
		BITBAND(&RCC_APB1RSTR, pin, ON); \
		BITBAND(&RCC_APB1RSTR, pin, OFF); \
	}
#define reset()	(SCB_AIRCR = (SCB_AIRCR & ~0xffff0004) | 0x05fa0004)

unsigned get_sysclk();
unsigned get_hclk  (unsigned sysclk);
unsigned get_pclk1 (unsigned hclk);
unsigned get_pclk2 (unsigned hclk);
unsigned get_adclk (unsigned pclk2);
unsigned get_stkclk(unsigned hclk);

/* GPIO */
#define SET_PORT_PIN(port, pin, mode) ( \
		*(volatile unsigned *)(port + (pin / 8 * 4)) = \
		MASK_RESET(*(volatile unsigned *)(port + (pin / 8 * 4)), \
			0xf << ((pin % 8) * 4)) \
		| ((mode) << ((pin % 8) * 4)) \
	)
#define SET_PORT_CLOCK(on, port)	SET_CLOCK_APB2(on, (port >> 10) & 0xf)
#define GET_PORT(port)			(*(volatile unsigned *)((port) + 8))
#define PUT_PORT(port, data)		(*(volatile unsigned *)((port) + 0xc) = data)
#define PUT_PORT_PIN(port, pin, on) \
	(*(volatile unsigned *)((port) + 0x10) = on? 1 << pin : 1 << (pin + 16))

/* Interrupt */
#include "interrupt.h"

/* Systick */
#define SYSTICK(on)	/* ON or OFF [| SYSTICKINT] */ \
		STK_CTRL = (STK_CTRL & ~3) | (on)
#define SYSTICK_FLAG()			(STK_CTRL & 0x10000)
#define SET_SYSTICK(v)			(STK_LOAD = v)
#define GET_SYSTICK()			(STK_VAL)
#define RESET_SYSTICK()			(STK_VAL  = 0)
#define SYSTICK_INT			2
#define SYSTICK_MAX			((1 << 24) - 1) /* 24-bit timer */

/* Embedded flash */
#define FLASH_WRITE_START()	(FLASH_CR |=   1 << PG)
#define FLASH_WRITE_END()	(FLASH_CR &= ~(1 << PG))
#define FLASH_WRITE_WORD(addr, data)	{ \
		*(volatile unsigned short int *)addr = (unsigned short int)data; \
		while (FLASH_SR & 1); /* Check BSY bit, need timeout */ \
		*(volatile unsigned short int *)(addr+2) = (unsigned short int)(data >> 16); \
		while (FLASH_SR & 1); /* Check BSY bit, need timeout */ \
	}
#define flash_lock()	(FLASH_CR |= 0x80)
#define flash_unlock()	{ \
		FLASH_KEYR = KEY1; \
		FLASH_KEYR = KEY2; \
	}
		
#endif /* __IO_H__ */

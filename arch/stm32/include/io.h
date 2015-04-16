#ifndef __STM32_IO_H__
#define __STM32_IO_H__

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

/* Embedded flash */
#define FLASH_WRITE_START()	(FLASH_CR |=   1 << PG)
#define FLASH_WRITE_END()	(FLASH_CR &= ~(1 << PG))
#define FLASH_WRITE_WORD(addr, data)	{ \
		*(volatile unsigned short int *)addr = (unsigned short int)data; \
		while (FLASH_SR & 1); /* Check BSY bit, need timeout */ \
		*(volatile unsigned short int *)(addr+2) = (unsigned short int)(data >> 16); \
		while (FLASH_SR & 1); /* Check BSY bit, need timeout */ \
	}
#define FLASH_LOCK()	(FLASH_CR |= 0x80)
#define FLASH_UNLOCK() { \
		FLASH_KEYR = KEY1; \
		FLASH_KEYR = KEY2; \
	}
		
/* Reset and Clock Control */
#define RCC_BASE		(0x40021000)
#define RCC_CR			(*(volatile unsigned *)RCC_BASE)
#define RCC_CFGR		(*(volatile unsigned *)(RCC_BASE + 4))
#define RCC_CIR 		(*(volatile unsigned *)(RCC_BASE + 8))
#define RCC_APB2RSTR		(*(volatile unsigned *)(RCC_BASE + 0xc))
#define RCC_APB1RSTR		(*(volatile unsigned *)(RCC_BASE + 0x10))
#define RCC_AHBENR		(*(volatile unsigned *)(RCC_BASE + 0x14))
#define RCC_APB2ENR		(*(volatile unsigned *)(RCC_BASE + 0x18))
#define RCC_APB1ENR		(*(volatile unsigned *)(RCC_BASE + 0x1c))
#define RCC_CSR			(*(volatile unsigned *)(RCC_BASE + 0x24))

/* Embedded Flash memory */
#define FLASH_BASEADDR		0x08000000
#define FLASH_SIZE		0x00080000	/* 512KB */
#define PAGESIZE		0x800		/*   2KB */
#define RDPRT			0x00a5
#define KEY1			0x45670123
#define KEY2			0xcdef89ab
#define FLASH_BASE		(0x40022000)
#define FLASH_ACR		(*(volatile unsigned *)FLASH_BASE)
#define FLASH_KEYR		(*(volatile unsigned *)(FLASH_BASE + 0x4))
#define FLASH_OPTKEYR		(*(volatile unsigned *)(FLASH_BASE + 0x8))
#define FLASH_SR		(*(volatile unsigned *)(FLASH_BASE + 0xc))
#define FLASH_CR		(*(volatile unsigned *)(FLASH_BASE + 0x10))
#define FLASH_AR		(*(volatile unsigned *)(FLASH_BASE + 0x14))
#define FLASH_OBR		(*(volatile unsigned *)(FLASH_BASE + 0x1c))
#define FLASH_WRPR		(*(volatile unsigned *)(FLASH_BASE + 0x20))

/* GPIO */
#define PIN_INPUT		0x0 /* mode */
#define PIN_OUTPUT_10MHZ	0x1
#define PIN_OUTPUT_2MHZ		0x2
#define PIN_OUTPUT_50MHZ	0x3
#define PIN_ANALOG		0x0 /* input */
#define PIN_FLOATING		0x4
#define PIN_PULL		0x8
#define PIN_GENERAL		0x00 /* output */
#define PIN_OPENDRAIN		0x4
#define PIN_ALT			0x8
#define PIN_ALT_OPENDRAIN	0xc

#define PORTA			PORTA_BASE
#define PORTB			PORTB_BASE
#define PORTC			PORTC_BASE
#define PORTD			PORTD_BASE
#define PORTE			PORTE_BASE
#define PORTF			PORTF_BASE
#define PORTG			PORTG_BASE
#define PORTA_BASE		(0x40010800)
#define PORTB_BASE		(0x40010c00)
#define PORTC_BASE		(0x40011000)
#define PORTD_BASE		(0x40011400)
#define PORTE_BASE		(0x40011800)
#define PORTF_BASE		(0x40011c00)
#define PORTG_BASE		(0x40012000)

#define PORTA_LCKR		(*(volatile unsigned *)(PORTA_BASE + 0x18))
#define PORTB_LCKR		(*(volatile unsigned *)(PORTB_BASE + 0x18))
#define PORTC_LCKR		(*(volatile unsigned *)(PORTC_BASE + 0x18))
#define PORTD_LCKR		(*(volatile unsigned *)(PORTD_BASE + 0x18))
#define PORTE_LCKR		(*(volatile unsigned *)(PORTE_BASE + 0x18))
#define PORTF_LCKR		(*(volatile unsigned *)(PORTF_BASE + 0x18))
#define PORTG_LCKR		(*(volatile unsigned *)(PORTG_BASE + 0x18))

/* AFIO */
#define AFIO_BASE		(0x40010000)
#define AFIO_EXTICR1		(*(volatile unsigned *)(AFIO_BASE + 8))
#define AFIO_EXTICR2		(*(volatile unsigned *)(AFIO_BASE + 0xc))
#define AFIO_EXTICR3		(*(volatile unsigned *)(AFIO_BASE + 0x10))
#define AFIO_EXTICR4		(*(volatile unsigned *)(AFIO_BASE + 0x14))

/* USART */
#define USART1			(0x40013800)
#define USART2			(0x40004400)
#define USART3			(0x40004800)
#define UART4			(0x40004c00)
#define UART5			(0x40005000)

/* NVIC, Cortex-M3's internal peripherals */
#define NVIC_BASE		(0xE000E100)
#define NVIC_ISER0		(*(volatile unsigned *)0xE000E100)
#define NVIC_ISER1		(*(volatile unsigned *)0xE000E104)
#define NVIC_ISER2		(*(volatile unsigned *)0xE000E108)
#define NVIC_ICER		(*(volatile unsigned *)0xE000E180)
#define NVIC_ISPR		(*(volatile unsigned *)0xE000E200)
#define NVIC_ICPR		(*(volatile unsigned *)0xE000E280)
#define NVIC_IABR		(*(volatile unsigned *)0xE000E300)
#define NVIC_IPR		(*(volatile unsigned *)0xE000E400)
#define NVIC_STIR		(*(volatile unsigned *)(NVIC_BASE + 0xe00))
		
/* System Control Block(SCB), Cortex-M3's internal peripherals */
#define SCB_BASE		(0xE000ED00)
#define SCB_ICSR		(*(volatile unsigned *)(SCB_BASE + 4))
#define SCB_VTOR		(*(volatile unsigned *)(SCB_BASE + 8))
#define SCB_AIRCR		(*(volatile unsigned *)(SCB_BASE + 0xc))
#define SCB_CCR 		(*(volatile unsigned *)(SCB_BASE + 0x14))
#define SCB_SHCSR		(*(volatile unsigned *)(SCB_BASE + 0x24))

/* Systick */
#define SYSTICK_BASE		(0xe000e010)
#define STK_CTRL		(*(volatile unsigned *)SYSTICK_BASE)
#define STK_LOAD		(*(volatile unsigned *)(SYSTICK_BASE + 4))
#define STK_VAL 		(*(volatile unsigned *)(SYSTICK_BASE + 8))
#define STK_CALIB		(*(volatile unsigned *)(SYSTICK_BASE + 0xc))

/* Timers */
#define TIM2_BASE		(0x40000000)
#define TIM2_CR1		(*(volatile unsigned *)TIM2_BASE)
#define TIM2_CR2		(*(volatile unsigned *)(TIM2_BASE + 4))
#define TIM2_SMCR		(*(volatile unsigned *)(TIM2_BASE + 8))
#define TIM2_DIER		(*(volatile unsigned *)(TIM2_BASE + 0xc))
#define TIM2_SR 		(*(volatile unsigned *)(TIM2_BASE + 0x10))
#define TIM2_EGR		(*(volatile unsigned *)(TIM2_BASE + 0x14))
#define TIM2_CCMR1		(*(volatile unsigned *)(TIM2_BASE + 0x18))
#define TIM2_CCMR2		(*(volatile unsigned *)(TIM2_BASE + 0x1c))
#define TIM2_CCER		(*(volatile unsigned *)(TIM2_BASE + 0x20))
#define TIM2_CNT		(*(volatile unsigned *)(TIM2_BASE + 0x24))
#define TIM2_PSC		(*(volatile unsigned *)(TIM2_BASE + 0x28))
#define TIM2_ARR		(*(volatile unsigned *)(TIM2_BASE + 0x2c))
#define TIM2_CCR1		(*(volatile unsigned *)(TIM2_BASE + 0x34))
#define TIM2_CCR2		(*(volatile unsigned *)(TIM2_BASE + 0x38))
#define TIM2_CCR3		(*(volatile unsigned *)(TIM2_BASE + 0x3c))
#define TIM2_CCR4		(*(volatile unsigned *)(TIM2_BASE + 0x40))

#define TIM3_BASE		(0x40000400)
#define TIM3_CR1		(*(volatile unsigned *)TIM3_BASE)
#define TIM3_CR2		(*(volatile unsigned *)(TIM3_BASE + 4))
#define TIM3_SMCR		(*(volatile unsigned *)(TIM3_BASE + 8))
#define TIM3_DIER		(*(volatile unsigned *)(TIM3_BASE + 0xc))
#define TIM3_SR 		(*(volatile unsigned *)(TIM3_BASE + 0x10))
#define TIM3_EGR		(*(volatile unsigned *)(TIM3_BASE + 0x14))
#define TIM3_CCMR1		(*(volatile unsigned *)(TIM3_BASE + 0x18))
#define TIM3_CCMR2		(*(volatile unsigned *)(TIM3_BASE + 0x1c))
#define TIM3_CCER		(*(volatile unsigned *)(TIM3_BASE + 0x20))
#define TIM3_CNT		(*(volatile unsigned *)(TIM3_BASE + 0x24))
#define TIM3_PSC		(*(volatile unsigned *)(TIM3_BASE + 0x28))
#define TIM3_ARR		(*(volatile unsigned *)(TIM3_BASE + 0x2c))
#define TIM3_CCR1		(*(volatile unsigned *)(TIM3_BASE + 0x34))
#define TIM3_CCR2		(*(volatile unsigned *)(TIM3_BASE + 0x38))
#define TIM3_CCR3		(*(volatile unsigned *)(TIM3_BASE + 0x3c))
#define TIM3_CCR4		(*(volatile unsigned *)(TIM3_BASE + 0x40))

/* ADC */
#define ADC1_BASE		(0x40012400)
#define ADC1_SR			(*(volatile unsigned *)ADC1_BASE)
#define ADC1_CR1		(*(volatile unsigned *)(ADC1_BASE + 4))
#define ADC1_CR2		(*(volatile unsigned *)(ADC1_BASE + 8))
#define ADC1_SMPR1		(*(volatile unsigned *)(ADC1_BASE + 0xc))
#define ADC1_SMPR2		(*(volatile unsigned *)(ADC1_BASE + 0x10))
#define ADC1_SQR1		(*(volatile unsigned *)(ADC1_BASE + 0x2c))
#define ADC1_SQR2		(*(volatile unsigned *)(ADC1_BASE + 0x30))
#define ADC1_SQR3		(*(volatile unsigned *)(ADC1_BASE + 0x34))
#define ADC1_DR 		(*(volatile unsigned *)(ADC1_BASE + 0x4c))

/* DAC */
#define DAC_BASE		(0x40007400)
#define DAC_CR			(*(volatile unsigned *)DAC_BASE)
#define DAC_DHR8R2		(*(volatile unsigned *)(DAC_BASE + 0x1c))

#endif /* __STM32_IO_H__ */

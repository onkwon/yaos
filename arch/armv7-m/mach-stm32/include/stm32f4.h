#ifndef __STM32F4_IO_H__
#define __STM32F4_IO_H__

#include <types.h>

/* GPIO */
#define SET_PORT_PIN(port, pin, mode) ( \
	*(volatile unsigned int *)((port) + ((pin) / 16 * WORD_SIZE)) = \
		MASK_RESET(*(volatile unsigned int *) \
			((port) + ((pin) / 16 * WORD_SIZE)), \
			0x3 << (((pin) % 16) * 2)) \
		| ((mode) << (((pin) % 16) * 2)) \
)
#define SET_PORT_ALT(port, pin, alt) ( \
	*(volatile unsigned int *)((port) + 0x20 + ((pin) / 8 * WORD_SIZE)) \
		= MASK_RESET(*(volatile unsigned int *) \
			((port) + 0x20 + ((pin) / 8 * WORD_SIZE)), \
			0xf << (((pin) % 8) * 4)) \
		| ((alt) << (((pin) % 8) * 4)) \
)
#define SET_PORT_CLOCK(on, port)	\
	SET_CLOCK_AHB1(on, (port >> 10) & 0xf)
#define GET_PORT(port)			\
	(*(volatile unsigned int *)((port) + 0x10))
#define PUT_PORT(port, data)		\
	(*(volatile unsigned int *)((port) + 0x14) = data)
#define PUT_PORT_PIN(port, pin, on) \
	(*(volatile unsigned int *)((port) + 0x18) = \
		(on)? 1 << (pin) : 1 << ((pin) + 16))

/* Reset and Clock Control */
#define RCC_BASE		(0x40023800)
#define RCC_CR			(*(volatile unsigned int *)RCC_BASE)
#define RCC_PLLCFGR		(*(volatile unsigned int *)(RCC_BASE + 4))
#define RCC_CFGR 		(*(volatile unsigned int *)(RCC_BASE + 8))
#define RCC_CIR			(*(volatile unsigned int *)(RCC_BASE + 0xc))
#define RCC_AHB1RSTR		(*(volatile unsigned int *)(RCC_BASE + 0x10))
#define RCC_AHB2RSTR		(*(volatile unsigned int *)(RCC_BASE + 0x14))
#define RCC_AHB3RSTR		(*(volatile unsigned int *)(RCC_BASE + 0x18))
#define RCC_APB1RSTR		(*(volatile unsigned int *)(RCC_BASE + 0x20))
#define RCC_APB2RSTR		(*(volatile unsigned int *)(RCC_BASE + 0x24))
#define RCC_AHB1ENR		(*(volatile unsigned int *)(RCC_BASE + 0x30))
#define RCC_AHB2ENR		(*(volatile unsigned int *)(RCC_BASE + 0x34))
#define RCC_AHB3ENR		(*(volatile unsigned int *)(RCC_BASE + 0x38))
#define RCC_APB1ENR		(*(volatile unsigned int *)(RCC_BASE + 0x40))
#define RCC_APB2ENR		(*(volatile unsigned int *)(RCC_BASE + 0x44))
#define RCC_AHB1LPENR		(*(volatile unsigned int *)(RCC_BASE + 0x50))
#define RCC_AHB2LPENR		(*(volatile unsigned int *)(RCC_BASE + 0x54))
#define RCC_AHB3LPENR		(*(volatile unsigned int *)(RCC_BASE + 0x58))
#define RCC_APB1LPENR		(*(volatile unsigned int *)(RCC_BASE + 0x60))
#define RCC_APB2LPENR		(*(volatile unsigned int *)(RCC_BASE + 0x64))
#define RCC_BDCR		(*(volatile unsigned int *)(RCC_BASE + 0x70))
#define RCC_CSR			(*(volatile unsigned int *)(RCC_BASE + 0x74))
#define RCC_SSCGR		(*(volatile unsigned int *)(RCC_BASE + 0x80))
#define RCC_PLLI2SCFGR		(*(volatile unsigned int *)(RCC_BASE + 0x84))

/* Embedded Flash memory */
#define FLASH_BASE		(0x40023c00)
#define FLASH_ACR		(*(volatile unsigned int *)FLASH_BASE)
#define FLASH_KEYR		(*(volatile unsigned int *)(FLASH_BASE + 0x4))
#define FLASH_OPTKEYR		(*(volatile unsigned int *)(FLASH_BASE + 0x8))
#define FLASH_SR		(*(volatile unsigned int *)(FLASH_BASE + 0xc))
#define FLASH_CR		(*(volatile unsigned int *)(FLASH_BASE + 0x10))
#define FLASH_OPTCR		(*(volatile unsigned int *)(FLASH_BASE + 0x14))

#define FLASH_OPT_BASE		(0x1fffc000)
#define FLASH_OPT_RDP		(*(volatile unsigned short int *)FLASH_OPT_BASE)

/* GPIO */
#define PIN_INPUT		0x0 /* mode */
#define PIN_OUTPUT		0x1
#define PIN_ALT			0x2
#define PIN_ANALOG		0x3

#define PORTA			PORTA_BASE
#define PORTB			PORTB_BASE
#define PORTC			PORTC_BASE
#define PORTD			PORTD_BASE
#define PORTE			PORTE_BASE
#define PORTF			PORTF_BASE
#define PORTG			PORTG_BASE
#define PORTH			PORTH_BASE
#define PORTI			PORTI_BASE
#define PORTA_BASE		(0x40020000)
#define PORTB_BASE		(0x40020400)
#define PORTC_BASE		(0x40020800)
#define PORTD_BASE		(0x40020c00)
#define PORTE_BASE		(0x40021000)
#define PORTF_BASE		(0x40021400)
#define PORTG_BASE		(0x40021800)
#define PORTH_BASE		(0x40021c00)
#define PORTI_BASE		(0x40022000)

#define PORTA_LCKR		(*(volatile unsigned int *)(PORTA_BASE + 0x1c))
#define PORTB_LCKR		(*(volatile unsigned int *)(PORTB_BASE + 0x1c))
#define PORTC_LCKR		(*(volatile unsigned int *)(PORTC_BASE + 0x1c))
#define PORTD_LCKR		(*(volatile unsigned int *)(PORTD_BASE + 0x1c))
#define PORTE_LCKR		(*(volatile unsigned int *)(PORTE_BASE + 0x1c))
#define PORTF_LCKR		(*(volatile unsigned int *)(PORTF_BASE + 0x1c))
#define PORTG_LCKR		(*(volatile unsigned int *)(PORTG_BASE + 0x1c))

/* SYSCFG */
#define SYSCFG_BASE		(0x40013800)
#define SYSCFG_MEMRMP		(*(volatile unsigned int *)(SYSCFG_BASE + 0x00))
#define SYSCFG_PMC		(*(volatile unsigned int *)(SYSCFG_BASE + 0x04))
#define SYSCFG_EXTICR1		(*(volatile unsigned int *)(SYSCFG_BASE + 0x08))
#define SYSCFG_EXTICR2		(*(volatile unsigned int *)(SYSCFG_BASE + 0x0c))
#define SYSCFG_EXTICR3		(*(volatile unsigned int *)(SYSCFG_BASE + 0x10))
#define SYSCFG_EXTICR4		(*(volatile unsigned int *)(SYSCFG_BASE + 0x14))
#define SYSCFG_CMPCR		(*(volatile unsigned int *)(SYSCFG_BASE + 0x20))

/* EXTI */
#define EXTI_BASE		(0x40013c00)
#define EXTI_IMR		(*(volatile unsigned int *)(EXTI_BASE + 0))
#define EXTI_EMR		(*(volatile unsigned int *)(EXTI_BASE + 4))
#define EXTI_RTSR		(*(volatile unsigned int *)(EXTI_BASE + 8))
#define EXTI_FTSR		(*(volatile unsigned int *)(EXTI_BASE + 0xc))
#define EXTI_PR			(*(volatile unsigned int *)(EXTI_BASE + 0x14))

/* USART */
#define USART1			(0x40011000)
#define USART2			(0x40004400)
#define USART3			(0x40004800)
#define UART4			(0x40004c00)
#define UART5			(0x40005000)
#define USART6			(0x40011400)

/* I2C1 */
#define I2C1_BASE		(0x40005400)
#define I2C1_CR1		(*(volatile unsigned int *)(I2C1_BASE + 0))
#define I2C1_CR2		(*(volatile unsigned int *)(I2C1_BASE + 4))
#define I2C1_OAR1		(*(volatile unsigned int *)(I2C1_BASE + 8))
#define I2C1_OAR2		(*(volatile unsigned int *)(I2C1_BASE + 0xc))
#define I2C1_DR			(*(volatile unsigned int *)(I2C1_BASE + 0x10))
#define I2C1_SR1		(*(volatile unsigned int *)(I2C1_BASE + 0x14))
#define I2C1_SR2		(*(volatile unsigned int *)(I2C1_BASE + 0x18))
#define I2C1_CCR		(*(volatile unsigned int *)(I2C1_BASE + 0x1c))
#define I2C1_TRISE		(*(volatile unsigned int *)(I2C1_BASE + 0x20))

/* Timers */
#define TIM1_BASE		(0x40010000)
#define TIM1_CR1		(*(volatile unsigned int *)TIM2_BASE)
#define TIM1_CR2		(*(volatile unsigned int *)(TIM2_BASE + 4))
#define TIM1_SMCR		(*(volatile unsigned int *)(TIM2_BASE + 8))
#define TIM1_DIER		(*(volatile unsigned int *)(TIM2_BASE + 0xc))
#define TIM1_SR 		(*(volatile unsigned int *)(TIM2_BASE + 0x10))
#define TIM1_EGR		(*(volatile unsigned int *)(TIM2_BASE + 0x14))
#define TIM1_CCMR1		(*(volatile unsigned int *)(TIM2_BASE + 0x18))
#define TIM1_CCMR2		(*(volatile unsigned int *)(TIM2_BASE + 0x1c))
#define TIM1_CCER		(*(volatile unsigned int *)(TIM2_BASE + 0x20))
#define TIM1_CNT		(*(volatile unsigned int *)(TIM2_BASE + 0x24))
#define TIM1_PSC		(*(volatile unsigned int *)(TIM2_BASE + 0x28))
#define TIM1_ARR		(*(volatile unsigned int *)(TIM2_BASE + 0x2c))
#define TIM1_RCR		(*(volatile unsigned int *)(TIM2_BASE + 0x30))
#define TIM1_CCR1		(*(volatile unsigned int *)(TIM2_BASE + 0x34))
#define TIM1_CCR2		(*(volatile unsigned int *)(TIM2_BASE + 0x38))
#define TIM1_CCR3		(*(volatile unsigned int *)(TIM2_BASE + 0x3c))
#define TIM1_CCR4		(*(volatile unsigned int *)(TIM2_BASE + 0x40))
#define TIM1_BDTR		(*(volatile unsigned int *)(TIM2_BASE + 0x44))
#define TIM1_DCR		(*(volatile unsigned int *)(TIM2_BASE + 0x48))
#define TIM1_DMAR		(*(volatile unsigned int *)(TIM2_BASE + 0x4c))

#define TIM2_BASE		(0x40000000)
#define TIM2_CR1		(*(volatile unsigned int *)TIM2_BASE)
#define TIM2_CR2		(*(volatile unsigned int *)(TIM2_BASE + 4))
#define TIM2_SMCR		(*(volatile unsigned int *)(TIM2_BASE + 8))
#define TIM2_DIER		(*(volatile unsigned int *)(TIM2_BASE + 0xc))
#define TIM2_SR 		(*(volatile unsigned int *)(TIM2_BASE + 0x10))
#define TIM2_EGR		(*(volatile unsigned int *)(TIM2_BASE + 0x14))
#define TIM2_CCMR1		(*(volatile unsigned int *)(TIM2_BASE + 0x18))
#define TIM2_CCMR2		(*(volatile unsigned int *)(TIM2_BASE + 0x1c))
#define TIM2_CCER		(*(volatile unsigned int *)(TIM2_BASE + 0x20))
#define TIM2_CNT		(*(volatile unsigned int *)(TIM2_BASE + 0x24))
#define TIM2_PSC		(*(volatile unsigned int *)(TIM2_BASE + 0x28))
#define TIM2_ARR		(*(volatile unsigned int *)(TIM2_BASE + 0x2c))
#define TIM2_CCR1		(*(volatile unsigned int *)(TIM2_BASE + 0x34))
#define TIM2_CCR2		(*(volatile unsigned int *)(TIM2_BASE + 0x38))
#define TIM2_CCR3		(*(volatile unsigned int *)(TIM2_BASE + 0x3c))
#define TIM2_CCR4		(*(volatile unsigned int *)(TIM2_BASE + 0x40))
#define TIM2_DCR		(*(volatile unsigned int *)(TIM2_BASE + 0x48))
#define TIM2_DMAR		(*(volatile unsigned int *)(TIM2_BASE + 0x4c))
#define TIM2_OR			(*(volatile unsigned int *)(TIM2_BASE + 0x50))

#define TIM3_BASE		(0x40000400)
#define TIM3_CR1		(*(volatile unsigned int *)TIM3_BASE)
#define TIM3_CR2		(*(volatile unsigned int *)(TIM3_BASE + 4))
#define TIM3_SMCR		(*(volatile unsigned int *)(TIM3_BASE + 8))
#define TIM3_DIER		(*(volatile unsigned int *)(TIM3_BASE + 0xc))
#define TIM3_SR 		(*(volatile unsigned int *)(TIM3_BASE + 0x10))
#define TIM3_EGR		(*(volatile unsigned int *)(TIM3_BASE + 0x14))
#define TIM3_CCMR1		(*(volatile unsigned int *)(TIM3_BASE + 0x18))
#define TIM3_CCMR2		(*(volatile unsigned int *)(TIM3_BASE + 0x1c))
#define TIM3_CCER		(*(volatile unsigned int *)(TIM3_BASE + 0x20))
#define TIM3_CNT		(*(volatile unsigned int *)(TIM3_BASE + 0x24))
#define TIM3_PSC		(*(volatile unsigned int *)(TIM3_BASE + 0x28))
#define TIM3_ARR		(*(volatile unsigned int *)(TIM3_BASE + 0x2c))
#define TIM3_CCR1		(*(volatile unsigned int *)(TIM3_BASE + 0x34))
#define TIM3_CCR2		(*(volatile unsigned int *)(TIM3_BASE + 0x38))
#define TIM3_CCR3		(*(volatile unsigned int *)(TIM3_BASE + 0x3c))
#define TIM3_CCR4		(*(volatile unsigned int *)(TIM3_BASE + 0x40))

/* ADC */
#define ADC_BASE		(0x40012300)
#define ADC_CSR			(*(volatile unsigned int *)ADC_BASE)
#define ADC_CCR			(*(volatile unsigned int *)(ADC_BASE + 4))
#define ADC_CDR			(*(volatile unsigned int *)(ADC_BASE + 8))

#define ADC1_BASE		(0x40012000)
#define ADC1_SR			(*(volatile unsigned int *)ADC1_BASE)
#define ADC1_CR1		(*(volatile unsigned int *)(ADC1_BASE + 4))
#define ADC1_CR2		(*(volatile unsigned int *)(ADC1_BASE + 8))
#define ADC1_SMPR1		(*(volatile unsigned int *)(ADC1_BASE + 0xc))
#define ADC1_SMPR2		(*(volatile unsigned int *)(ADC1_BASE + 0x10))
#define ADC1_JOFR1		(*(volatile unsigned int *)(ADC1_BASE + 0x14))
#define ADC1_JOFR2		(*(volatile unsigned int *)(ADC1_BASE + 0x18))
#define ADC1_JOFR3		(*(volatile unsigned int *)(ADC1_BASE + 0x1c))
#define ADC1_JOFR4		(*(volatile unsigned int *)(ADC1_BASE + 0x20))
#define ADC1_HTR		(*(volatile unsigned int *)(ADC1_BASE + 0x24))
#define ADC1_LTR		(*(volatile unsigned int *)(ADC1_BASE + 0x28))
#define ADC1_SQR1		(*(volatile unsigned int *)(ADC1_BASE + 0x2c))
#define ADC1_SQR2		(*(volatile unsigned int *)(ADC1_BASE + 0x30))
#define ADC1_SQR3		(*(volatile unsigned int *)(ADC1_BASE + 0x34))
#define ADC1_JSQR		(*(volatile unsigned int *)(ADC1_BASE + 0x38))
#define ADC1_JDR1		(*(volatile unsigned int *)(ADC1_BASE + 0x3c))
#define ADC1_JDR2		(*(volatile unsigned int *)(ADC1_BASE + 0x40))
#define ADC1_JDR3		(*(volatile unsigned int *)(ADC1_BASE + 0x44))
#define ADC1_JDR4		(*(volatile unsigned int *)(ADC1_BASE + 0x48))
#define ADC1_DR 		(*(volatile unsigned int *)(ADC1_BASE + 0x4c))

/* DAC */
#define DAC_BASE		(0x40007400)
#define DAC_CR			(*(volatile unsigned int *)DAC_BASE)
#define DAC_DHR8R2		(*(volatile unsigned int *)(DAC_BASE + 0x1c))

/* POWER */
#define PWR_BASE		(0x40007000)
#define PWR_CR			(*(volatile unsigned int *)PWR_BASE)
#define PWR_CSR			(*(volatile unsigned int *)(PWR_BASE + 0x04))

#endif /* __STM32F4_IO_H__ */

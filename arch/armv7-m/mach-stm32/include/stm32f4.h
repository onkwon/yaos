#ifndef __STM32F4_IO_H__
#define __STM32F4_IO_H__

#include <types.h>

/* Reset and Clock Control */
#define RCC_BASE		(0x40023800)
#define RCC_CR			(*(reg_t *)RCC_BASE)
#define RCC_PLLCFGR		(*(reg_t *)(RCC_BASE + 4))
#define RCC_CFGR 		(*(reg_t *)(RCC_BASE + 8))
#define RCC_CIR			(*(reg_t *)(RCC_BASE + 0xc))
#define RCC_AHB1RSTR		(*(reg_t *)(RCC_BASE + 0x10))
#define RCC_AHB2RSTR		(*(reg_t *)(RCC_BASE + 0x14))
#define RCC_AHB3RSTR		(*(reg_t *)(RCC_BASE + 0x18))
#define RCC_APB1RSTR		(*(reg_t *)(RCC_BASE + 0x20))
#define RCC_APB2RSTR		(*(reg_t *)(RCC_BASE + 0x24))
#define RCC_AHB1ENR		(*(reg_t *)(RCC_BASE + 0x30))
#define RCC_AHB2ENR		(*(reg_t *)(RCC_BASE + 0x34))
#define RCC_AHB3ENR		(*(reg_t *)(RCC_BASE + 0x38))
#define RCC_APB1ENR		(*(reg_t *)(RCC_BASE + 0x40))
#define RCC_APB2ENR		(*(reg_t *)(RCC_BASE + 0x44))
#define RCC_AHB1LPENR		(*(reg_t *)(RCC_BASE + 0x50))
#define RCC_AHB2LPENR		(*(reg_t *)(RCC_BASE + 0x54))
#define RCC_AHB3LPENR		(*(reg_t *)(RCC_BASE + 0x58))
#define RCC_APB1LPENR		(*(reg_t *)(RCC_BASE + 0x60))
#define RCC_APB2LPENR		(*(reg_t *)(RCC_BASE + 0x64))
#define RCC_BDCR		(*(reg_t *)(RCC_BASE + 0x70))
#define RCC_CSR			(*(reg_t *)(RCC_BASE + 0x74))
#define RCC_SSCGR		(*(reg_t *)(RCC_BASE + 0x80))
#define RCC_PLLI2SCFGR		(*(reg_t *)(RCC_BASE + 0x84))

#define RCC_SYSCFGEN_BIT	14

/* Embedded Flash memory */
#define FLASH_BASE		(0x40023c00)
#define FLASH_ACR		(*(reg_t *)FLASH_BASE)
#define FLASH_KEYR		(*(reg_t *)(FLASH_BASE + 0x4))
#define FLASH_OPTKEYR		(*(reg_t *)(FLASH_BASE + 0x8))
#define FLASH_SR		(*(reg_t *)(FLASH_BASE + 0xc))
#define FLASH_CR		(*(reg_t *)(FLASH_BASE + 0x10))
#define FLASH_OPTCR		(*(reg_t *)(FLASH_BASE + 0x14))

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

#define PORTA_LCKR		(*(reg_t *)(PORTA_BASE + 0x1c))
#define PORTB_LCKR		(*(reg_t *)(PORTB_BASE + 0x1c))
#define PORTC_LCKR		(*(reg_t *)(PORTC_BASE + 0x1c))
#define PORTD_LCKR		(*(reg_t *)(PORTD_BASE + 0x1c))
#define PORTE_LCKR		(*(reg_t *)(PORTE_BASE + 0x1c))
#define PORTF_LCKR		(*(reg_t *)(PORTF_BASE + 0x1c))
#define PORTG_LCKR		(*(reg_t *)(PORTG_BASE + 0x1c))

/* SYSCFG */
#define SYSCFG_BASE		(0x40013800)
#define SYSCFG_MEMRMP		(*(reg_t *)(SYSCFG_BASE + 0x00))
#define SYSCFG_PMC		(*(reg_t *)(SYSCFG_BASE + 0x04))
#define SYSCFG_EXTICR1		(*(reg_t *)(SYSCFG_BASE + 0x08))
#define SYSCFG_EXTICR2		(*(reg_t *)(SYSCFG_BASE + 0x0c))
#define SYSCFG_EXTICR3		(*(reg_t *)(SYSCFG_BASE + 0x10))
#define SYSCFG_EXTICR4		(*(reg_t *)(SYSCFG_BASE + 0x14))
#define SYSCFG_CMPCR		(*(reg_t *)(SYSCFG_BASE + 0x20))

/* EXTI */
#define EXTI_BASE		(0x40013c00)
#define EXTI_IMR		(*(reg_t *)(EXTI_BASE + 0))
#define EXTI_EMR		(*(reg_t *)(EXTI_BASE + 4))
#define EXTI_RTSR		(*(reg_t *)(EXTI_BASE + 8))
#define EXTI_FTSR		(*(reg_t *)(EXTI_BASE + 0xc))
#define EXTI_PR			(*(reg_t *)(EXTI_BASE + 0x14))

/* USART */
#define USART1			(0x40011000)
#define USART2			(0x40004400)
#define USART3			(0x40004800)
#define UART4			(0x40004c00)
#define UART5			(0x40005000)
#define USART6			(0x40011400)

/* I2C1 */
#define I2C1_BASE		(0x40005400)
#define I2C1_CR1		(*(reg_t *)(I2C1_BASE + 0))
#define I2C1_CR2		(*(reg_t *)(I2C1_BASE + 4))
#define I2C1_OAR1		(*(reg_t *)(I2C1_BASE + 8))
#define I2C1_OAR2		(*(reg_t *)(I2C1_BASE + 0xc))
#define I2C1_DR			(*(reg_t *)(I2C1_BASE + 0x10))
#define I2C1_SR1		(*(reg_t *)(I2C1_BASE + 0x14))
#define I2C1_SR2		(*(reg_t *)(I2C1_BASE + 0x18))
#define I2C1_CCR		(*(reg_t *)(I2C1_BASE + 0x1c))
#define I2C1_TRISE		(*(reg_t *)(I2C1_BASE + 0x20))

/* Timers */
#define TIM1_BASE		(0x40010000)
#define TIM1_CR1		(*(reg_t *)TIM2_BASE)
#define TIM1_CR2		(*(reg_t *)(TIM2_BASE + 4))
#define TIM1_SMCR		(*(reg_t *)(TIM2_BASE + 8))
#define TIM1_DIER		(*(reg_t *)(TIM2_BASE + 0xc))
#define TIM1_SR 		(*(reg_t *)(TIM2_BASE + 0x10))
#define TIM1_EGR		(*(reg_t *)(TIM2_BASE + 0x14))
#define TIM1_CCMR1		(*(reg_t *)(TIM2_BASE + 0x18))
#define TIM1_CCMR2		(*(reg_t *)(TIM2_BASE + 0x1c))
#define TIM1_CCER		(*(reg_t *)(TIM2_BASE + 0x20))
#define TIM1_CNT		(*(reg_t *)(TIM2_BASE + 0x24))
#define TIM1_PSC		(*(reg_t *)(TIM2_BASE + 0x28))
#define TIM1_ARR		(*(reg_t *)(TIM2_BASE + 0x2c))
#define TIM1_RCR		(*(reg_t *)(TIM2_BASE + 0x30))
#define TIM1_CCR1		(*(reg_t *)(TIM2_BASE + 0x34))
#define TIM1_CCR2		(*(reg_t *)(TIM2_BASE + 0x38))
#define TIM1_CCR3		(*(reg_t *)(TIM2_BASE + 0x3c))
#define TIM1_CCR4		(*(reg_t *)(TIM2_BASE + 0x40))
#define TIM1_BDTR		(*(reg_t *)(TIM2_BASE + 0x44))
#define TIM1_DCR		(*(reg_t *)(TIM2_BASE + 0x48))
#define TIM1_DMAR		(*(reg_t *)(TIM2_BASE + 0x4c))

#define TIM2_BASE		(0x40000000)
#define TIM2_CR1		(*(reg_t *)TIM2_BASE)
#define TIM2_CR2		(*(reg_t *)(TIM2_BASE + 4))
#define TIM2_SMCR		(*(reg_t *)(TIM2_BASE + 8))
#define TIM2_DIER		(*(reg_t *)(TIM2_BASE + 0xc))
#define TIM2_SR 		(*(reg_t *)(TIM2_BASE + 0x10))
#define TIM2_EGR		(*(reg_t *)(TIM2_BASE + 0x14))
#define TIM2_CCMR1		(*(reg_t *)(TIM2_BASE + 0x18))
#define TIM2_CCMR2		(*(reg_t *)(TIM2_BASE + 0x1c))
#define TIM2_CCER		(*(reg_t *)(TIM2_BASE + 0x20))
#define TIM2_CNT		(*(reg_t *)(TIM2_BASE + 0x24))
#define TIM2_PSC		(*(reg_t *)(TIM2_BASE + 0x28))
#define TIM2_ARR		(*(reg_t *)(TIM2_BASE + 0x2c))
#define TIM2_CCR1		(*(reg_t *)(TIM2_BASE + 0x34))
#define TIM2_CCR2		(*(reg_t *)(TIM2_BASE + 0x38))
#define TIM2_CCR3		(*(reg_t *)(TIM2_BASE + 0x3c))
#define TIM2_CCR4		(*(reg_t *)(TIM2_BASE + 0x40))
#define TIM2_DCR		(*(reg_t *)(TIM2_BASE + 0x48))
#define TIM2_DMAR		(*(reg_t *)(TIM2_BASE + 0x4c))
#define TIM2_OR			(*(reg_t *)(TIM2_BASE + 0x50))

#define TIM3_BASE		(0x40000400)
#define TIM3_CR1		(*(reg_t *)TIM3_BASE)
#define TIM3_CR2		(*(reg_t *)(TIM3_BASE + 4))
#define TIM3_SMCR		(*(reg_t *)(TIM3_BASE + 8))
#define TIM3_DIER		(*(reg_t *)(TIM3_BASE + 0xc))
#define TIM3_SR 		(*(reg_t *)(TIM3_BASE + 0x10))
#define TIM3_EGR		(*(reg_t *)(TIM3_BASE + 0x14))
#define TIM3_CCMR1		(*(reg_t *)(TIM3_BASE + 0x18))
#define TIM3_CCMR2		(*(reg_t *)(TIM3_BASE + 0x1c))
#define TIM3_CCER		(*(reg_t *)(TIM3_BASE + 0x20))
#define TIM3_CNT		(*(reg_t *)(TIM3_BASE + 0x24))
#define TIM3_PSC		(*(reg_t *)(TIM3_BASE + 0x28))
#define TIM3_ARR		(*(reg_t *)(TIM3_BASE + 0x2c))
#define TIM3_CCR1		(*(reg_t *)(TIM3_BASE + 0x34))
#define TIM3_CCR2		(*(reg_t *)(TIM3_BASE + 0x38))
#define TIM3_CCR3		(*(reg_t *)(TIM3_BASE + 0x3c))
#define TIM3_CCR4		(*(reg_t *)(TIM3_BASE + 0x40))

/* ADC */
#define ADC_BASE		(0x40012300)
#define ADC_CSR			(*(reg_t *)ADC_BASE)
#define ADC_CCR			(*(reg_t *)(ADC_BASE + 4))
#define ADC_CDR			(*(reg_t *)(ADC_BASE + 8))

#define ADC1_BASE		(0x40012000)
#define ADC1_SR			(*(reg_t *)ADC1_BASE)
#define ADC1_CR1		(*(reg_t *)(ADC1_BASE + 4))
#define ADC1_CR2		(*(reg_t *)(ADC1_BASE + 8))
#define ADC1_SMPR1		(*(reg_t *)(ADC1_BASE + 0xc))
#define ADC1_SMPR2		(*(reg_t *)(ADC1_BASE + 0x10))
#define ADC1_JOFR1		(*(reg_t *)(ADC1_BASE + 0x14))
#define ADC1_JOFR2		(*(reg_t *)(ADC1_BASE + 0x18))
#define ADC1_JOFR3		(*(reg_t *)(ADC1_BASE + 0x1c))
#define ADC1_JOFR4		(*(reg_t *)(ADC1_BASE + 0x20))
#define ADC1_HTR		(*(reg_t *)(ADC1_BASE + 0x24))
#define ADC1_LTR		(*(reg_t *)(ADC1_BASE + 0x28))
#define ADC1_SQR1		(*(reg_t *)(ADC1_BASE + 0x2c))
#define ADC1_SQR2		(*(reg_t *)(ADC1_BASE + 0x30))
#define ADC1_SQR3		(*(reg_t *)(ADC1_BASE + 0x34))
#define ADC1_JSQR		(*(reg_t *)(ADC1_BASE + 0x38))
#define ADC1_JDR1		(*(reg_t *)(ADC1_BASE + 0x3c))
#define ADC1_JDR2		(*(reg_t *)(ADC1_BASE + 0x40))
#define ADC1_JDR3		(*(reg_t *)(ADC1_BASE + 0x44))
#define ADC1_JDR4		(*(reg_t *)(ADC1_BASE + 0x48))
#define ADC1_DR 		(*(reg_t *)(ADC1_BASE + 0x4c))

/* DAC */
#define DAC_BASE		(0x40007400)
#define DAC_CR			(*(reg_t *)DAC_BASE)
#define DAC_DHR8R2		(*(reg_t *)(DAC_BASE + 0x1c))

/* POWER */
#define PWR_BASE		(0x40007000)
#define PWR_CR			(*(reg_t *)PWR_BASE)
#define PWR_CSR			(*(reg_t *)(PWR_BASE + 0x04))

#endif /* __STM32F4_IO_H__ */

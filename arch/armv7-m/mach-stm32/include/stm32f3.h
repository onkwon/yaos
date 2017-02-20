#ifndef __STM32F3_IO_H__
#define __STM32F3_IO_H__

#include <types.h>

/* Reset and Clock Control */
#define RCC_BASE		(0x40021000)
#define RCC_CR			(*(reg_t *)RCC_BASE)
#define RCC_CFGR		(*(reg_t *)(RCC_BASE + 4))
#define RCC_CIR 		(*(reg_t *)(RCC_BASE + 8))
#define RCC_APB2RSTR		(*(reg_t *)(RCC_BASE + 0xc))
#define RCC_APB1RSTR		(*(reg_t *)(RCC_BASE + 0x10))
#define RCC_AHBENR		(*(reg_t *)(RCC_BASE + 0x14))
#define RCC_APB2ENR		(*(reg_t *)(RCC_BASE + 0x18))
#define RCC_APB1ENR		(*(reg_t *)(RCC_BASE + 0x1c))
#define RCC_BDCR		(*(reg_t *)(RCC_BASE + 0x20))
#define RCC_CSR			(*(reg_t *)(RCC_BASE + 0x24))
#define RCC_AHBRSTR		(*(reg_t *)(RCC_BASE + 0x28))
#define RCC_CFGR2		(*(reg_t *)(RCC_BASE + 0x2c))
#define RCC_CFGR3		(*(reg_t *)(RCC_BASE + 0x30))

#define RCC_SYSCFGEN_BIT	0

/* Embedded Flash memory */
#define FLASH_BASE		(0x40022000)
#define FLASH_ACR		(*(reg_t *)FLASH_BASE)
#define FLASH_KEYR		(*(reg_t *)(FLASH_BASE + 0x4))
#define FLASH_OPTKEYR		(*(reg_t *)(FLASH_BASE + 0x8))
#define FLASH_SR		(*(reg_t *)(FLASH_BASE + 0xc))
#define FLASH_CR		(*(reg_t *)(FLASH_BASE + 0x10))
#define FLASH_AR		(*(reg_t *)(FLASH_BASE + 0x14))
#define FLASH_OBR		(*(reg_t *)(FLASH_BASE + 0x1c))
#define FLASH_WRPR		(*(reg_t *)(FLASH_BASE + 0x20))

#define FLASH_OPT_BASE		(0x1ffff800)
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
#define PORTA_BASE		(0x48000000)
#define PORTB_BASE		(0x48000400)
#define PORTC_BASE		(0x48000800)
#define PORTD_BASE		(0x48000c00)
#define PORTE_BASE		(0x48001000)
#define PORTF_BASE		(0x48001400)

#define PORTA_LCKR		(*(reg_t *)(PORTA_BASE + 0x1c))
#define PORTB_LCKR		(*(reg_t *)(PORTB_BASE + 0x1c))
#define PORTC_LCKR		(*(reg_t *)(PORTC_BASE + 0x1c))
#define PORTD_LCKR		(*(reg_t *)(PORTD_BASE + 0x1c))
#define PORTE_LCKR		(*(reg_t *)(PORTE_BASE + 0x1c))
#define PORTF_LCKR		(*(reg_t *)(PORTF_BASE + 0x1c))
#define PORTG_LCKR		(*(reg_t *)(PORTG_BASE + 0x1c))

/* SYSCFG */
#define SYSCFG_BASE		(0x40010000)
#define SYSCFG_MEMRMP		(*(reg_t *)(SYSCFG_BASE + 0x00))
#define SYSCFG_PMC		(*(reg_t *)(SYSCFG_BASE + 0x04))
#define SYSCFG_EXTICR1		(*(reg_t *)(SYSCFG_BASE + 0x08))
#define SYSCFG_EXTICR2		(*(reg_t *)(SYSCFG_BASE + 0x0c))
#define SYSCFG_EXTICR3		(*(reg_t *)(SYSCFG_BASE + 0x10))
#define SYSCFG_EXTICR4		(*(reg_t *)(SYSCFG_BASE + 0x14))
#define SYSCFG_CMPCR		(*(reg_t *)(SYSCFG_BASE + 0x20))

/* EXTI */
#define EXTI_BASE		(0x40010400)
#define EXTI_IMR		(*(reg_t *)(EXTI_BASE + 0))
#define EXTI_EMR		(*(reg_t *)(EXTI_BASE + 4))
#define EXTI_RTSR		(*(reg_t *)(EXTI_BASE + 8))
#define EXTI_FTSR		(*(reg_t *)(EXTI_BASE + 0xc))
#define EXTI_PR			(*(reg_t *)(EXTI_BASE + 0x14))

/* USART */
#define USART1			(0x40013800)
#define USART2			(0x40004400)
#define USART3			(0x40004800)

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

#define TIM14_BASE		(0x40002000)
#define TIM14_CR1		(*(reg_t *)TIM14_BASE)
#define TIM14_SMCR		(*(reg_t *)(TIM14_BASE + 0x08))
#define TIM14_DIER		(*(reg_t *)(TIM14_BASE + 0x0c))
#define TIM14_SR		(*(reg_t *)(TIM14_BASE + 0x10))
#define TIM14_EGR		(*(reg_t *)(TIM14_BASE + 0x14))
#define TIM14_CCMR		(*(reg_t *)(TIM14_BASE + 0x18))
#define TIM14_CCER		(*(reg_t *)(TIM14_BASE + 0x20))
#define TIM14_CNT		(*(reg_t *)(TIM14_BASE + 0x24))
#define TIM14_PSC		(*(reg_t *)(TIM14_BASE + 0x28))
#define TIM14_ARR		(*(reg_t *)(TIM14_BASE + 0x2c))
#define TIM14_CCR1		(*(reg_t *)(TIM14_BASE + 0x34))
#define TIM14_OR		(*(reg_t *)(TIM14_BASE + 0x50))

/* ADC */
#define ADC_BASE		(0x40012400)
#define ADC_CSR			(*(reg_t *)ADC_BASE)
#define ADC_CCR			(*(reg_t *)(ADC_BASE + 4))
#define ADC_CDR			(*(reg_t *)(ADC_BASE + 8))

/* SDADC */
#define SDADC1_BASE		(0x40016000)
#define SDADC1_CR1		(*(reg_t *)SDADC1_BASE)
#define SDADC1_CR2		(*(reg_t *)(SDADC1_BASE + 0x04))
#define SDADC1_ISR		(*(reg_t *)(SDADC1_BASE + 0x08))
#define SDADC1_CLRISR		(*(reg_t *)(SDADC1_BASE + 0x0c))
#define SDADC1_JCHGR		(*(reg_t *)(SDADC1_BASE + 0x14))
#define SDADC1_CONF0R		(*(reg_t *)(SDADC1_BASE + 0x20))
#define SDADC1_CONF1R		(*(reg_t *)(SDADC1_BASE + 0x24))
#define SDADC1_CONF2R		(*(reg_t *)(SDADC1_BASE + 0x28))
#define SDADC1_CONFCHR1		(*(reg_t *)(SDADC1_BASE + 0x40))
#define SDADC1_CONFCHR2		(*(reg_t *)(SDADC1_BASE + 0x44))
#define SDADC1_JDATAR		(*(reg_t *)(SDADC1_BASE + 0x60))
#define SDADC1_RDATAR		(*(reg_t *)(SDADC1_BASE + 0x64))
#define SDADC1_JDATA12R		(*(reg_t *)(SDADC1_BASE + 0x70))
#define SDADC1_RDATA12R		(*(reg_t *)(SDADC1_BASE + 0x74))
#define SDADC1_JDATA13R		(*(reg_t *)(SDADC1_BASE + 0x78))
#define SDADC1_RDATA13R		(*(reg_t *)(SDADC1_BASE + 0x7c))

/* DAC */
#define DAC_BASE		(0x40007400)
#define DAC_CR			(*(reg_t *)DAC_BASE)
#define DAC_DHR8R2		(*(reg_t *)(DAC_BASE + 0x1c))

/* POWER */
#define PWR_BASE		(0x40007000)
#define PWR_CR			(*(reg_t *)PWR_BASE)
#define PWR_CSR			(*(reg_t *)(PWR_BASE + 0x04))

/* RTC */
#define RTC_BASE		(0x40002800)
#define RTC_TR			(*(reg_t *)RTC_BASE)
#define RTC_DR			(*(reg_t *)(RTC_BASE + 0x04))
#define RTC_CR			(*(reg_t *)(RTC_BASE + 0x08))
#define RTC_ISR			(*(reg_t *)(RTC_BASE + 0x0c))
#define RTC_PRER		(*(reg_t *)(RTC_BASE + 0x10))
#define RTC_WUTR		(*(reg_t *)(RTC_BASE + 0x14))
#define RTC_ALRMAR		(*(reg_t *)(RTC_BASE + 0x1c))
#define RTC_ALRMBR		(*(reg_t *)(RTC_BASE + 0x20))
#define RTC_WPR			(*(reg_t *)(RTC_BASE + 0x24))
#define RTC_SSR			(*(reg_t *)(RTC_BASE + 0x28))
#define RTC_SHIFTR		(*(reg_t *)(RTC_BASE + 0x2c))
#define RTC_TSTR		(*(reg_t *)(RTC_BASE + 0x30))
#define RTC_TSDR		(*(reg_t *)(RTC_BASE + 0x34))
#define RTC_TSSSR		(*(reg_t *)(RTC_BASE + 0x38))
#define RTC_CALR		(*(reg_t *)(RTC_BASE + 0x3c))
#define RTC_TAFCR		(*(reg_t *)(RTC_BASE + 0x40))
#define RTC_ALRMASSR		(*(reg_t *)(RTC_BASE + 0x44))
#define RTC_ALRMBSSR		(*(reg_t *)(RTC_BASE + 0x48))
#define RTC_BAKR		(*(reg_t *)(RTC_BASE + 0x50))

#endif /* __STM32F3_IO_H__ */

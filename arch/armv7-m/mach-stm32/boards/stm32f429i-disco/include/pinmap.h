#ifndef __PINMAP_H__
#define __PINMAP_H__

/* 00: PA base 0
 * 01: PB base 16
 * 02: PC base 32
 * 03: PD base 48
 * 04: PE base 64
 * 05: PF base 80
 * 06: PG base 96
 * 07: PH base 112
 * 08: PI base 128
 * 09: PJ base 144
 * 10: PK base 160 */

#define PIN_STATUS_LED			PIN_LED_GREEN
#define PIN_DEBUG			0 /* PA.0 */

#define PIN_LED_GREEN			109 //(96+13) /* PG.13 */
#define PIN_LED_RED			110 //(96+14) /* PG.14 */

/* UART */
#define PIN_UART1_TX			 9 /* PA.9 */
#define PIN_UART1_RX			10 /* PA.10 */

/* SDRAM */
#define PIN_FMC_D2			48 /* PD.0 */
#define PIN_FMC_D3			49 /* PD.1 */
#define PIN_FMC_D13			56 /* PD.8 */
#define PIN_FMC_D14			57 /* PD.9 */
#define PIN_FMC_D15			58 /* PD.10 */
#define PIN_FMC_D0			62 /* PD.14 */
#define PIN_FMC_D1			63 /* PD.15 */
#define PIN_FMC_SDCKE1			21 /* PB.5 */
#define PIN_FMC_SDNE1			22 /* PB.6 */
#define PIN_FMC_SDNWE			32 /* PC.0 */
#define PIN_FMC_NBL0			64 /* PE.0 */
#define PIN_FMC_NBL1			65 /* PE.1 */
#define PIN_FMC_D4			71 /* PE.7 */
#define PIN_FMC_D5			72 /* PE.8 */
#define PIN_FMC_D6			73 /* PE.9 */
#define PIN_FMC_D7			74 /* PE.10 */
#define PIN_FMC_D8			75 /* PE.11 */
#define PIN_FMC_D9			76 /* PE.12 */
#define PIN_FMC_D10			77 /* PE.13 */
#define PIN_FMC_D11			78 /* PE.14 */
#define PIN_FMC_D12			79 /* PE.15 */
#define PIN_FMC_A0			80 /* PF.0 */
#define PIN_FMC_A1			81 /* PF.1 */
#define PIN_FMC_A2			82 /* PF.2 */
#define PIN_FMC_A3			83 /* PF.3 */
#define PIN_FMC_A4			84 /* PF.4 */
#define PIN_FMC_A5			85 /* PF.5 */
#define PIN_FMC_NRAS			91 /* PF.11 */
#define PIN_FMC_A6			92 /* PF.12 */
#define PIN_FMC_A7			93 /* PF.13 */
#define PIN_FMC_A8			94 /* PF.14 */
#define PIN_FMC_A9			95 /* PF.15 */
#define PIN_FMC_A10			96 /* PG.0 */
#define PIN_FMC_A11			97 /* PG.1 */
#define PIN_FMC_BA0			100 /* PG.4 */
#define PIN_FMC_BA1			101 /* PG.5 */
#define PIN_FMC_SDCLK			104 /* PG.8 */
#define PIN_FMC_NCAS			111 /* PG.15 */

/* LCD */
#define PIN_LCD_MCU_WRX			 61 /* PD.13 */
#define PIN_LCD_MCU_RDX			 60 /* PD.12 */
#define PIN_LCD_MCU_NCS			 34 /* PC.2 */

#define PIN_LCD_IM0			 50 /* PD.2 */
#define PIN_LCD_IM1			 52 /* PD.4 */
#define PIN_LCD_IM2			 53 /* PD.5 */
#define PIN_LCD_IM3			 55 /* PD.7 */

#define PIN_LCD_R2			 42 /* PC.10 */
#define PIN_LCD_R3			 16 /* PB.0 */
#define PIN_LCD_R4			 11 /* PA.11 */
#define PIN_LCD_R5			 12 /* PA.12 */
#define PIN_LCD_R6			 17 /* PB.01 */
#define PIN_LCD_R7			102 /* PG.06 */
#define PIN_LCD_G2			  6 /* PA.06 */
#define PIN_LCD_G3			106 /* PG.10 */
#define PIN_LCD_G4			 26 /* PB.10 */
#define PIN_LCD_G5			 27 /* PB.11 */
#define PIN_LCD_G6			 39 /* PC.07 */
#define PIN_LCD_G7			 51 /* PD.03 */
#define PIN_LCD_B2			 54 /* PD.06 */
#define PIN_LCD_B3			107 /* PG.11 */
#define PIN_LCD_B4			108 /* PG.12 */
#define PIN_LCD_B5			  3 /* PA.03 */
#define PIN_LCD_B6			 24 /* PB.08 */
#define PIN_LCD_B7			 25 /* PB.09 */
#define PIN_LCD_HSYNC			 38 /* PC.06 */
#define PIN_LCD_VSYNC			  4 /* PA.04 */
#define PIN_LCD_CLK			103 /* PG.07 */
#define PIN_LCD_DE			 90 /* PF.10 */
#define PIN_LCD_TE			 59 /* PD.11 */

/* SPI5 */
#define PIN_SPI5_SCK			87 /* PF.7 */
#define PIN_SPI5_MISO			88 /* PF.8 */
#define PIN_SPI5_MOSI			89 /* PF.9 */

/* Timer */
#define PIN_TIM2CH1			 0 /* PA.0 */
#define PIN_TIM2CH2			 1 /* PA.1 */
#define PIN_TIM2CH3			 2 /* PA.2 */
#define PIN_TIM2CH4			 3 /* PA.3 */
#define PIN_TIM3CH1			 6 /* PA.6 */
#define PIN_TIM3CH2			 7 /* PA.7 */
#define PIN_TIM3CH3			16 /* PB.0 */
#define PIN_TIM3CH4			17 /* PB.1 */
#define PIN_TIM4CH1			22 /* PB.6 */
#define PIN_TIM4CH2			23 /* PB.7 */
#define PIN_TIM4CH3			24 /* PB.8 */
#define PIN_TIM4CH4			25 /* PB.9 */
#define PIN_TIM5CH1			 0 /* PA.0 */
#define PIN_TIM5CH2			PIN_TIM2CH2
#define PIN_TIM5CH3			PIN_TIM2CH3
#define PIN_TIM5CH4			PIN_TIM2CH4

#endif /* __PINMAP_H__ */

#ifndef __YAOS_PINMAP_H__
#define __YAOS_PINMAP_H__

/* 00: PA base 0
 * 01: PB base 16
 * 02: PC base 32 */

#define PIN_DEBUG			28 /* PB.12 */
#define PIN_STATUS_LED			45 /* PC.13 */

/* UART */
#define PIN_UART1_TX			 9 /* PA.9 */
#define PIN_UART1_RX			10 /* PA.10 */
#define PIN_UART2_TX			 2 /* PA.2 */
#define PIN_UART2_RX			 3 /* PA.3 */
#define PIN_UART3_TX			26 /* PB.10 */
#define PIN_UART3_RX			27 /* PB.11 */
#define PIN_UART4_TX			42 /* PC.10 */
#define PIN_UART4_RX			43 /* PC.11 */
#define PIN_UART5_TX			44 /* PC.12 */
#define PIN_UART5_RX			50 /* PD.2 */

/* SPI */
#define PIN_SPI1_NSS			 4 /* PA.4 */
#define PIN_SPI1_SCK			 5 /* PA.5 */
#define PIN_SPI1_MISO			 6 /* PA.6 */
#define PIN_SPI1_MOSI			 7 /* PA.7 */
#define PIN_SPI2_NSS			28 /* PB.12 */
#define PIN_SPI2_SCK			29 /* PB.13 */
#define PIN_SPI2_MISO			30 /* PB.14 */
#define PIN_SPI2_MOSI			31 /* PB.15 */

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

/* e-paper pinout */
#define PIN_EPD_CS			16 /* PB.0 */
#define PIN_EPD_DC			17 /* PB.1 */
#define PIN_EPD_RST			26 /* PB.10 */
#define PIN_EPD_BUSY			27 /* PB.11 */

/* CLCD */
#define PIN_CLCD_DB7			 4
#define PIN_CLCD_DB6			 5
#define PIN_CLCD_DB5			 6
#define PIN_CLCD_DB4			 7
#define PIN_CLCD_E			 8
#define PIN_CLCD_RW			11
#define PIN_CLCD_RS			12

/* Infrared Receiver(IR) */
#define PIN_IR				 0

#endif /* __YAOS_PINMAP_H__ */

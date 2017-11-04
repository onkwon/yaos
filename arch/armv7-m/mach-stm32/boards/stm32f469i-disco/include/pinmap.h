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

#define PIN_STATUS_LED			PIN_LED_BLUE

#define PIN_LED_GREEN			112//(96+6) /* PG.6 */
#define PIN_LED_ORANGE			52//(48+4) /* PD.4 */
#define PIN_LED_RED			53//(48+5) /* PD.5 */
#define PIN_LED_BLUE			163//(160+3) /* PK.3 */

/* UART */
#define PIN_UART6_TX			(32+6) /* PC.6 */
#define PIN_UART6_RX			(32+7) /* PC.7 */

#define PIN_LCD_RESET			(112+7) /* PH.7 */
#define PIN_LCD_TEAR			(144+2) /* PJ.2 */
#define PIN_LCD_BACKLIGHT		(3) /* PA.3 */

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

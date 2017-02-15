#ifndef __PINMAP_H__
#define __PINMAP_H__

#define PIN_DEBUG			24 /* PORTB.8 */
#define PIN_STATUS_LED			50 /* PORTD.2 */

/* Timer */
#define PIN_TIM2CH2			1 /* PA.1 */
#define PIN_TIM2CH3			2 /* PA.2 */
#define PIN_TIM2CH4			3 /* PA.3 */
#define PIN_TIM3CH1			6 /* PA.6 */
#define PIN_TIM3CH2			7 /* PA.7 */
#define PIN_TIM3CH3			16 /* PB.0 */
#define PIN_TIM3CH4			17 /* PB.1 */
#define PIN_TIM4CH1			22 /* PB.6 */
#define PIN_TIM4CH2			23 /* PB.7 */
#define PIN_TIM4CH3			24 /* PB.8 */
#define PIN_TIM4CH4			25 /* PB.9 */
#define PIN_TIM5CH1			0 /* PA.0 */
#define PIN_TIM5CH2			PIN_TIM2CH2
#define PIN_TIM5CH3			PIN_TIM2CH3
#define PIN_TIM5CH4			PIN_TIM2CH4

/* CLCD */
#define PIN_CLCD_DB7			4
#define PIN_CLCD_DB6			5
#define PIN_CLCD_DB5			6
#define PIN_CLCD_DB4			7
#define PIN_CLCD_E			8
#define PIN_CLCD_RW			11
#define PIN_CLCD_RS			12

/* Infrared Receiver(IR) */
#define PIN_IR				0

#endif /* __PINMAP_H__ */

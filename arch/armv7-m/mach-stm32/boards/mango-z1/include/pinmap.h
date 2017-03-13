#ifndef __PINMAP_H__
#define __PINMAP_H__

/* PA base 0
 * PB base 16
 * PC base 32
 * PD base 48
 * PE base 64
 * PF base 80
 * PG base 96 */

#define PIN_DEBUG			24 /* PORTB.8 */

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

/* Infrared Receiver(IR) */
#define PIN_IR				 0

#endif /* __PINMAP_H__ */

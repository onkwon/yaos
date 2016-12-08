#ifndef __PINMAP_H__
#define __PINMAP_H__

#ifndef stm32f1
#define stm32f1	1
#define stm32f4	2
#endif

#define PIN_DEBUG			24 /* PORTB.8 */

/* Status LED */
#if (SOC == stm32f1)
#define PIN_STATUS_LED			50 /* PORTD.2 */
#elif (SOC == stm32f3)
#define PIN_STATUS_LED			32 /* PORTC.0 */
#elif (SOC == stm32f4)
#define PIN_STATUS_LED			20 /* PORTB.4 */
#endif

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

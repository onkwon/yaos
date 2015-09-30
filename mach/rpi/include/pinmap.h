#ifndef __PINMAP_H__
#define __PINMAP_H__

/* Status LED */
#ifdef RPI2
#define PIN_STATUS_LED			47
#else
#define PIN_STATUS_LED			16
#endif

/* CLCD */
#define PIN_CLCD_DB7			4
#define PIN_CLCD_DB6			17
#define PIN_CLCD_DB5			27
#define PIN_CLCD_DB4			22
#define PIN_CLCD_E			10
#define PIN_CLCD_RW			9
#define PIN_CLCD_RS			11

/* Infrared Receiver(IR) */
#define PIN_IR				7

#endif /* __PINMAP_H__ */

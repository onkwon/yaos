#include <usart.h>
#include <stdlib.h>
#include "exti.h"
#include "clock.h"

#ifndef stm32f1
#define stm32f1	1
#define stm32f3	3
#define stm32f4	4
#endif

#define RXNE		5
#define TXE		7

static unsigned int brr2reg(unsigned int baudrate, unsigned int clk)
{
	unsigned int fraction, mantissa;

	/* 25 * 4 = 100; not to lose the result below the decimal point */
	fraction = (clk * 25) / (baudrate * 4);
	mantissa = fraction / 100; /* to get the actual integer part */
	fraction = fraction - (mantissa * 100); /* to get the fraction part */
	fraction = ((fraction << 4/* sampling */) + 50/* round up */) / 100;
	baudrate = (mantissa << 4) | (fraction & 0xf);

	return baudrate;
}

static inline int ch2vec(unsigned int channel)
{
	int nvec = -1;

	if (channel < 3)
		nvec = 53 + channel;
	else if (channel < 5)
		nvec = 68 + channel - 3;

	return nvec;
}

static inline reg_t *ch2reg(unsigned int channel)
{
	reg_t *reg = NULL;

	switch (channel) {
	case 1: reg = (reg_t *)USART2;
		break;
	case 2: reg = (reg_t *)USART3;
		break;
#ifdef ADDITIONAL_UART
	case 3: reg = (reg_t *)UART4;
		break;
	case 4: reg = (reg_t *)UART5;
		break;
#endif
	case 0: reg = (reg_t *)USART1;
		break;
	default:
		break;
	}

	return reg;
}

static inline int usart_open(unsigned int channel, struct usart arg)
{
	unsigned int port, pin, apb_nbit;
#if (SOC == stm32f3 || SOC == stm32f4)
	unsigned int alt = 7;
#endif
	reg_t *reg = ch2reg(channel);

	/* USART signal can be remapped to some other port pins. */
	switch ((unsigned int)reg) {
	case USART1:
		port     = PORTA;
		pin      = 9;  /* PA9: TX, PA10: RX */
#if (SOC == stm32f1 || SOC == stm32f3)
		apb_nbit = 14;
#elif (SOC == stm32f4)
		apb_nbit = 4;
#endif
		break;
	case USART2:
		port     = PORTA;
		pin      = 2;  /* PA2: TX, PA3: RX */
		apb_nbit = 17;
		break;
	case USART3:
		port     = PORTB;
		pin      = 10; /* PB10: TX, PB11: RX */
		apb_nbit = 18;
		break;
#ifdef ADDITIONAL_UART
	case UART4:
		port     = PORTC;
		pin      = 10; /* PC10: TX, PC11: RX */
		apb_nbit = 19;
		break;
	case UART5:
		port     = PORTC;
		pin      = 12; /* PC12: TX, PD2: RX */
		apb_nbit = 20;
		break;
#endif
	default:
		return -1;
	}

	if ((unsigned int)reg == USART1) {
		__turn_apb2_clock(apb_nbit, ON);
		__reset_apb2_device(apb_nbit);

		arg.brr = brr2reg(arg.brr, get_pclk2());
	} else {
		__turn_apb1_clock(apb_nbit, ON);
		__reset_apb1_device(apb_nbit);

		arg.brr = brr2reg(arg.brr, get_pclk1());
	}

	/* gpio configuration. in case of remapping, check pinout. */
	gpio_init(reg2port((reg_t *)port) * PINS_PER_PORT + pin, GPIO_MODE_ALT | GPIO_SPD_SLOW);
	/* FIXME: rx pin doesn't match when uart5 */
	/* TODO: support dynamic pin mapping not fixed */
	gpio_init(reg2port((reg_t *)port) * PINS_PER_PORT + pin + 1, GPIO_MODE_ALT);

	/* TODO: FOR TEST, use rx pin as wake-up source */
	link_exti_to_nvic(reg2port((reg_t *)port), pin+1);

	nvic_set(vec2irq(ch2vec(channel)), ON);

#if (SOC == stm32f3)
	reg[0] = arg.cr1;
	reg[1] = arg.cr2;
	reg[2] = arg.cr3;
	reg[3] = arg.brr;
	reg[4] = arg.gtpr;
#else
	reg[2] = arg.brr;
	reg[3] = arg.cr1;
	reg[4] = arg.cr2;
	reg[5] = arg.cr3;
	reg[6] = arg.gtpr;
#endif

	return ch2vec(channel);
}

static inline void usart_close(unsigned int channel)
{
	reg_t *reg = ch2reg(channel);

	/* check if still in transmission. */
#if (SOC == stm32f3)
	while (!gbi(reg[7], 7));
#else
	while (!gbi(reg[0], 7)); /* wait until TXE bit set */
#endif

	/* Use APB2 peripheral reset register (RCC_APB2RSTR),
	 * or just turn off enable bit of tranceiver, controller and clock. */

	/* Turn off enable bit of transmitter, receiver, and clock.
	 * It leaves port clock, pin, irq, and configuration as set */
#if (SOC == stm32f3)
	reg[0] &= ~(
			(1 << 0) 	/* UE: USART enable */
			| (1 << 5)	/* RXNEIE: RXNE interrupt enable */
			| (1 << 3) 	/* TE: Transmitter enable */
			| (1 << 2));	/* RE: Receiver enable */
#else
	reg[3] &= ~(
			(1 << 13) 	/* UE: USART enable */
			| (1 << 5)	/* RXNEIE: RXNE interrupt enable */
			| (1 << 3) 	/* TE: Transmitter enable */
			| (1 << 2));	/* RE: Receiver enable */
#endif

	if (!channel) { /* USART1 */
#if (SOC == stm32f1 || SOC == stm32f3)
		__turn_apb2_clock(14, OFF);
#elif (SOC == stm32f4)
		__turn_apb2_clock(4, OFF);
#endif
	} else {
		__turn_apb1_clock(channel + 16, OFF);
	}

	nvic_set(vec2irq(ch2vec(channel)), OFF);
}

/* to get buf index from register address */
#define GET_USART_NR(from)     (from == USART1? 0 : (((from >> 8) & 0xff) - 0x40) / 4)

static inline int usart_putc(unsigned int channel, int c)
{
	reg_t *reg = ch2reg(channel);

#if (SOC == stm32f3)
	if (!gbi(reg[7], TXE))
		return 0;
#else
	if (!gbi(reg[0], TXE))
		return 0;
#endif

#if (SOC == stm32f3)
	reg[10] = (unsigned int)c;
#else
	reg[1] = (unsigned int)c;
#endif

	return 1;
}

int __usart_open(unsigned int channel, unsigned int baudrate)
{
#if (SOC == stm32f3)
	return usart_open(channel, (struct usart) {
		.brr  = baudrate,
		.gtpr = 0,
		.cr3  = 0,
		.cr2  = 0,
		.cr1  = (1 << 0)	/* UE    : USART enable */
			| (1 << 5)	/* RXNEIE: RXNE interrupt enable */
			| (1 << 3)	/* TE    : Transmitter enable */
			| (1 << 2)	/* RE    : Receiver enable */
	});
#else
	return usart_open(channel, (struct usart) {
		.brr  = baudrate,
		.gtpr = 0,
		.cr3  = 0,
		.cr2  = 0,
		.cr1  = (1 << 13)	/* UE    : USART enable */
			| (1 << 5)	/* RXNEIE: RXNE interrupt enable */
			| (1 << 3)	/* TE    : Transmitter enable */
			| (1 << 2)	/* RE    : Receiver enable */
	});
#endif
}

void __usart_close(unsigned int channel)
{
	usart_close(channel);
}

int __usart_putc(unsigned int channel, int c)
{
	return usart_putc(channel, c);
}

int __usart_check_rx(unsigned int channel)
{
	reg_t *reg = ch2reg(channel);

#if (SOC == stm32f3)
	if (reg[7] & (1 << RXNE))
		return 1;
#else
	if (reg[0] & (1 << RXNE))
		return 1;
#endif

	return 0;
}

int __usart_check_tx(unsigned int channel)
{
	reg_t *reg = ch2reg(channel);

#if (SOC == stm32f3)
	if (reg[7] & (1 << TXE))
		return 1;
#else
	if (reg[0] & (1 << TXE))
		return 1;
#endif

	return 0;
}

int __usart_getc(unsigned int channel)
{
	reg_t *reg = ch2reg(channel);

#if (SOC == stm32f3)
	return reg[9];
#else
	return reg[1];
#endif
}

void __usart_tx_irq_reset(unsigned int channel)
{
	reg_t *reg = ch2reg(channel);

	/* TXE interrupt disable */
#if (SOC == stm32f3)
	reg[0] &= ~(1 << 7); /* TXEIE */
#else
	reg[3] &= ~(1 << TXE); /* TXEIE */
#endif
}

void __usart_tx_irq_raise(unsigned int channel)
{
	reg_t *reg = ch2reg(channel);

	/* TXE interrupt enable */
#if (SOC == stm32f3)
	reg[0] |= 1 << 7; /* TXEIE */
#else
	reg[3] |= 1 << TXE; /* TXEIE */
#endif
}

void __usart_flush(unsigned int channel)
{
	reg_t *reg = ch2reg(channel);

	/* wait until transmission complete */
#if (SOC == stm32f3)
	while (!gbi(reg[7], 6));
#else
	while (!gbi(reg[0], 6));
#endif
}

unsigned int __usart_get_baudrate(unsigned int channel)
{
	return 0;
}

int __usart_set_baudrate(unsigned int channel, unsigned int baudrate)
{
	reg_t *reg = ch2reg(channel);

	if (!channel) /* USART1 */
		baudrate = brr2reg(baudrate, get_pclk2());
	else
		baudrate = brr2reg(baudrate, get_pclk1());

#if (SOC == stm32f3)
	reg[3] = baudrate;
#else
	reg[2] = baudrate;
#endif

	return 0;
}

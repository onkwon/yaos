#include <usart.h>
#include <stdlib.h>
#include <io.h>
#include "exti.h"

#ifndef stm32f1
#define stm32f1	1
#define stm32f4	2
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

static int get_usart_vector(unsigned int channel)
{
	int nvector = -1;

	switch (channel) {
	case USART1:
		nvector = 53; /* IRQ 37 */
		break;
	case USART2:
		nvector = 54; /* IRQ 38 */
		break;
	case USART3:
		nvector = 55; /* IRQ 39 */
		break;
	case UART4:
		nvector = 68; /* IRQ 52 */
		break;
	case UART5:
		nvector = 69; /* IRQ 53 */
		break;
	}

	return nvector;
}

static int usart_open(unsigned int channel, struct usart arg)
{
	unsigned int port, pin, apb_nbit;
#if (SOC == stm32f4)
	unsigned int alt = 7;
#endif

	/* USART signal can be remapped to some other port pins. */
	switch (channel) {
	case USART1:
		port     = PORTA;
		pin      = 9;  /* PA9: TX, PA10: RX */
#if (SOC == stm32f1)
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
	default:
		return -1;
	}

	if (channel == USART1) {
		SET_CLOCK_APB2(ENABLE, apb_nbit); /* USART1 clock enable */
		/* reset usart */
		RCC_APB2RSTR |= (1 << apb_nbit);
		RCC_APB2RSTR &= ~(1 << apb_nbit);

		arg.brr = brr2reg(arg.brr, get_pclk2());
	} else {
		SET_CLOCK_APB1(ENABLE, apb_nbit); /* USARTn clock enable */
		/* reset usart */
		RCC_APB1RSTR |= (1 << apb_nbit);
		RCC_APB1RSTR &= ~(1 << apb_nbit);

		arg.brr = brr2reg(arg.brr, get_pclk1());
	}

	SET_PORT_CLOCK(ENABLE, port);

	/* gpio configuration. in case of remapping, check pinout. */
#if (SOC == stm32f1)
	SET_PORT_PIN(port, pin, PIN_ALT | PIN_OUTPUT); /* tx */
#elif (SOC == stm32f4)
	SET_PORT_PIN(port, pin, PIN_ALT); /* tx */
	SET_PORT_ALT(port, pin, alt);
	SET_PORT_ALT(port, pin+1, alt);
#endif
	if (channel == UART5)
		SET_PORT_PIN(PORTD, 2, PIN_ALT); /* rx */
	else
		SET_PORT_PIN(port, pin+1, PIN_ALT); /* rx */

	/* FOR TEST to use rx pin as wake-up source */
	link_exti_to_nvic(port, pin+1);

	nvic_set(exc2irq(get_usart_vector(channel)), ON);

	*(volatile unsigned int *)(channel + 0x08) = arg.brr;
	*(volatile unsigned int *)(channel + 0x0c) = arg.cr1;
	*(volatile unsigned int *)(channel + 0x10) = arg.cr2;
	*(volatile unsigned int *)(channel + 0x14) = arg.cr3;
	*(volatile unsigned int *)(channel + 0x18) = arg.gtpr;

	return get_usart_vector(channel);
}

static void usart_close(unsigned int channel)
{
	/* check if still in transmission. */
	while (!gbi(*(volatile unsigned int *)channel, 7)); /* wait until TXE bit set */

	/* Use APB2 peripheral reset register (RCC_APB2RSTR),
	 * or just turn off enable bit of tranceiver, controller and clock. */

	/* Turn off enable bit of transmitter, receiver, and clock.
	 * It leaves port clock, pin, irq, and configuration as set */
	*(volatile unsigned int *)(channel + 0x0c) &= ~(
			(1 << 13) 	/* UE: USART enable */
			| (1 << 5)	/* RXNEIE: RXNE interrupt enable */
			| (1 << 3) 	/* TE: Transmitter enable */
			| (1 << 2));	/* RE: Receiver enable */

	if (channel == USART1) {
#if (SOC == stm32f1)
		SET_CLOCK_APB2(DISABLE, 14); /* USART1 clock disable */
#elif (SOC == stm32f4)
		SET_CLOCK_APB2(DISABLE, 4); /* USART1 clock disable */
#endif
	} else {
		/* USARTn clock disable */
		SET_CLOCK_APB1(DISABLE, (((channel >> 8) & 0x1f) >> 2) + 16);
	}

	nvic_set(exc2irq(get_usart_vector(channel)), OFF);
}

/* to get buf index from register address */
#define GET_USART_NR(from)     (from == USART1? 0 : (((from >> 8) & 0xff) - 0x40) / 4)

static int usart_putc(unsigned int channel, int c)
{
	if (!gbi(*(volatile unsigned int *)channel, 7))
		return 0;

	*(volatile unsigned int *)(channel + 0x04) = (unsigned int)c;

	return 1;
}

static inline unsigned int conv_channel(unsigned int channel)
{
	switch (channel) {
	case 1: channel = USART2;
		break;
	case 2: channel = USART3;
		break;
	case 3: channel = UART4;
		break;
	case 4: channel = UART5;
		break;
	case 0: channel = USART1;
		break;
	default:channel = -1;
		break;
	}

	return channel;
}

int __usart_open(unsigned int channel, unsigned int baudrate)
{
	return usart_open(conv_channel(channel), (struct usart) {
		.brr  = baudrate,
		.gtpr = 0,
		.cr3  = 0,
		.cr2  = 0,
		.cr1  = (1 << 13)	/* UE    : USART enable */
			| (1 << 5)	/* RXNEIE: RXNE interrupt enable */
			| (1 << 3)	/* TE    : Transmitter enable */
			| (1 << 2)	/* RE    : Receiver enable */
	});
}

void __usart_close(unsigned int channel)
{
	usart_close(conv_channel(channel));
}

int __usart_putc(unsigned int channel, int c)
{
	return usart_putc(conv_channel(channel), c);
}

int __usart_check_rx(unsigned int channel)
{
	channel = conv_channel(channel);

	if (*(volatile unsigned int *)channel & (1 << RXNE))
		return 1;

	return 0;
}

int __usart_check_tx(unsigned int channel)
{
	channel = conv_channel(channel);

	if (*(volatile unsigned int *)channel & (1 << TXE))
		return 1;

	return 0;
}

int __usart_getc(unsigned int channel)
{
	channel = conv_channel(channel);

	return *(volatile unsigned int *)(channel + 0x04);
}

void __usart_tx_irq_reset(unsigned int channel)
{
	channel = conv_channel(channel);

	/* TXE interrupt disable */
	*(volatile unsigned int *)(channel + 0x0c) &= ~(1 << TXE); /* TXEIE */
}

void __usart_tx_irq_raise(unsigned int channel)
{
	channel = conv_channel(channel);

	/* TXE interrupt enable */
	*(volatile unsigned int *)(channel + 0x0c) |= 1 << TXE; /* TXEIE */
}

void __usart_flush(unsigned int channel)
{
	/* wait until transmission complete */
	while (!gbi(*(volatile unsigned int *)conv_channel(channel), 6));
}

unsigned int __usart_get_baudrate(unsigned int channel)
{
	channel = conv_channel(channel);
	return 0;
}

int __usart_set_baudrate(unsigned int channel, unsigned int baudrate)
{
	channel = conv_channel(channel);

	if (channel == USART1)
		baudrate = brr2reg(baudrate, get_pclk2());
	else
		baudrate = brr2reg(baudrate, get_pclk1());

	*(volatile unsigned int *)(channel + 0x08) = baudrate;

	return 0;
}

#include <usart.h>
#include <stdlib.h>
#include <foundation.h>

#define RXNE		5
#define TXE		7

static unsigned int brr2reg(unsigned int baudrate, unsigned int clk)
{
	unsigned int fraction, mantissa;

	baudrate /= 100;
	mantissa  = (clk * 10) / (16 * baudrate);
	fraction  = mantissa % 1000;
	mantissa /= 1000;
	fraction  = fraction * 16 / 1000;
	baudrate  = (mantissa << 4) + fraction;

	return baudrate;
}

static int usart_open(unsigned int channel, struct usart arg)
{
	unsigned int port, pin, nvector, apb_nbit;

	/* USART signal can be remapped to some other port pins. */
	switch (channel) {
	case USART1:
		port      = PORTA;
		pin       = 9;  /* PA9: TX, PA10: RX */
		nvector   = 53; /* IRQ 37 */
		apb_nbit  = 14;
		break;
	case USART2:
		port      = PORTA;
		pin       = 2;  /* PA2: TX, PA3: RX */
		nvector   = 54; /* IRQ 38 */
		apb_nbit  = 17;
		break;
	case USART3:
		port      = PORTB;
		pin       = 10; /* PB10: TX, PB11: RX */
		nvector   = 55; /* IRQ 39 */
		apb_nbit  = 18;
		break;
	case UART4:
		port      = PORTC;
		pin       = 10; /* PC10: TX, PC11: RX */
		nvector   = 68; /* IRQ 52 */
		apb_nbit  = 19;
		break;
	case UART5:
		port      = PORTC;
		pin       = 12; /* PC12: TX, PD2: RX */
		nvector   = 69; /* IRQ 53 */
		apb_nbit  = 20;
		break;
	default:
		return -1;
	}

	if (channel == USART1) {
		SET_CLOCK_APB2(ENABLE, apb_nbit); /* USART1 clock enable */
	} else {
		SET_CLOCK_APB1(ENABLE, apb_nbit); /* USARTn clock enable */
	}

	SET_PORT_CLOCK(ENABLE, port);

	/* gpio configuration. in case of remapping or UART5, check pinout. */
	SET_PORT_PIN(port, pin, PIN_OUTPUT_50MHZ | PIN_ALT); /* tx */
	SET_PORT_PIN(port, pin+1, PIN_INPUT | PIN_FLOATING); /* rx */

	/* FOR TEST to use rx pin as wake-up source */
	LINK_EXTI2NVIC(port, pin+1);

	SET_IRQ(ON, nvector - 16);

	*(volatile unsigned int *)(channel + 0x08) = arg.brr;
	*(volatile unsigned int *)(channel + 0x0c) = arg.cr1;
	*(volatile unsigned int *)(channel + 0x10) = arg.cr2;
	*(volatile unsigned int *)(channel + 0x14) = arg.cr3;
	*(volatile unsigned int *)(channel + 0x18) = arg.gtpr;

	return nvector;
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
			(1 << 13) |	/* UE: USART enable */
			(1 << 3) |	/* TE: Transmitter enable */
			(1 << 2));	/* RE: Receiver enable */

	if (channel == USART1)
		SET_CLOCK_APB2(DISABLE, 14); /* USART1 clock disable */
	else
		SET_CLOCK_APB1(DISABLE, (((channel >> 8) & 0x1f) >> 2) + 16); /* USARTn clock disable */

	/* SET_IRQ(OFF, nvector-16); */
}

/* to get buf index from register address */
#define GET_USART_NR(from)     (from == USART1? 0 : (((from >> 8) & 0xff) - 0x40) / 4)

static void usart_putc(unsigned int channel, int c)
{
	while (!gbi(*(volatile unsigned int *)channel, 7)); /* wait until TXE bit set */
	*(volatile unsigned int *)(channel + 0x04) = (unsigned int)c;
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
		.brr  = brr2reg(baudrate, get_sysclk()),
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

void __usart_putc(unsigned int channel, int c)
{
	usart_putc(conv_channel(channel), c);
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

void __putc_debug(int c)
{
	__usart_putc(0, c);

	if (c == '\n')
		__usart_putc(0, '\r');
}

void __usart_flush(unsigned int channel)
{
	/* wait until transmission complete */
	while (!gbi(*(volatile unsigned int *)conv_channel(channel), 6));
}

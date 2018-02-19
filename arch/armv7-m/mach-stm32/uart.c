/*
 * "[...] Sincerity (comprising truth-to-experience, honesty towards the self,
 * and the capacity for human empathy and compassion) is a quality which
 * resides within the laguage of literature. It isn't a fact or an intention
 * behind the work [...]"
 *
 *             - An introduction to Literary and Cultural Theory, Peter Barry
 *
 *
 *                                                   o8o
 *                                                   `"'
 *     oooo    ooo  .oooo.    .ooooo.   .oooo.o     oooo   .ooooo.
 *      `88.  .8'  `P  )88b  d88' `88b d88(  "8     `888  d88' `88b
 *       `88..8'    .oP"888  888   888 `"Y88b.       888  888   888
 *        `888'    d8(  888  888   888 o.  )88b .o.  888  888   888
 *         .8'     `Y888""8o `Y8bod8P' 8""888P' Y8P o888o `Y8bod8P'
 *     .o..P'
 *     `Y8P'                   Kyunghwan Kwon <kwon@yaos.io>
 *
 *  Welcome aboard!
 */

#include "include/uart.h"
#include <stdlib.h>
#include "include/exti.h"
#include "include/clock.h"
#include <asm/pinmap.h>
#include <error.h>

enum {
	RE	= 2, /* Receiver enable */
	TE	= 3, /* Transmitter enable */
	RXNEIE	= 5, /* RXNE interrupt enable */
	RXNE	= 5,
	TXE	= 7,
#ifdef stm32f3
	UE	= 0, /* USART enable */
#else
	UE	= 13,
#endif
};

struct _regs {
	/*unsigned int sr;*/
	/*unsigned int dr;*/
	unsigned int brr;
	unsigned int cr1;
	unsigned int cr2;
	unsigned int cr3;
	unsigned int gtpr;
};

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

static inline int ch2vec(int channel)
{
	int nvec = -1;

	if (channel < 3)
		nvec = 53 + channel;
	else if (channel < 5)
		nvec = 68 + channel - 3;
	else if (channel == 5)
		nvec = 87;
	else
		nvec = 98 + channel - 6;

	return nvec;
}

static inline reg_t *ch2reg(int channel)
{
	reg_t *reg = NULL;

	switch (channel) {
	case 1: reg = (reg_t *)USART2;
		break;
	case 2: reg = (reg_t *)USART3;
		break;
#ifdef UART4
	case 3: reg = (reg_t *)UART4;
		break;
#endif
#ifdef UART5
	case 4: reg = (reg_t *)UART5;
		break;
#endif
	case 6:
	case 7:
		error("UART7,8 are not supported");
		reg = NULL;
		break;
#ifdef USART6
	case 5: reg = (reg_t *)USART6;
		break;
#endif
	case 0: reg = (reg_t *)USART1;
		break;
	default:
		break;
	}

	return reg;
}

static inline int uart_open(int channel, struct _regs arg)
{
	reg_t *reg;

	if (!(reg = ch2reg(channel)))
		return -EINVAL;

	if ((unsigned int)reg & 0x10000) { /* if USART1 or USART6 */
#if defined(stm32f1) || defined(stm32f3)
		__turn_apb2_clock(14, ON);
		__reset_apb2_device(14);
#elif defined(stm32f4)
		__turn_apb2_clock(4 + (channel >> 2), ON);
		__reset_apb2_device(4 + (channel >> 2));
#else
#error undefined machine
#endif
		arg.brr = brr2reg(arg.brr, get_pclk2());
	} else {
		__turn_apb1_clock(channel + 16, ON);
		__reset_apb1_device(channel + 16);

		arg.brr = brr2reg(arg.brr, get_pclk1());
	}

#ifdef stm32f3
	reg[0] = arg.cr1;
	reg[1] = arg.cr2;
	reg[2] = arg.cr3;
	reg[3] = arg.brr;
	reg[4] = arg.gtpr;
#elif defined(stm32f1) || defined(stm32f4)
	reg[2] = arg.brr;
	reg[3] = arg.cr1;
	reg[4] = arg.cr2;
	reg[5] = arg.cr3;
	reg[6] = arg.gtpr;
#else
#error undefined machine
#endif

	nvic_enable(ch2vec(channel), true);

	return ch2vec(channel);
}

static inline void uart_close(int channel)
{
	reg_t *reg;

	if (!(reg = ch2reg(channel)))
		return;

	/* check if still in transmission. */
#ifdef stm32f3
	while (!gbi(reg[7], TXE));
#elif defined(stm32f1) || defined(stm32f4)
	while (!gbi(reg[0], TXE)); /* wait until TXE bit set */
#else
#error undefined machine
#endif

	if ((unsigned int)reg & 0x10000) { /* if USART1 or USART6 */
#if defined(stm32f1) || defined(stm32f3)
		__turn_apb2_clock(14, OFF);
#elif defined(stm32f4)
		__turn_apb2_clock(4 + (channel >> 3), OFF);
#else
#error undefined machine
#endif
	} else {
		__turn_apb1_clock(channel + 16, OFF);
	}

	nvic_enable(ch2vec(channel), false);
	/* TODO: gpio_fini() and unlink_exti_to_nvic() */
}

static inline int uart_putc(int channel, int c)
{
	reg_t *reg;

	if (!(reg = ch2reg(channel)))
		return 0;

	/* FIXME: Disable interrupt between checking and writing
	 * to make sure nothing interrupts in the mean time. */
#ifdef stm32f3
	if (!gbi(reg[7], TXE))
		return 0;
#else
	if (!gbi(reg[0], TXE))
		return 0;
#endif

#ifdef stm32f3
	reg[10] = (unsigned int)c;
#else
	reg[1] = (unsigned int)c;
#endif

	return 1;
}

static inline int set_uart_port(int channel, struct uart *conf)
{
	/* TODO: in case of remapping, check pinout. */
	if (conf->rx) {
		if (!conf->npin_rx) {
			switch (channel) {
			case 0:
#ifdef PIN_UART1_RX
				conf->npin_rx = PIN_UART1_RX;
#else
				goto errout;
#endif
				break;
			case 1:
#ifdef PIN_UART2_RX
				conf->npin_rx = PIN_UART2_RX;
#else
				goto errout;
#endif
				break;
			case 2:
#ifdef PIN_UART3_RX
				conf->npin_rx = PIN_UART3_RX;
#else
				goto errout;
#endif
				break;
			case 5:
#ifdef PIN_UART6_RX
				conf->npin_rx = PIN_UART6_RX;
#else
				goto errout;
#endif
				break;
			default:
				goto errout;
				break;
			}
		}

		if (channel < 5)
			gpio_init(conf->npin_rx, gpio_altfunc(7));
		else /* USART6 */
			gpio_init(conf->npin_rx, gpio_altfunc(8));

		/* TODO: FOR TEST, use rx pin as wake-up source */
		/* FIXME: register handler */
		exti_enable(conf->npin_rx, ON);
	}

	if (conf->tx) {
		if (!conf->npin_tx) {
			switch (channel) {
			case 0:
#ifdef PIN_UART1_TX
				conf->npin_tx = PIN_UART1_TX;
#else
				goto errout;
#endif
				break;
			case 1:
#ifdef PIN_UART2_TX
				conf->npin_tx = PIN_UART2_TX;
#else
				goto errout;
#endif
				break;
			case 2:
#ifdef PIN_UART3_TX
				conf->npin_tx = PIN_UART3_TX;
#else
				goto errout;
#endif
				break;
			case 5:
#ifdef PIN_UART6_TX
				conf->npin_tx = PIN_UART6_TX;
#else
				goto errout;
#endif
				break;
			default:
				goto errout;
				break;
			}
		}

		if (channel < 5)
			gpio_init(conf->npin_tx, gpio_altfunc(7) | GPIO_SPD_FASTEST);
		else
			gpio_init(conf->npin_tx, gpio_altfunc(8) | GPIO_SPD_FASTEST);
	}

	if (conf->flow) {
		if (!conf->npin_rts) {
		}
		if (!conf->npin_cts) {
		}
	}

	return 0;
errout:
	error("PIN_UARTx_[R|T]X is not defined");
	return -ERANGE;
}

/* TODO: support flow control and parity */
int __uart_open(int channel, struct uart conf)
{
	unsigned int cr1, cr2, cr3, gtpr;

	cr1 = cr2 = cr3 = gtpr = 0;

	if (set_uart_port(channel, &conf))
		return -ERANGE;

	if (conf.rx)
		cr1 |= (1 << RE) | (1 << RXNEIE);
	if (conf.tx)
		cr1 |= 1 << TE;

	cr1 |= 1 << UE;

	return uart_open(channel, (struct _regs) {
			.brr = conf.baudrate,
			.cr1 = cr1,
			.cr2 = cr2,
			.cr3 = cr3,
			.gtpr = gtpr });
}

void __uart_close(int channel)
{
	uart_close(channel);
}

int __uart_putc(int channel, int c)
{
	return uart_putc(channel, c);
}

bool __uart_has_rx(int channel)
{
	reg_t *reg;

	if (!(reg = ch2reg(channel)))
		return false;

#ifdef stm32f3
	if (reg[7] & (1 << RXNE))
		return true;
#else
	if (reg[0] & (1 << RXNE))
		return true;
#endif

	return false;
}

bool __uart_has_tx(int channel)
{
	reg_t *reg;

	if (!(reg = ch2reg(channel)))
		return false;

#ifdef stm32f3
	if (reg[7] & (1 << TXE))
		return true;
#else
	if (reg[0] & (1 << TXE))
		return true;
#endif

	return false;
}

int __uart_getc(int channel)
{
	reg_t *reg;

	if (!(reg = ch2reg(channel)))
		return false;

#ifdef stm32f3
	return reg[9];
#else
	return reg[1];
#endif
}

void __uart_tx_irq_reset(int channel)
{
	reg_t *reg;

	if (!(reg = ch2reg(channel)))
		return;

	/* TXE interrupt disable */
#ifdef stm32f3
	reg[0] &= ~(1 << TXE); /* TXEIE */
#else
	reg[3] &= ~(1 << TXE); /* TXEIE */
#endif
}

void __uart_tx_irq_raise(int channel)
{
	reg_t *reg;

	if (!(reg = ch2reg(channel)))
		return;

	/* TXE interrupt enable */
#ifdef stm32f3
	reg[0] |= 1 << TXE; /* TXEIE */
#else
	reg[3] |= 1 << TXE; /* TXEIE */
#endif
}

void __uart_flush(int channel)
{
	reg_t *reg;

	if (!(reg = ch2reg(channel)))
		return;

	/* wait until transmission complete */
#ifdef stm32f3
	while (!gbi(reg[7], 6));
#else
	while (!gbi(reg[0], 6));
#endif
}

unsigned int __uart_get_baudrate(int channel)
{
	(void)channel;
	return 0;
}

int __uart_set_baudrate(int channel, unsigned int baudrate)
{
	reg_t *reg;

	if (!(reg = ch2reg(channel)))
		return 0;

	if (!channel) /* USART1 */
		baudrate = brr2reg(baudrate, get_pclk2());
	else
		baudrate = brr2reg(baudrate, get_pclk1());

#ifdef stm32f3
	reg[3] = baudrate;
#else
	reg[2] = baudrate;
#endif

	return 0;
}

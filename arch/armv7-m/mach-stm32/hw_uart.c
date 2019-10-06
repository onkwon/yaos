#include "include/hw_uart.h"
#include "include/hw_exti.h"
#include "include/hw_clock.h"
#include "syslog.h"
#include "drivers/gpio.h"
#include "kernel/interrupt.h"

#include "arch/mach/board/pinmap.h"

#include <errno.h>

enum {
	RE	= 2, /* Receiver enable */
	TE	= 3, /* Transmitter enable */
	RXNEIE	= 5, /* RXNE interrupt enable */
	RXNE	= 5,
	TC	= 6,
	TXE	= 7,
#ifdef stm32f3
	UE	= 0, /* USART enable */
#else
	UE	= 13,
#endif
};

static inline uintptr_t brr2reg(uintptr_t baudrate, unsigned int clk)
{
	uintptr_t fraction, mantissa;

	/* 25 * 4 = 100; not to lose the result below the decimal point */
	fraction = (clk * 25) / (baudrate * 4);
	mantissa = fraction / 100; /* to get the actual integer part */
	fraction = fraction - (mantissa * 100); /* to get the fraction part */
	fraction = ((fraction << 4/* sampling */) + 50/* round up */) / 100;
	baudrate = (mantissa << 4) | (fraction & 0xf);

	return baudrate;
}

static inline int ch2vec(const int channel)
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

static inline reg_t *ch2reg(const int channel)
{
	reg_t *reg = NULL;

	switch (channel) {
	case 0: reg = (reg_t *)USART1;
		break;
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
#ifdef USART6
	case 5: reg = (reg_t *)USART6;
		break;
#endif
	case 6:
	case 7:
		debug("UART7,8 are not supported");
		break;
	default:
		break;
	}

	return reg;
}

static inline bool has_received(reg_t * const reg)
{
#ifdef stm32f3
	if (reg[7] & (1UL << RXNE))
		return true;
#else
	if (reg[0] & (1UL << RXNE))
		return true;
#endif
	return false;
}

static inline int rx_pin(const int channel)
{
	int rxpin = -ENOENT;

	switch (channel) {
	case 0:
#ifdef PIN_UART1_RX
		rxpin = PIN_UART1_RX;
#endif
		break;
	case 1:
#ifdef PIN_UART2_RX
		rxpin = PIN_UART2_RX;
#endif
		break;
	case 2:
#ifdef PIN_UART3_RX
		rxpin = PIN_UART3_RX;
#endif
		break;
	case 5:
#ifdef PIN_UART6_RX
		rxpin = PIN_UART6_RX;
#endif
		break;
	default:
		break;
	}

	return rxpin;
}

static inline int tx_pin(const int channel)
{
	int txpin = -ENOENT;

	switch (channel) {
	case 0:
#ifdef PIN_UART1_TX
		txpin = PIN_UART1_TX;
#endif
		break;
	case 1:
#ifdef PIN_UART2_TX
		txpin = PIN_UART2_TX;
#endif
		break;
	case 2:
#ifdef PIN_UART3_TX
		txpin = PIN_UART3_TX;
#endif
		break;
	case 5:
#ifdef PIN_UART6_TX
		txpin = PIN_UART6_TX;
#endif
		break;
	default:
		break;
	}

	return txpin;
}

static inline int hw_uart_gpio_init(const int channel, struct uart_conf conf)
{
	int rxpin, txpin;

	rxpin = txpin = -1;

	/* TODO: in case of remapping, check pinout. */
	if (conf.rx) {
		if ((rxpin = rx_pin(channel)) < 0)
			goto errout;

		if (channel < 5)
			gpio_init(rxpin, gpio_altfunc(7), NULL);
		else /* USART6 */
			gpio_init(rxpin, gpio_altfunc(8), NULL);

		/* TODO: FOR TEST, use rx pin as wake-up source */
		/* FIXME: register handler */
		if (UART_INTERRUPT & conf.rx)
			hw_exti_enable(rxpin, true);
	}

	if (conf.tx) {
		if ((txpin = tx_pin(channel)) < 0)
			goto errout;

		if (channel < 5)
			gpio_init(txpin, gpio_altfunc(7) | GPIO_SPD_FASTEST, NULL);
		else
			gpio_init(txpin, gpio_altfunc(8) | GPIO_SPD_FASTEST, NULL);
	}

	if (conf.flow) {
		if (!conf.rts) {
		}
		if (!conf.cts) {
		}
	}

	return 0;
errout:
	debug("PIN_UARTx_[R|T]X is not defined");
	return -ERANGE;
}

/* TODO: support flow control and parity */
int hw_uart_open(const int channel, struct uart_conf conf)
{
	uintptr_t cr1, cr2, cr3, gtpr, brr;
	reg_t *reg;

	cr1 = cr2 = cr3 = gtpr = 0;

	if (hw_uart_gpio_init(channel, conf))
		return -ERANGE;

	if (conf.rx) {
		cr1 |= (1UL << RE);
		if (UART_INTERRUPT & conf.rx)
			cr1 |= (1UL << RXNEIE);
	}

	if (conf.tx) {
		cr1 |= 1UL << TE;
	}

	cr1 |= 1UL << UE;

	if (!(reg = ch2reg(channel)))
		return -EINVAL;

	if ((uintptr_t)reg & 0x10000UL) { /* if USART1 or USART6 */
#if defined(stm32f1) || defined(stm32f3)
		hw_clock_set_apb2(14, true);
		hw_clock_reset_apb2_conf(14);
#elif defined(stm32f4)
		hw_clock_set_apb2(4 + (channel >> 2), true);
		hw_clock_reset_apb2_conf(4 + (channel >> 2));
#else
#error "undefined machine"
#endif
		brr = brr2reg(conf.baudrate, hw_clock_get_pclk2());
	} else {
		hw_clock_set_apb1(channel + 16, true);
		hw_clock_reset_apb1_conf(channel + 16);

		brr = brr2reg(conf.baudrate, hw_clock_get_pclk1());
	}

#ifdef stm32f3
	reg[0] = cr1;
	reg[1] = cr2;
	reg[2] = cr3;
	reg[3] = brr;
	reg[4] = gtpr;
#elif defined(stm32f1) || defined(stm32f4)
	reg[2] = brr;
	reg[3] = cr1;
	reg[4] = cr2;
	reg[5] = cr3;
	reg[6] = gtpr;
#else
#error "undefined machine"
#endif

	hw_irq_set(ch2vec(channel), true);

	return ch2vec(channel);
}

void hw_uart_close(const int channel)
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
#error "undefined machine"
#endif

	if ((uintptr_t)reg & 0x10000UL) { /* if USART1 or USART6 */
#if defined(stm32f1) || defined(stm32f3)
		hw_clock_set_apb2(14, false);
#elif defined(stm32f4)
		hw_clock_set_apb2(4 + (channel >> 3), false);
#else
#error "undefined machine"
#endif
	} else {
		hw_clock_set_apb1(channel + 16, false);
	}

	hw_irq_set(ch2vec(channel), false);
	/* TODO: gpio_fini() and hw_exti_enable(false) */
}

int hw_uart_writeb(const int channel, const uint8_t byte)
{
	reg_t *reg;

	if (!(reg = ch2reg(channel)))
		return -EINVAL;

#ifdef stm32f3
	if (!gbi(reg[7], TXE))
		return -EBUSY;
#else
	if (!gbi(reg[0], TXE))
		return -EBUSY;
#endif

#ifdef stm32f3
	reg[10] = (uintptr_t)byte;
#else
	reg[1] = (uintptr_t)byte;
#endif

	return 0;
}

int hw_uart_readb(const int channel, uint8_t * const byte)
{
	reg_t *reg;

	if (!byte || !(reg = ch2reg(channel)))
		return -EINVAL;

	if (!has_received(reg))
		return -ENOENT;

#ifdef stm32f3
	*byte = (uint8_t)reg[9];
#else
	*byte = (uint8_t)reg[1];
#endif
	return 0;
}

void hw_uart_flush(const int channel)
{
	reg_t *reg;

	if (!(reg = ch2reg(channel)))
		return;

	/* wait until transmission complete */
#ifdef stm32f3
	while (!gbi(reg[7], TC));
#else
	while (!gbi(reg[0], TC));
#endif
}

int hw_uart_baudrate_set(const int channel, uintptr_t baudrate)
{
	reg_t *reg;

	if (!(reg = ch2reg(channel)))
		return -EINVAL;

	if (!channel) /* USART1 */
		baudrate = brr2reg(baudrate, hw_clock_get_pclk2());
	else
		baudrate = brr2reg(baudrate, hw_clock_get_pclk1());

	// FIXME: 1. wait until ongoing transmission complete
	//        2. deactivate uart
	//        3. change baudrate
	//        4. activate uart
#ifdef stm32f3
	reg[3] = baudrate;
#else
	reg[2] = baudrate;
#endif

	return 0;
}

// TODO: Implement
uintptr_t hw_uart_baudrate_get(const int channel)
{
	(void)channel;
	return 0;
}

bool hw_uart_has_received(const int channel)
{
	reg_t *reg;

	if (!(reg = ch2reg(channel)))
		return false;

	return has_received(reg);
}

bool hw_uart_is_tx_ready(const int channel)
{
	reg_t *reg;

	if (!(reg = ch2reg(channel)))
		return false;

#ifdef stm32f3
	if (reg[7] & (1UL << TXE))
		return true;
#else
	if (reg[0] & (1UL << TXE))
		return true;
#endif

	return false;
}

void hw_uart_disable_tx_interrupt(const int channel)
{
	reg_t *reg;

	if (!(reg = ch2reg(channel)))
		return;

	/* TXE interrupt disable */
#ifdef stm32f3
	reg[0] &= ~(1UL << TXE); /* TXEIE */
#else
	reg[3] &= ~(1UL << TXE); /* TXEIE */
#endif
}

void hw_uart_raise_tx_interrupt(const int channel)
{
	reg_t *reg;

	if (!(reg = ch2reg(channel)))
		return;

	/* TXE interrupt enable */
#ifdef stm32f3
	reg[0] |= 1UL << TXE; /* TXEIE */
#else
	reg[3] |= 1UL << TXE; /* TXEIE */
#endif
}

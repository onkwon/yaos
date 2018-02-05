#include <uart.h>
#include <kernel/interrupt.h>
#include "nrf52.h"
#include "nrf_gpio.h"
#include <pinmap.h>
#undef NVIC_BASE
#undef SCB_BASE
#include <io.h>

static inline void set_uart_port(NRF_UART_Type *reg, struct uart *conf)
{
	if (conf->rx && !conf->npin_rx)
		conf->npin_rx = PIN_UART1_RX;

	if (conf->tx && !conf->npin_tx)
		conf->npin_tx = PIN_UART1_TX;

	gpio_init(conf->npin_rx, GPIO_MODE_INPUT);
	gpio_init(conf->npin_tx, GPIO_MODE_OUTPUT);

	reg->PSELRXD = conf->npin_rx;
	reg->PSELTXD = conf->npin_tx;

	if (conf->flow) {
		nrf_gpio_cfg_input(conf->npin_cts, NRF_GPIO_PIN_NOPULL);
		nrf_gpio_pin_set(conf->npin_rts);
		nrf_gpio_cfg_output(conf->npin_rts);

		reg->PSELRTS = conf->npin_rts;
		reg->PSELCTS = conf->npin_cts;
	}
}

static inline void set_baudrate(NRF_UART_Type *reg, struct uart *conf)
{
	switch (conf->baudrate) {
	case 2400:
		conf->baudrate = UART_BAUDRATE_BAUDRATE_Baud2400;
		break;
	case 4800:
		conf->baudrate = UART_BAUDRATE_BAUDRATE_Baud4800;
		break;
	case 9600:
		conf->baudrate = UART_BAUDRATE_BAUDRATE_Baud9600;
		break;
	case 14400:
		conf->baudrate = UART_BAUDRATE_BAUDRATE_Baud14400;
		break;
	case 19200:
		conf->baudrate = UART_BAUDRATE_BAUDRATE_Baud19200;
		break;
	case 28800:
		conf->baudrate = UART_BAUDRATE_BAUDRATE_Baud28800;
		break;
	case 31250:
		conf->baudrate = UART_BAUDRATE_BAUDRATE_Baud31250;
		break;
	case 38400:
		conf->baudrate = UART_BAUDRATE_BAUDRATE_Baud38400;
		break;
	case 56000:
		conf->baudrate = UART_BAUDRATE_BAUDRATE_Baud56000;
		break;
	case 57600:
		conf->baudrate = UART_BAUDRATE_BAUDRATE_Baud57600;
		break;
	case 76800:
		conf->baudrate = UART_BAUDRATE_BAUDRATE_Baud76800;
		break;
	case 115200:
		conf->baudrate = UART_BAUDRATE_BAUDRATE_Baud115200;
		break;
	case 230400:
		conf->baudrate = UART_BAUDRATE_BAUDRATE_Baud230400;
		break;
	case 250000:
		conf->baudrate = UART_BAUDRATE_BAUDRATE_Baud250000;
		break;
	case 460800:
		conf->baudrate = UART_BAUDRATE_BAUDRATE_Baud460800;
		break;
	case 921600:
		conf->baudrate = UART_BAUDRATE_BAUDRATE_Baud921600;
		break;
	case 1000000:
		conf->baudrate = UART_BAUDRATE_BAUDRATE_Baud1M;
		break;
	default:
		conf->baudrate = UART_BAUDRATE_BAUDRATE_Baud115200;
		break;
	}

	reg->BAUDRATE = conf->baudrate;
}

#define TXDRDY_MASK	0x80
#define RXDRDY_MASK	0x4
#define TXTO_MASK	0x20000

int __uart_open(int channel, struct uart conf)
{
	int nirq = 2;

	if (channel)
		return -ERANGE;

	NRF_UART_Type *reg = (NRF_UART_Type *)NRF_UART0_BASE;

	set_uart_port(reg, &conf);
	set_baudrate(reg, &conf);

	if (conf.parity)
		reg->CONFIG |= 0x7 << 1;
	if (conf.flow)
		reg->CONFIG |= 1;

	reg->EVENTS_RXDRDY = 0;
	reg->EVENTS_TXDRDY = 0;
	reg->EVENTS_RXTO = 0;
	reg->EVENTS_ERROR = 0;
	reg->INTENSET = RXDRDY_MASK;

	nvic_set(nirq, ON);

	reg->ENABLE = 4;
	reg->TASKS_STARTRX = 1;

	return irq2vec(nirq);
}

void __uart_close(int channel)
{
}

int __uart_putc(int channel, int c)
{
	NRF_UART_Type *reg = (NRF_UART_Type *)NRF_UART0_BASE;

	//if (reg->TASKS_STARTTX && !reg->EVENTS_TXDRDY)
	//	return 0;

	reg->EVENTS_TXDRDY = 0;
	reg->TASKS_STARTTX = 1;
	reg->TXD = c;

#if 0
	while (!reg->EVENTS_TXDRDY &&
			(!in_interrupt() || which_context() < NVECTOR_IRQ)) ;
#else
	while (!reg->EVENTS_TXDRDY);
#endif

	return 1;
}

bool __uart_has_rx(int channel)
{
	NRF_UART_Type *reg = (NRF_UART_Type *)NRF_UART0_BASE;
	return (bool)!!reg->EVENTS_RXDRDY;
}

bool __uart_has_tx(int channel)
{
	return false;
	NRF_UART_Type *reg = (NRF_UART_Type *)NRF_UART0_BASE;
	return (bool)!!reg->EVENTS_TXDRDY;
}

int __uart_getc(int channel)
{
	NRF_UART_Type *reg = (NRF_UART_Type *)NRF_UART0_BASE;
	reg->EVENTS_RXDRDY = 0;
	return reg->RXD;
}

void __uart_tx_irq_reset(int channel)
{
	NRF_UART_Type *reg = (NRF_UART_Type *)NRF_UART0_BASE;
	reg->EVENTS_TXDRDY = 0;
}

void __uart_tx_irq_raise(int channel)
{
	return;
}

void __uart_flush(int channel)
{
}

unsigned int __uart_get_baudrate(int channel)
{
	return 0;
}

int __uart_set_baudrate(int channel, unsigned int baudrate)
{
	return 0;
}

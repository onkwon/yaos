#include <usart.h>
#include <stdlib.h>
#include <foundation.h>

#define BUF_SIZE	256
#define USART_NUM	5

#define RXNE		5
#define TXE		7

static struct fifo_t rxq[USART_NUM];
static struct fifo_t txq[USART_NUM];

static struct semaphore rx_lock[USART_NUM];
static struct semaphore tx_lock[USART_NUM];

static void isr_usart();

unsigned brr2reg(unsigned baudrate, unsigned clk)
{
	unsigned fraction, mantissa;

	baudrate /= 100;
	mantissa  = (clk * 10) / (16 * baudrate);
	fraction  = mantissa % 1000;
	mantissa /= 1000;
	fraction  = fraction * 16 / 1000;
	baudrate  = (mantissa << 4) + fraction;

	return baudrate;
}

void usart_open(unsigned channel, struct usart_t arg)
{
	unsigned port, pin, nvector, apb_nbit;

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
		return;
	}

	if (channel == USART1) {
		SET_CLOCK_APB2(ENABLE, apb_nbit); /* USART1 clock enable */

		fifo_init(&rxq[0], malloc(BUF_SIZE), BUF_SIZE);
		fifo_init(&txq[0], malloc(BUF_SIZE), BUF_SIZE);

		semaphore_init(&rx_lock[0], 1);
		semaphore_init(&tx_lock[0], 1);
	} else {
		SET_CLOCK_APB1(ENABLE, apb_nbit); /* USARTn clock enable */

		fifo_init(&rxq[apb_nbit-16], malloc(BUF_SIZE), BUF_SIZE);
		fifo_init(&txq[apb_nbit-16], malloc(BUF_SIZE), BUF_SIZE);

		semaphore_init(&rx_lock[apb_nbit - 16], 1);
		semaphore_init(&tx_lock[apb_nbit - 16], 1);
	}

	SET_PORT_CLOCK(ENABLE, port);

	/* gpio configuration. in case of remapping or UART5, check pinout. */
	SET_PORT_PIN(port, pin, PIN_OUTPUT_50MHZ | PIN_ALT); /* tx */
	SET_PORT_PIN(port, pin+1, PIN_INPUT | PIN_FLOATING); /* rx */

	ISR_REGISTER(nvector, isr_usart);
	SET_IRQ(ON, nvector - 16);

	*(volatile unsigned *)(channel + 0x08) = arg.brr;
	*(volatile unsigned *)(channel + 0x0c) = arg.cr1;
	*(volatile unsigned *)(channel + 0x10) = arg.cr2;
	*(volatile unsigned *)(channel + 0x14) = arg.cr3;
	*(volatile unsigned *)(channel + 0x18) = arg.gtpr;
}

void usart_close(unsigned channel)
{
	/* check if still in transmission. */
	while (!gbi(*(volatile unsigned *)channel, 7)); /* wait until TXE bit set */

	/* Use APB2 peripheral reset register (RCC_APB2RSTR),
	 * or just turn off enable bit of tranceiver, controller and clock. */

	/* Turn off enable bit of transmitter, receiver, and clock.
	 * It leaves port clock, pin, irq, and configuration as set */
	*(volatile unsigned *)(channel + 0x0c) &= ~(
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

void __usart_putc(unsigned channel, int c)
{
	while (!gbi(*(volatile unsigned *)channel, 7)); /* wait until TXE bit set */
	*(volatile unsigned *)(channel + 0x04) = (unsigned)c;
}

void usart_putc(unsigned channel, int c)
{
	unsigned idx = GET_USART_NR(channel);
	unsigned long irq_flag;
	int err;

	do {
		spinlock_irqsave(&tx_lock[idx], &irq_flag);
		err = fifo_put(&txq[idx], c, 1);
		spinlock_irqrestore(&tx_lock[idx], &irq_flag);
	} while (err == -1);

	*(volatile unsigned *)(channel + 0x0c) |= 1 << TXE;
}

int usart_getc(unsigned channel)
{
	int data;
	unsigned idx = GET_USART_NR(channel);
	unsigned long irq_flag;
	
	spinlock_irqsave(&rx_lock[idx], &irq_flag);
	data = fifo_get(&rxq[idx], 1);
	spinlock_irqrestore(&rx_lock[idx], &irq_flag);

	if (data == -1)
		return -1;

	return data & 0xff;
}

int usart_kbhit(unsigned channel)
{
	unsigned idx = GET_USART_NR(channel);

	return (rxq[idx].front != rxq[idx].rear);
}

void usart_fflush(unsigned channel)
{
	unsigned idx = GET_USART_NR(channel);
	unsigned long irq_flag;

	spinlock_irqsave(&rx_lock[idx], &irq_flag);
	fifo_flush(&rxq[idx]);
	spinlock_irqrestore(&rx_lock[idx], &irq_flag);
}

static void isr_usart()
{
	/* IRQ number:
	 * USART1 USART2 USART3 UART4 UART5
	 * 53     54     55     68    69   */
	unsigned nirq, reg;
	unsigned long irq_flag;

	nirq = GET_PSR() & 0x1ff;
	nirq = nirq < 60? nirq - 53 : nirq - 65;
	reg  = nirq? USART2 + ((nirq-1) * 0x400) : USART1;

	if (*(volatile unsigned *)reg & (1 << RXNE)) {
		unsigned rx = *(volatile unsigned *)(reg + 0x04);
		int      err;

		spinlock_irqsave(&rx_lock[nirq], &irq_flag);
		err = fifo_put(&rxq[nirq], rx, 1);
		spinlock_irqrestore(&rx_lock[nirq], &irq_flag);

		if (err == -1) {
			/* overflow */
		}
	}

	if (*(volatile unsigned *)reg & (1 << TXE)) {
		int tx;

		spinlock_irqsave(&tx_lock[nirq], &irq_flag);
		tx = fifo_get(&txq[nirq], 1);
		spinlock_irqrestore(&tx_lock[nirq], &irq_flag);

		if (tx == -1)
			*(volatile unsigned *)(reg + 0x0c) &= ~(1 << TXE);
		else
			*(volatile unsigned *)(reg + 0x04) = (unsigned char)tx;
	}
}

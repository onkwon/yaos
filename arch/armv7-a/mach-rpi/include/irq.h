#ifndef __RPI_INTERRUPT_H__
#define __RPI_INTERRUPT_H__

/*
 * IRQ:
 * 0	- ARM Timer                 -
 * 1	- ARM Mailbox               |
 * 2	- ARM Doorbell 0            |
 * 3	- ARM Doorbell 1            |
 * 4	- GPU0 halted               | basic interrupts
 * 5	- GPU1 halted               |
 * 6	- Illegal access type 1     |
 * 7	- Illegal access type 0     -
 * ...
 * 29	- Aux int(mini uart)
 * ...
 * 43	- i2c_spi_slv_int
 * ...
 * 45	- pwa0
 * 46	- pwa1
 * ...
 * 48	- smi
 * 49	- gpio_int[0]
 * 50	- gpio_int[1]
 * 51	- gpio_int[2]
 * 52	- gpio_int[3]
 * 53	- i2c_int
 * 54	- spi_int
 * 55	- pcm_int
 * ...
 * 57	- uart_int
 */

#define IRQ_TIMER			0
#define IRQ_PEND1			8
#define IRQ_PEND2			9
#define IRQ_AUX				29
#define IRQ_GPIO0			49
#define IRQ_GPIO1			50
#define IRQ_GPIO2			51
#define IRQ_GPIO3			52
#define IRQ_UART			57
#define IRQ_MAX				64

#endif /* __RPI_INTERRUPT_H__ */

#ifndef __RPI_USART_H__
#define __RPI_USART_H__

#include <io.h>

#define USART_CHANNEL_MAX		1
#define __get_usart_active_irq()	0

int  __usart_open(unsigned int channel, unsigned int baudrate);
void __usart_close(unsigned int channel);
int __usart_putc(unsigned int channel, int c);
int __usart_getc(unsigned int channel);
int __usart_check_rx(unsigned int channel);
int __usart_check_tx(unsigned int channel);
void __usart_tx_irq_raise(unsigned int channel);
void __usart_tx_irq_reset(unsigned int channel);
void __usart_flush(unsigned int channel);
unsigned int __usart_get_baudrate(unsigned int channel);
int __usart_set_baudrate(unsigned int channel, unsigned int baudrate);

#endif /* __RPI_USART_H__ */

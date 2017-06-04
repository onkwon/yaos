#ifndef __TYPEDEF_H__
#define __TYPEDEF_H__

typedef struct {
	unsigned int SR;
	unsigned int DR;
	unsigned int BRR;
	unsigned int CR1;
	unsigned int CR2;
	unsigned int CR3;
	unsigned int GTPR;
} __attribute__((packed)) UART_T;

#endif /* __TYPEDEF_H__ */

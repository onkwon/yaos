#ifndef __STM32_DMA2D_H__
#define __STM32_DMA2D_H__

#include <types.h>

#define DMA2D_BASEADDR	0x4002B000

struct dma2d_t {
	reg_t CR;
	reg_t ISR;
	reg_t IFCR;
	reg_t FGMAR;
	reg_t FGOR;
	reg_t BGMAR;
	reg_t BGOR;
	reg_t FGPFCCR;
	reg_t FGCOLR;
	reg_t BGPFCCR;
	reg_t BGCOLR;
	reg_t FGCMAR;
	reg_t BGCMAR;
	reg_t OPFCCR;
	reg_t OCOLR;
	reg_t OMAR;
	reg_t OOR;
	reg_t NLR;
	reg_t LWR;
	reg_t AMTCR;
} __attribute__((packed, aligned(4)));

struct dma2d_t *DMA2D;

void dma2d_init();

#endif /* __STM32_DMA2D_H__ */

#include <asm/mach/board/dma2d.h>
#include <asm/mach/board/lcd.h>
#include <io.h>

struct dma2d_t *DMA2D = (struct dma2d_t *)DMA2D_BASEADDR;

void dma2d_init()
{
	__turn_ahb1_clock(23, ON); /* DMA2D clock enable */

	DMA2D->CR = 3 << 16; // Register-to-memory
	DMA2D->OPFCCR = PF_RGB565;
}

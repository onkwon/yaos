#include "arch/hw_debug.h"
#include "arch/regs.h"
#include "arch/mach/hw_clock.h"

void hw_debug_putc(const int v)
{
	if ((ITM->TCR & 1UL) == 0UL)
		return;
	//if ((ITM->TER & (1UL << ch)) == 0)
	if ((ITM->TER & 1UL) == 0UL)
		return;

	while (ITM->PORT[0].u32 == 0UL) ;
	//if (ITM->PORT[0].u32 == 0)
	//	return 0;

	//ITM->PORT[0].u16 = 0x08 | (c << 8);
	ITM->PORT[0].u8 = (uint8_t)v;

	//return 1;
}

void hw_debug_init(uint32_t ch, uint32_t baudrate)
{
	uint32_t prescaler = (hw_clock_get_hclk() / baudrate) - 1;

	COREDEBUG->DEMCR = 1UL << 24; // enable DWT and ITM

	*((volatile uintptr_t *)0xE0042004UL) = 0x00000027;

	TPIU_SPPR = 0x00000002; // 2: SWO NRZ, 1: SWO Manchester encoding
	TPIU_ACPR = prescaler;

	 /* ITM Lock Access Register, C5ACCE55 enables more write access to Control Register 0xE00 :: 0xFFC */
	*((volatile uintptr_t *)(ITM_BASE + 0x00FB0)) = 0xC5ACCE55;

	ITM->TCR = (0x7fUL << 16) | (1UL << 4) | (1UL << 2) | (1UL << 0);
	ITM->TPR = 0xf;
	ITM->TER = ch;

	DWT_CTRL = 0x400003FE;
	TPIU_FFCR = 0x00000100;
}

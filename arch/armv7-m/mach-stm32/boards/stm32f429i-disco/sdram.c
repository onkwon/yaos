#include "sdram.h"
#include <foundation.h>
#include <kernel/timer.h>

static inline void sdram_gpio_init()
{
	//__turn_ahb3_clock(0, ON);
	SET_CLOCK_AHB3(0, ON);

	gpio_init(PIN_FMC_D0, GPIO_MODE_ALT | gpio_altfunc(12) | GPIO_SPD_FASTEST);
	gpio_init(PIN_FMC_D1, GPIO_MODE_ALT | gpio_altfunc(12) | GPIO_SPD_FASTEST);
	gpio_init(PIN_FMC_D2, GPIO_MODE_ALT | gpio_altfunc(12) | GPIO_SPD_FASTEST);
	gpio_init(PIN_FMC_D3, GPIO_MODE_ALT | gpio_altfunc(12) | GPIO_SPD_FASTEST);
	gpio_init(PIN_FMC_D4, GPIO_MODE_ALT | gpio_altfunc(12) | GPIO_SPD_FASTEST);
	gpio_init(PIN_FMC_D5, GPIO_MODE_ALT | gpio_altfunc(12) | GPIO_SPD_FASTEST);
	gpio_init(PIN_FMC_D6, GPIO_MODE_ALT | gpio_altfunc(12) | GPIO_SPD_FASTEST);
	gpio_init(PIN_FMC_D7, GPIO_MODE_ALT | gpio_altfunc(12) | GPIO_SPD_FASTEST);
	gpio_init(PIN_FMC_D8, GPIO_MODE_ALT | gpio_altfunc(12) | GPIO_SPD_FASTEST);
	gpio_init(PIN_FMC_D9, GPIO_MODE_ALT | gpio_altfunc(12) | GPIO_SPD_FASTEST);
	gpio_init(PIN_FMC_D10, GPIO_MODE_ALT | gpio_altfunc(12) | GPIO_SPD_FASTEST);
	gpio_init(PIN_FMC_D11, GPIO_MODE_ALT | gpio_altfunc(12) | GPIO_SPD_FASTEST);
	gpio_init(PIN_FMC_D12, GPIO_MODE_ALT | gpio_altfunc(12) | GPIO_SPD_FASTEST);
	gpio_init(PIN_FMC_D13, GPIO_MODE_ALT | gpio_altfunc(12) | GPIO_SPD_FASTEST);
	gpio_init(PIN_FMC_D14, GPIO_MODE_ALT | gpio_altfunc(12) | GPIO_SPD_FASTEST);
	gpio_init(PIN_FMC_D15, GPIO_MODE_ALT | gpio_altfunc(12) | GPIO_SPD_FASTEST);

	gpio_init(PIN_FMC_A0, GPIO_MODE_ALT | gpio_altfunc(12) | GPIO_SPD_FASTEST);
	gpio_init(PIN_FMC_A1, GPIO_MODE_ALT | gpio_altfunc(12) | GPIO_SPD_FASTEST);
	gpio_init(PIN_FMC_A2, GPIO_MODE_ALT | gpio_altfunc(12) | GPIO_SPD_FASTEST);
	gpio_init(PIN_FMC_A3, GPIO_MODE_ALT | gpio_altfunc(12) | GPIO_SPD_FASTEST);
	gpio_init(PIN_FMC_A4, GPIO_MODE_ALT | gpio_altfunc(12) | GPIO_SPD_FASTEST);
	gpio_init(PIN_FMC_A5, GPIO_MODE_ALT | gpio_altfunc(12) | GPIO_SPD_FASTEST);
	gpio_init(PIN_FMC_A6, GPIO_MODE_ALT | gpio_altfunc(12) | GPIO_SPD_FASTEST);
	gpio_init(PIN_FMC_A7, GPIO_MODE_ALT | gpio_altfunc(12) | GPIO_SPD_FASTEST);
	gpio_init(PIN_FMC_A8, GPIO_MODE_ALT | gpio_altfunc(12) | GPIO_SPD_FASTEST);
	gpio_init(PIN_FMC_A9, GPIO_MODE_ALT | gpio_altfunc(12) | GPIO_SPD_FASTEST);
	gpio_init(PIN_FMC_A10, GPIO_MODE_ALT | gpio_altfunc(12) | GPIO_SPD_FASTEST);
	gpio_init(PIN_FMC_A11, GPIO_MODE_ALT | gpio_altfunc(12) | GPIO_SPD_FASTEST);

	gpio_init(PIN_FMC_SDCKE1, GPIO_MODE_ALT | gpio_altfunc(12) | GPIO_SPD_FASTEST);
	gpio_init(PIN_FMC_SDNE1, GPIO_MODE_ALT | gpio_altfunc(12) | GPIO_SPD_FASTEST);
	gpio_init(PIN_FMC_SDNWE, GPIO_MODE_ALT | gpio_altfunc(12) | GPIO_SPD_FASTEST);
	gpio_init(PIN_FMC_NBL0, GPIO_MODE_ALT | gpio_altfunc(12) | GPIO_SPD_FASTEST);
	gpio_init(PIN_FMC_NBL1, GPIO_MODE_ALT | gpio_altfunc(12) | GPIO_SPD_FASTEST);
	gpio_init(PIN_FMC_NRAS, GPIO_MODE_ALT | gpio_altfunc(12) | GPIO_SPD_FASTEST);
	gpio_init(PIN_FMC_NCAS, GPIO_MODE_ALT | gpio_altfunc(12) | GPIO_SPD_FASTEST);
	gpio_init(PIN_FMC_SDCLK, GPIO_MODE_ALT | gpio_altfunc(12) | GPIO_SPD_FASTEST);
	gpio_init(PIN_FMC_BA0, GPIO_MODE_ALT | gpio_altfunc(12) | GPIO_SPD_FASTEST);
	gpio_init(PIN_FMC_BA1, GPIO_MODE_ALT | gpio_altfunc(12) | GPIO_SPD_FASTEST);
}

/* SDRAM clock = HCLK / 2 = 120 / 2 = 60MHz
 *
 * TMRD - 2 clock cycle = 1
 * TXSR - 70ns		= 4 = 83ns = 16.6ns * 5
 * TRAS - 42ns		= 2 = 50ns = 16.6ns * 3
 * TRC  - 63ns		= 3 = 66ns = 16.6ns * 4
 * TWR  - 2 clock cycle	= 1
 * TRP  - 15ns		= 0 = 16.6ns
 * TRCD - 15ns		= 0 = 16.6ns
 *
 * Row addressing - A0..A11
 * Column addressing - A0..A7
 * 16-bit bus width
 * 1M x 16 x 4 banks
 *
 * RPIPE  - 1 clock cycle
 * RBURST - not bursts
 * SDCLK  - 2xHCLK periods
 * WP     - allowed
 * CAS    - 2 clock cycle
 * NB     - 4 internal banks
 * NR     - 12-bit
 * NC     - 8-bit
 *
 * Refresh timer count = 917
 *  = (SDRAM refresh rate * SDRAM clock frequency) - 20
 *  = 15.62us * 60MHz - 20 = 917
 *  SDRAM refresh rate = SDRAM refresh period / number of rows
 *   = 64ms / 4096 = 15.62us
 * */
void sdram_init()
{
	sdram_gpio_init();

	struct sdram_t *sdram = (struct sdram_t *)FMC_SDRAM_BASEADDR;

	sdram->SDCR[0] = (1 << RPIPE) | (2 << SDCLK);
	sdram->SDCR[1] = (2 << CAS) | (1 << NB) | (1 << MWID) | (1 << NR);
	sdram->SDTR[0] = (3 << TRC);
	sdram->SDTR[1] = (1 << TWR) | (2 << TRAS) | (4 << TXSR) | (1 << TMRD);
#if 0
	sdram->SDCR[BANK] = (1 << RPIPE) | (2 << SDCLK) | (2 << CAS) |
		(1 << NB) | (1 << MWID) | (1 << NR);
	sdram->SDTR[BANK] = (1 << TWR) | (3 << TRC) | (2 << TRAS) |
		(4 << TXSR) | (1 << TMRD);
	sdram->SDCR[0] = (1 << RPIPE) | (2 << SDCLK) | (2 << CAS) |
		(1 << NB) | (1 << MWID) | (1 << NR);
	sdram->SDTR[0] = (1 << TWR) | (3 << TRC) | (2 << TRAS) |
		(4 << TXSR) | (1 << TMRD);
#endif

	// 1. clock enable
	sdram->SDCMR = (CMD_CLOCK << MODE) | (1 << CTB2);
	while (gbi(sdram->SDSR, BUSY));
	// 2. 100us minimum delay
	udelay(100);
	// 3. pall
	sdram->SDCMR = (CMD_PALL << MODE) | (1 << CTB2);
	while (gbi(sdram->SDSR, BUSY));
	// 4. auto refresh
	sdram->SDCMR = (CMD_AUTOREFRESH << MODE) | (1 << CTB2) | (3 << NRFS);
	while (gbi(sdram->SDSR, BUSY));
	// 5. external memory mode register
	sdram->SDCMR = (CMD_LOAD_MODREG << MODE) | (1 << CTB2) | (0x220 << MRD);
	while (gbi(sdram->SDSR, BUSY));
	// 6. refresh rate counter
	sdram->SDRTR = 917 << 1;
}

#if 0
static void sdram_test()
{
	sdram_init();

	volatile unsigned int *mem = (unsigned int *)SDRAM_BASEADDR;
	int i;

#define MAGIC	0xdeafc0de
	for (i = 0; i < 1024; i++)
		mem[i] = MAGIC;

	mdelay(500);

	for (i = 0; i < 1024; i++) {
		if (mem[i] != MAGIC)
			notice("err %08x %08x", &mem[i], mem[i]);
	}
}
REGISTER_TASK(sdram_test, TASK_KERNEL, DEFAULT_PRIORITY, STACK_SIZE_DEFAULT);
#endif

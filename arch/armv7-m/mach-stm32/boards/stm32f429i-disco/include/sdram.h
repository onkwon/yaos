/*
 * "[...] Sincerity (comprising truth-to-experience, honesty towards the self,
 * and the capacity for human empathy and compassion) is a quality which
 * resides within the laguage of literature. It isn't a fact or an intention
 * behind the work [...]"
 *
 *             - An introduction to Literary and Cultural Theory, Peter Barry
 *
 *
 *                                                   o8o
 *                                                   `"'
 *     oooo    ooo  .oooo.    .ooooo.   .oooo.o     oooo   .ooooo.
 *      `88.  .8'  `P  )88b  d88' `88b d88(  "8     `888  d88' `88b
 *       `88..8'    .oP"888  888   888 `"Y88b.       888  888   888
 *        `888'    d8(  888  888   888 o.  )88b .o.  888  888   888
 *         .8'     `Y888""8o `Y8bod8P' 8""888P' Y8P o888o `Y8bod8P'
 *     .o..P'
 *     `Y8P'                   Kyunghwan Kwon <kwon@yaos.io>
 *
 *  Welcome aboard!
 */

#ifndef __STM32_SDRAM_H__
#define __STM32_SDRAM_H__

#include <types.h>

#define FMC_CONTROL_BASEADDR	0xA0000000U
#define FMC_SDRAM_BASEADDR	(FMC_CONTROL_BASEADDR + 0x140U)
#define BANK			1 // bank2

#define SDRAM_BASEADDR		0xD0000000

struct sdram_t {
	reg_t SDCR[2];	// 0x140
	//reg_t SDCR2;	// 0x144
	reg_t SDTR[2];	// 0x148
	//reg_t SDTR2;	// 0x14c
	reg_t SDCMR;	// 0x150
	reg_t SDRTR;	// 0x154
	reg_t SDSR;	// 0x158
} __attribute__((packed, aligned(4)));

enum FMC_SDCR_BIT {
	RPIPE	= 13,
	RBURST	= 12,
	SDCLK	= 10,
	WP	= 9,
	CAS	= 7,
	NB	= 6,
	MWID	= 4,
	NR	= 2,
	NC	= 0,
};

enum FMC_SDTR_BIT {
	TRCD	= 24,
	TRP	= 20,
	TWR	= 16,
	TRC	= 12,
	TRAS	= 8,
	TXSR	= 4,
	TMRD	= 0,
};

enum FMC_SDCMR_BIT {
	MRD	= 9,
	NRFS	= 5,
	CTB1	= 4,
	CTB2	= 3,
	MODE	= 0,
};

enum FMC_SDCMR_MODE_BIT {
	CMD_NORMAL	= 0,
	CMD_CLOCK	= 1,
	CMD_PALL	= 2,
	CMD_AUTOREFRESH	= 3,
	CMD_LOAD_MODREG	= 4,
	CMD_SELFREFRESH	= 5,
	CMD_POWERDOWN	= 6,
};

enum FMC_SDSR_BIT {
	BUSY	= 5,
	MODES2	= 3,
	MODES1	= 1,
	RE	= 0,
};

#endif /* __STM32_SDRAM_H__ */

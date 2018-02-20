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

#ifndef __STM32_SPI_H__
#define __STM32_SPI_H__

#include <types.h>
#include <stdint.h>

#define SPI1_BASEADDR	0x40013000U
#define SPI4_BASEADDR	0x40013400U
#define SPI5_BASEADDR	0x40015000U
#define SPI6_BASEADDR	0x40015400U

#define SPI2_BASEADDR	0x40003800U
#define SPI3_BASEADDR	0x40003c00U

enum spi_channel {
	SPI1	= SPI1_BASEADDR,
	SPI2	= SPI2_BASEADDR,
	SPI3	= SPI3_BASEADDR,
	SPI4	= SPI4_BASEADDR,
	SPI5	= SPI5_BASEADDR,
	SPI6	= SPI6_BASEADDR,
};

struct spi_t {
	reg_t CR1;
	reg_t CR2;
	reg_t SR;
	reg_t DR;
	reg_t CRCPR;
	reg_t RXCRCR;
	reg_t TXCRCR;
	reg_t I2SCFGR;
	reg_t I2SPR;
} __attribute__((packed, aligned(4)));

enum SPI_CR1_BIT {
	BIDIMODE	= 15,
	BIDIOE		= 14,
	CRCEN		= 13,
	CRCNEXT		= 12,
	DFF		= 11,
	RXONLY		= 10,
	SSM		= 9,
	SSI		= 8,
	LSBFIRST	= 7,
	SPE		= 6,
	BR		= 3,
	MSTR		= 2,
	CPOL		= 1,
	CPHA		= 0,
};

enum SPI_SR_BIT {
	FRE		= 8,
	BSY		= 7,
	OVR		= 6,
	MODF		= 5,
	CRCERR		= 4,
	UDR		= 3,
	CHSIDE		= 2,
	TXE		= 1,
	RXNE		= 0,
};

enum spi_mode {
	SPI_SLAVE	= 0,
	SPI_MASTER	= 1,
};

enum spi_opt {
	SPI_NSS_HARD	= 0,
	SPI_NSS_SOFT	= 1,
};

void spi_init(enum spi_channel ch, enum spi_mode mode, uint32_t freq_khz, int opt);
void spi_write_byte(enum spi_channel ch, uint8_t byte);

#endif /* __STM32_SPI_H__ */

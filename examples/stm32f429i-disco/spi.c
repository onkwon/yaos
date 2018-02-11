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

#include "spi.h"
#include <io.h>
#include <foundation.h>
#include <bitops.h>
#include <kernel/timer.h>

void spi_write_byte(enum spi_channel ch, uint8_t byte)
{
	struct spi_t *spi = (struct spi_t *)ch;
	unsigned int tmp, tout;

	sbi(spi->CR1, SPE);

	set_timeout(&tout, msec_to_ticks(1000));
	while (!gbi(spi->SR, TXE) && !is_timeout(tout));
	spi->DR = byte;
	tmp = spi->DR;
	(void)tmp;
	while (gbi(spi->SR, BSY) && !is_timeout(tout));

	cbi(spi->CR1, SPE);
}

/* SPI clock and its GPIOs must be configured first before calling this
 * function. */
void spi_init(enum spi_channel ch, enum spi_mode mode, uint32_t freq_khz, int opt)
{
	struct spi_t *spi = (struct spi_t *)ch;
	unsigned int div, pre, tmp = 0;

	cbi(spi->CR1, SPE);

	sbi(tmp, mode * MSTR);
	sbi(tmp, opt * SSM);
	sbi(tmp, opt * SSI);

	div = get_pclk2() / freq_khz / 1000;
	pre = 1 << (fls(div) - 1);
	pre <<= 1 * !!(div - pre);
	if (pre < 2 || pre > 256) {
		error("not supported frequency %d", freq_khz);
		pre = (pre < 2)? 2 : 256;
	}
	tmp |= ((log2(pre)-1) << BR);

	spi->CR1 = tmp;

	sbi(spi->CR1, SPE);
}

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

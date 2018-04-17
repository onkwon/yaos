#include <foundation.h>
#include <drivers/spi.h>
#include <drivers/gpio.h>
#include <kernel/timer.h>

#include "epd.h"
#include "font.h"

/* Waveshare 1.54inch e-Paper display
 *
 * NAME | DIR |  PIN  | DESC
 * -----------------------
 * DIN  | out | PA.7  | SPI1 MOSI
 * CLK  | out | PA.5  | SPI1 SCK
 * CS   | out | PB.0  | SPI chip select (Low active)
 * DC   | out | PB.1  | Data/Command control pin (High for data, and low for command)
 * RST  | out | PB.10 | External reset pin (Low for reset)
 * BUSY | in  | PB.11 | Busy state output pin (Low for busy)
 */

static inline void pin_init()
{
	gpio_init(PIN_SPI1_SCK, GPIO_MODE_ALT | GPIO_SPD_FASTEST);
	gpio_init(PIN_SPI1_MOSI, GPIO_MODE_ALT | GPIO_SPD_FASTEST);
	gpio_init(PIN_EPD_CS, GPIO_MODE_OUTPUT | GPIO_SPD_FASTEST);
	gpio_init(PIN_EPD_DC, GPIO_MODE_OUTPUT | GPIO_SPD_FASTEST);
	gpio_init(PIN_EPD_RST, GPIO_MODE_OUTPUT | GPIO_SPD_FASTEST);
	gpio_init(PIN_EPD_BUSY, GPIO_MODE_INPUT | GPIO_SPD_FASTEST);
	__turn_apb2_clock(12, ON);
}

static void epaper1_54()
{
	uint8_t *fb;
	if ((fb = kmalloc(EPD_WIDTH * EPD_HEIGHT / 8)) == NULL)
		notice("kmalloc failed");

	pin_init();
	spi_init(SPI1, SPI_MASTER, 4500, SPI_NSS_SOFT); 
	epd_init(fb);

	epd_font(&BebasNeue120B);

	epd_clear(0);
#if 1
	sleep(1);
	epd_clear(0);
	sleep(1);
	epd_mode_set(EPD_PARTIAL_UPDATE);
#endif
	for (int i = 0; i < 10; i++) {
		epd_putc(77, 40, i + '0');
		sleep(1);
		epd_draw_pixel(i, 10, 1, 0);
	}

	int fd, v = 0;
	const char *pathname = "/dev/led" def2str(PIN_STATUS_LED);

	if ((fd = open(pathname, O_WRONLY)) <= 0) {
		error("can not open %d\n", fd);
		return;
	}

	while (1) {
		v ^= 1;
		write(fd, &v, 1);
		msleep(500);
	}

	close(fd);
}
REGISTER_TASK(epaper1_54, TASK_KERNEL, DEFAULT_PRIORITY, STACK_SIZE_DEFAULT);

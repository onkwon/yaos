#include "epd.h"
#include <drivers/gpio.h>
#include <drivers/spi.h>
#include <kernel/timer.h>

#define LUT_SIZE	30

static uint8_t *_fb = NULL;
static const struct font_t *_font = NULL;

enum epd_cmd_t {
	DRIVER_OUTPUT_CONTROL		= 0x01,
	BOOSTER_SOFT_START_CONTROL	= 0x0C,
	GATE_SCAN_START_POSITION	= 0x0F,
	DEEP_SLEEP_MODE			= 0x10,
	DATA_ENTRY_MODE_SETTING		= 0x11,
	SW_RESET			= 0x12,
	TEMPERATURE_SENSOR_CONTROL	= 0x1A,
	MASTER_ACTIVATION		= 0x20,
	DISPLAY_UPDATE_CONTROL1		= 0x21,
	DISPLAY_UPDATE_CONTROL2		= 0x22,
	WRITE_RAM			= 0x24,
	WRITE_VCOM_REGISTER		= 0x2C,
	WRITE_LUT_REGISTER		= 0x32,
	SET_DUMMY_LINE_PERIOD		= 0x3A,
	SET_GATE_TIME			= 0x3B,
	BORDER_WAVEFORM_CONTROL		= 0x3C,
	SET_RAM_X_ADDR			= 0x44,
	SET_RAM_Y_ADDR			= 0x45,
	SET_RAM_X_ADDR_COUNTER		= 0x4E,
	SET_RAM_Y_ADDR_COUNTER		= 0x4F,
	TERMINATE_FRAME_READ_WRITE	= 0xFF,
};

enum disp_comm_t {
	CMD	= 0,
	DAT	= 1,
};

static const uint8_t epd_lut_full_update[] =
{
	0x02, 0x02, 0x01, 0x11, 0x12, 0x12, 0x22, 0x22,
	0x66, 0x69, 0x69, 0x59, 0x58, 0x99, 0x99, 0x88,
	0x00, 0x00, 0x00, 0x00, 0xF8, 0xB4, 0x13, 0x51,
	0x35, 0x51, 0x51, 0x19, 0x01, 0x00
};

static const uint8_t epd_lut_partial_update[] =
{
	0x10, 0x18, 0x18, 0x08, 0x18, 0x18, 0x08, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x13, 0x14, 0x44, 0x12,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static inline void stall()
{
	while (gpio_get(PIN_EPD_BUSY));
}

static inline void send(enum disp_comm_t type, uint8_t dat)
{
	gpio_put(PIN_EPD_DC, !!type);

	gpio_put(PIN_EPD_CS, LOW);
	spi_write_byte(SPI1, dat);
	gpio_put(PIN_EPD_CS, HIGH);
}

static inline void epd_lut_set(int mode)
{
	const uint8_t *p;
	int i;

	if (mode == EPD_FULL_UPDATE)
		p = epd_lut_full_update;
	else
		p = epd_lut_partial_update;

	send(CMD, WRITE_LUT_REGISTER);
	for (i = 0; i < LUT_SIZE; i++)
		send(DAT, p[i]);
}

static inline void epd_mem_area_set(int x1, int y1, int x2, int y2)
{
	send(CMD, SET_RAM_X_ADDR);
	send(DAT, (x1 >> 3) & 0xff); // multiple of 8
	send(DAT, (x2 >> 3) & 0xff); // multiple of 8
	send(CMD, SET_RAM_Y_ADDR);
	send(DAT, y1 & 0xff);
	send(DAT, (y1 >> 8) & 0xff);
	send(DAT, y2 & 0xff);
	send(DAT, (y2 >> 8) & 0xff);
}

static inline void epd_mem_pointer_set(int x, int y)
{
	send(CMD, SET_RAM_X_ADDR_COUNTER);
	send(DAT, (x >> 3) & 0xff);
	send(CMD, SET_RAM_Y_ADDR_COUNTER);
	send(DAT, y & 0xff);
	send(DAT, (y >> 8) & 0xff);
	stall();
}

static inline void epd_upload(void *img, int x, int y, int width, int height)
{
	uint8_t *p;

	x = x / 8 * 8;
	width = width / 8 * 8;
	p = img;

	epd_mem_area_set(x, y, x + width - 1, y + height - 1);
	epd_mem_pointer_set(x, y);
	send(CMD, WRITE_RAM);

	for (int i = 0; i < width * height / 8; i++)
		send(DAT, p[i]);
}

static inline void _epd_init()
{
	gpio_put(PIN_EPD_RST, LOW);
	msleep(200);
	gpio_put(PIN_EPD_RST, HIGH);
	msleep(200);

	send(CMD, DRIVER_OUTPUT_CONTROL);
	send(DAT, (EPD_HEIGHT - 1) & 0xff);
	send(DAT, ((EPD_HEIGHT - 1) >> 8) & 0xff);
	send(DAT, 0);
	send(CMD, BOOSTER_SOFT_START_CONTROL);
	send(DAT, 0xD7);
	send(DAT, 0xD6);
	send(DAT, 0x9D);
	send(CMD, WRITE_VCOM_REGISTER);
	send(DAT, 0xA8);
	send(CMD, SET_DUMMY_LINE_PERIOD);
	send(DAT, 0x1A);
	send(CMD, SET_GATE_TIME);
	send(DAT, 0x08);
	send(CMD, DATA_ENTRY_MODE_SETTING);
	send(DAT, 0x03);
	epd_lut_set(EPD_FULL_UPDATE);
}

void epd_update()
{
	send(CMD, DISPLAY_UPDATE_CONTROL2);
	send(DAT, 0xC4);
	send(CMD, MASTER_ACTIVATION);
	send(CMD, TERMINATE_FRAME_READ_WRITE);
	stall();
}

void epd_font(const struct font_t *font)
{
	_font = font;
}

void epd_clear(int color)
{
	unsigned int *p = (unsigned int *)_fb;

	color = color * 0xffffffff;

	for (int i = 0; i < EPD_WIDTH * EPD_HEIGHT / 8 / 4; i++)
		p[i] = color;

	epd_upload(_fb, 0, 0, EPD_WIDTH, EPD_HEIGHT);
	epd_update();
}

void epd_draw_pixel(int x, int y, int color, int rotate)
{
	int t = x;

	switch (rotate) {
	case EPD_ROTATE_90:
		x = EPD_WIDTH - y;
		y = t;
		break;
	case EPD_ROTATE_180:
		x = EPD_WIDTH - x;
		y = EPD_HEIGHT - y;
		break;
	case EPD_ROTATE_270:
		x = y;
		y = EPD_HEIGHT - t;
	default:
		break;
	}

	if (x < 0 || y < 0 || x >= EPD_WIDTH || y >= EPD_HEIGHT)
		return;

	if (color)
		_fb[(y * EPD_WIDTH + x) / 8] |= 0x80 >> (x % 8);
	else
		_fb[(y * EPD_WIDTH + x) / 8] &= ~(0x80 >> (x % 8));
}

void epd_putc(int x, int y, char c)
{
	if (!_font || c < _font->start || c > _font->end)
		return;

	const uint8_t *bitmap = _font->data[c - _font->start];
	int size = _font->height * _font->width; /* bit */
	int width = (_font->width + 7) / 8 * 8;
	int i, pos;

#if 0
	for (i = 0; i < size; i += 8) {
		pos = ((y + i / width) * EPD_WIDTH) / 8 + (x / 8) + i % width / 8;
		_fb[pos] = bitmap[i / 8];
	}
#else
	int color;

	pos = x;
	for (i = 0; i < size; i++) {
		color = !!(bitmap[i / 8] & (0x80 >> i % 8));
		epd_draw_pixel(x, y, color, 0);

		if (!((++x - pos) % width)) {
			x = pos;
			y++;
		}
	}
#endif

#if 0
	//epd_upload((void *)bitmap, x, y, width, _font->height);
#else
	epd_upload(_fb, 0, 0, EPD_WIDTH, EPD_HEIGHT);
#endif
	epd_update();
}

void epd_mode_set(int mode)
{
	epd_lut_set(mode);
}

void epd_sleep()
{
	send(CMD, DEEP_SLEEP_MODE);
	stall();
}

void epd_wakeup()
{
	_epd_init();
}

void epd_init(void *fb)
{
	_epd_init();
	_fb = fb;
}

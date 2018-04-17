#ifndef __EPD_H__
#define __EPD_H__

#include <stdint.h>

#define EPD_WIDTH	200
#define EPD_HEIGHT	200

enum {
	EPD_PARTIAL_UPDATE	= 0,
	EPD_FULL_UPDATE		= 1,
};

enum {
	EPD_ROTATE_NONE		= 0,
	EPD_ROTATE_90,
	EPD_ROTATE_180,
	EPD_ROTATE_270,
};

#include "font.h"

void epd_init(void *fb);
void epd_sleep();
void epd_wakeup();
void epd_mode_set(int mode);

void epd_clear(int color);
void epd_font(const struct font_t *font);
void epd_putc(int x, int y, char c);

void epd_draw_pixel(int x, int y, int color, int rotate);

#endif /* __EPD_H__ */

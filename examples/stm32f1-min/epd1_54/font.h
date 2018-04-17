#ifndef __FONT_H__
#define __FONT_H__

#include <stdint.h>

struct font_t {
	int height;
	int width;
	uint8_t start, end;
	const uint8_t *data[95]; /* ascii 32 ~ 126 */ 
};

const struct font_t BebasNeue120B;

#endif /* __FONT_H__ */

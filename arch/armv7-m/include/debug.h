#ifndef __YAOS_HW_DEBUG_H__
#define __YAOS_HW_DEBUG_H__

#include <stdint.h>

void hw_debug_init(uint32_t ch, uint32_t baudrate);
void hw_debug_putc(const int v);

#endif /* __YAOS_HW_DEBUG_H__ */

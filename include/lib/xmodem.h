#ifndef __XMODEM_H__
#define __XMODEM_H__

#include <stdint.h>

#define XMODEM_DATA_SIZE	128
#define XMODEM_META_SIZE	4
#define XMODEM_PACKET_SIZE	(XMODEM_DATA_SIZE + XMODEM_META_SIZE)

#define XMODEM_RETRY_MAX	10

int xmodem_receive(void *dst, int n, int (*get)(), void (*put)(uint8_t));

#endif /* __XMODEM_H__ */

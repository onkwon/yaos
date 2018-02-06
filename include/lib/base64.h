#ifndef __BASE64_H__
#define __BASE64_H__

#include <stddef.h>

size_t base64_encode(unsigned char *dst, const unsigned char *src, size_t slen);
size_t base64_decode(unsigned char *dst, const unsigned char *src, size_t slen);

#endif /* __BASE64_H__ */

#ifndef __STRING_H__
#define __STRING_H__

#include <types.h>

#define c2i(c)		(c >= 'a' && c <= 'f'? c - 87 : \
			c >= 'A' && c <= 'F'? c - 55 : c - '0')

char *itoa(int v, char *buf, unsigned int base, size_t n);
int atoi(const char *s);
int strncmp(const char *s1, const char *s2, size_t n);
int strcmp(const char *s1, const char *s2);
char *strncpy(char *d, const char *s, size_t n);
size_t strlen(const char *s);
size_t strnlen(const char *s, size_t n);
char *strtok(char *line, const char *const token);
unsigned int toknum(const char *line, const char *const token);
char *strchr(char *s, const char c);

extern size_t sprintf(char *out, const char *format, ...);
extern size_t snprintf(char *out, size_t n, const char *format, ...);

#endif /* __STRING_H__ */

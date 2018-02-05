#ifndef __STRING_H__
#define __STRING_H__

#include <types.h>

#define c2i(c)		((c) >= 'a' && (c) <= 'f'? (c) - 87 : \
			(c) >= 'A' && (c) <= 'F'? (c) - 55 : (c) - '0')
#define isdigit(c)	((c) >= '0' && (c) <= '9')

size_t itos(int v, char *buf, int base, size_t n);
char *itoa(int v, char *buf, unsigned int base, size_t n);
int strtoi(const char *s, int base);
int atoi(const char *s);
double atof(const char *s);
size_t ftos(double v, char *buf, int flen, size_t maxlen);
int strncmp(const char *s1, const char *s2, size_t n);
int strcmp(const char *s1, const char *s2);
char *strncpy(char *d, const char *s, size_t n);
char *strcpy(char *d, const char *s);
size_t strlen(const char *s);
size_t strnlen(const char *s, size_t n);
char *strtok(char *line, const char *const token);
unsigned int toknum(const char *line, const char *const token);
char *strchr(char *s, const char c);
char *strstr(const char *string, const char *word);

void *memcpy(void *dst, const void *src, size_t len);
void *memset(void *src, int c, size_t len);

#endif /* __STRING_H__ */

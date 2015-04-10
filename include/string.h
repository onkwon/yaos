#ifndef __STRING_H__
#define __STRING_H__

#include "types.h"

#define c2i(c)	(c >= 'a' && c <= 'f'? c - 87 : \
		c >= 'A' && c <= 'F'? c - 55 : c - '0')

int      atoi    (char *s);
int      strncmp (const char *s1, const char *s2, size_t n);
int      strcmp  (const char *s1, const char *s2);
char    *strncpy (char *s1, const char *s2, size_t n);
size_t   strlen  (const char *s);
char    *strtok  (char *line, const char *const token);
unsigned toknum  (const char *line, const char *const token);
char    *strchr  (char *s, const char c);

#endif /* __STRING_H__ */

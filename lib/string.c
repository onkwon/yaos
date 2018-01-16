#include <string.h>

/* TODO: Support all double-precision numbers
 * It only gets a few small floating-point numbers correctly and has bad
 * rounding erros. */
double atof(const char *s)
{
#ifdef CONFIG_FLOAT
	double v;
	int integer, decimal, *p;
	unsigned int ndigit;
	int sign;

	integer = decimal = 0;
	ndigit  = 1;
	sign    = 1;

	if (*s == '-') {
		sign = -1;
		s++;
	}

	for (p = &integer; *s; s++) {
		if (*s == '.') {
			p = &decimal;
			ndigit = 1;
			continue;
		}

		*p *= 10;
		*p += *s - '0';
		ndigit *= 10;
	}

	v = (double)integer;

	if (decimal)
		v += (double)decimal / ndigit;

	return v * sign;
#else
	return 0;
#endif
}

size_t ftos(double v, char *buf, int flen, size_t maxlen)
{
#ifdef CONFIG_FLOAT
	double f;
	int i;
	char c;
	size_t off = 0;

	if (v < 0) {
		buf[off++] = '-';
		v = -v;
	}

	f = v;
	i = (int)f;
	off += itos(i, &buf[off], 10, maxlen - off - 1);
	buf[off++] = '.';

	while ((f = f - (double)i) != (double)0.0 && off < maxlen-1) {
		c = (int)(f * 10) + '0';
		if (!isdigit(c))
			break;

		buf[off++] = c;
		v *= 10;
		f = v;
		i = (int)f;

		if (--flen == 0)
			break;
	}

	/*
	while (flen-- > 0 && off < maxlen-1)
		buf[off++] = '0';
	*/

	if (buf[off-1] == '.')
		off--;

	buf[off] = '\0';

	return off;
#else
	return 0;
#endif
}

int strtoi(const char *s, int base)
{
	unsigned int digit;
	int v = 0;

	while (*s) {
		digit = *s - '0';

		if (digit > ('Z' - '0')) /* lower case */
			digit = *s - 'a' + 10;
		else if (digit > 9) /* upper case */
			digit = *s - 'A' + 10;

		if (digit >= base)
			break;

		v = v * base + digit;
		s++;
	}

	return v;
}

#define BASE_MAX	16

/* TODO: Get the result from the first digits
 * if n is less than the actual string length as the converting result,
 * it saves from the last digits, not from the first digits. */
size_t itos(int v, char *buf, int base, size_t n)
{
	unsigned int u;
	bool is_negative;
	size_t i;
	char *p, t;

	if (!buf || base > BASE_MAX)
		return 0;

	is_negative = false;
	p = buf;
	n--; /* to preserve a null byte */

	if (v < 0 && base == 10) {
		is_negative = true;
		v = -v;
		n--;
		*p++ = '-';
	}

	u = (unsigned int)v;

	for (i = 0; u && (i < n); i++) {
		t = "0123456789abcdef"[u % base];
		u /= base;
		*p++ = t;
	}

	if (!i) { /* in case of u == 0 */
		*p++ = '0';
		i = 1;
	}

	*p = '\0';
	n = i; /* Note that the local variable, n, is redefined here */
	p = buf + is_negative;

	for (i = 0; i < (n >> 1); i++) {
		t = p[i];
		p[i] = p[n-i-1];
		p[n-i-1] = t;
	}

	return n + is_negative;
}

char *itoa(int v, char *buf, unsigned int base, size_t n)
{
	unsigned int u;
	char *s;
	bool is_negative;

	if (!buf || base > BASE_MAX)
		return NULL;

	s = &buf[n-1];
	*s = '\0';
	is_negative = false;

	if ((v < 0) && (base == 10)) {
		is_negative = true;
		v = -v;
	}

	u = (unsigned int)v;

	while (u && n) {
		*--s = "0123456789abcdef"[u % base];
		u /= base;
		n--;
	}

	if (is_negative) *--s = '-';

	return s;
}

int atoi(const char *s)
{
	int v;
	int base = 10;
	int sign = 1;

	if (!s) return 0;

	if (s[0] == '0') {
		if (s[1] == 'x' || s[1] == 'X')
			base = 16, s += 2;
	} else if (s[0] == '-')
		sign = -1, s++;

	for (v = 0; *s; s++) {
		v *= base;
		v += c2i(*s);
	}

	return v * sign;
}

int strncmp(const char *s1, const char *s2, size_t n)
{
	if (!s1 || !s2) return -1;

	for (; n > 0; s1++, s2++, n--) {
		if (*s1 != *s2)
			return *(const unsigned char *)s1 -
				*(const unsigned char *)s2;
		else if (!*s1)
			return 0;
	}

	return 0;
}

int strcmp(const char *s1, const char *s2)
{
	if (!s1 || !s2) return -1;

	for (; *s1 == *s2; s1++, s2++)
		if (!*s1) return 0;

	return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}

char *strncpy(char *d, const char *s, size_t n)
{
	char *p;

	if (!d || !s) return NULL;

	for (p = d; *s && n; n--) *p++ = *s++;
	for (; n--; *p++ = '\0');

	return d;
}

char *strcpy(char *d, const char *s)
{
	char *p;

	if (!d || !s) return NULL;

	for (p = d; *s;) *p++ = *s++;

	return d;
}

size_t strnlen(const char *s, size_t n)
{
	register const char *p;

	if (!s) return 0;

	for (p = s; *p && n; p++, n--);

	return p - s;
}

size_t strlen(const char *s)
{
	register const char *p;

	if (!s) return 0;

	for (p = s; *p; p++);

	return p - s;
}

char *strtok(char *line, const char *const token)
{
	static char *saveline = NULL;
	char *p;
	register unsigned int i, j;

	if (line) saveline = line;

	if (!saveline || !*saveline || !token) return NULL;

	for (i = 0; saveline[i]; i++) {
		for (j = 0; token[j] && (saveline[i] != token[j]); j++) ;
		if (token[j]) break;
	}

	p = saveline;
	saveline += i;

	if (*saveline)
		*saveline++ = '\0';

	return p;
}

unsigned int toknum(const char *line, const char *const token)
{
	register unsigned int i, cnt = 0;

	if (!line || !token) return 0;

	while (*line) {
		for (i = 0; token[i] && (*line != token[i]); i++) ;
		if (token[i]) cnt++;
		line++;
	}

	return cnt;
}

char *strchr(char *s, const char c)
{
	for (; s && *s; s++)
		if (*s == c) return s;

	return NULL;
}

char *strstr(const char *string, const char *word)
{
	const char *s, *s1, *s2;

	if (!word || !*word || !string) return NULL;

	s = string;

	while (*s) {
		s1 = s;
		s2 = word;

		while (*s1 && *s2 && !(*s1 - *s2))
			s1++, s2++;

		if (!*s2) return (char *)s;

		s++;
	}

	return NULL;
}

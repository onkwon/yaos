#include <string.h>

char *itoa(int v, char *buf, unsigned int base, size_t maxlen)
{
	char *s;
	int neg = 0;

	s = &buf[maxlen-1];
	*s = '\0';

	if ((v < 0) && (base == 10)) {
		neg = 1;
		v = -v;
	}

	while (v && maxlen) {
		*--s = "0123456789abcdef"[v % base];
		v /= base;
		maxlen--;
	}

	if (neg)
		*--s = '-';

	return s;
/*
	char *s = buf;

	if ((v < 0) && (base == 10)) {
		*s++ = '-';
		v *= -1;
	}

	while (v) {
		*s++ = "0123456789abcdef"[v % base];
		v /= base;
	}
	*s = '\0';

	unsigned int len, i = 0;
	char t;

	if (buf[0] == '-')
		i = 1;

	len = s - buf + i;

	for (s = buf; i < (len/2); i++) {
		t = s[i];
		s[i] = s[len-i-1];
		s[len-i-1] = t;
	}

	return buf;
	*/
}

int atoi(const char *s)
{
	int v;
	int base = 10;

	if (s[0] == '0') {
		if (s[1] == 'x' || s[1] == 'X')
			base = 16, s += 2;
	}

	for (v = 0; *s; s++) {
		v *= base;
		v += c2i(*s);
	}

	return v;
}

int strncmp(const char *s1, const char *s2, size_t n)
{
	for (; n > 0; s1++, s2++, n--)
		if (*s1 != *s2) return *s1 - *s2;

	return 0;
}

int strcmp(const char *s1, const char *s2)
{
	for (; *s1 == *s2; s1++, s2++)
		if (!*s1) return 0;

	return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}

char *strncpy(char *d, const char *s, size_t n)
{
	char *p;

	for (p = d; n && *s; n--) *p++ = *s++;
	for (; n--; *p++ = '\0');

	return d;
}

size_t strlen(const char *s)
{
	register const char *p;

	for (p = s; *p; p++);

	return p - s;
}

char *strtok(char *line, const char *const token)
{
	static char *saveline = NULL;
	char *p;
	register unsigned int i, j;

	if (line) saveline = line;

	if (!saveline || !(*saveline)) return NULL;

	for (i = 0; saveline[i]; i++) {
		for (j = 0; token[j] && (saveline[i] != token[j]); j++) ;
		if (token[j]) break;
	}

	p = saveline;
	saveline += i;

	if (*saveline) *saveline++ = '\0';

	return p;
}

unsigned int toknum(const char *line, const char *const token)
{
	register unsigned int i, cnt = 0;

	while (*line) {
		for (i = 0; token[i] && (*line != token[i]); i++) ;
		if (token[i]) cnt++;
		line++;
	}

	return cnt;
}

char *strchr(char *s, const char c)
{
	for (; *s; s++)
		if (*s == c) return s;

	return NULL;
}

#include <string.h>

int atoi(const char *s)
{
	int v;
	int nr_sys = 10;

	if (s[0] == '0') {
		if (s[1] == 'x' || s[1] == 'X')
			nr_sys = 16, s += 2;
	}

	for (v = 0; *s; s++) {
		v *= nr_sys;
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

char *strncpy(char *s1, const char *s2, size_t n)
{
	char *s;

	for (s = s1; n && *s2; n--) *s++ = *s2++;
	for (; n--; *s++ = '\0');

	return s1;
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

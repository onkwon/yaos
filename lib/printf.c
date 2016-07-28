#include <io.h>
#include <string.h>

#define BUFSIZE			(WORD_SIZE * 8 + 1) /* max length of binary */

#define FORWARD(addr)		((int *)(addr)++)
#define BACKWARD(addr)		((int *)(addr)--)
#define getarg(args, type)	(*(type *)FORWARD(args))

#define PAD_RIGHT		1
#define PAD_ZERO		2

static void printc(int fd, char **s, int c)
{
	if (s)
		*(*s)++ = c;
	else if (fd)
		fputc(fd, c);
	else
		putchar(c);
}

static size_t prints(int fd, char **to, const char *s, size_t width, int pad,
		size_t maxlen)
{
	int len;
	char padchar = ' ';

	if (pad & PAD_ZERO)
		padchar = '0';

	for (len = 0; s[len]; len++) ;

	if (pad & PAD_RIGHT)
		for (; *s && maxlen; s++, maxlen--) printc(fd, to, *s);

	if (width > len) {
		for (len = width - len; len && maxlen; len--, maxlen--)
			printc(fd, to, padchar);
	} else
		width = len;

	if (!(pad & PAD_RIGHT))
		for (; *s && maxlen; s++, maxlen--) printc(fd, to, *s);

	return maxlen;
}

static size_t printi(int fd, char **to, int v, unsigned int base, size_t width,
		int pad, size_t maxlen)
{
	char buf[BUFSIZE], *s;

	s = itoa(v, buf, base, BUFSIZE);

	if (*s == '-') {
		if ((pad & PAD_ZERO) && !(pad & PAD_RIGHT)) {
			printc(fd, to, '-');
			if (width > 0) width--;
			maxlen--;
			s++;
		}
	}

	if (strnlen(s, BUFSIZE) == 0) {
		*s = '0';
		s[1] = '\0';
	}

	return prints(fd, to, s, width, pad, maxlen);
}

#ifdef FLOAT_POINT_TEST
/* n = s * 1.m * 2^(e-127)
 * ex) -6.25(-110.01)
 *     = -1 * 1.1001 * 2^2
 *     s = 1,
 *     e = 2 + 127 = 10000001,
 *     m = [1.]1001
 */
#define EXPONENT_SIZE	8
#define MANTISSA_SIZE	23
#define BIAS		127
#define RESOLUTION	1000000
#define GETEXP(r)	(r.i.e - BIAS)
static size_t printr(int fd, char **to, double v, unsigned int base,
		size_t width, int pad, size_t maxlen)
{
	unsigned int integer, fraction, i;
	int exp;
	union {
		float f;
		struct {
			unsigned int m: MANTISSA_SIZE;
			unsigned int e: EXPONENT_SIZE;
			unsigned int s: 1;
		} i;
	} r;

	r.f = v;
	integer  = 0;
	fraction = 0;

	if ((exp = GETEXP(r)) >= 0) {
		integer = (1 << exp) | (r.i.m >> (MANTISSA_SIZE - exp));
	} else {
		r.i.m = ((1 << MANTISSA_SIZE) | r.i.m) >> -exp;
		exp = 0;
	}

	for (i = 0; i < (MANTISSA_SIZE - exp); i++) {
		if (r.i.m & (1 << i))
			fraction +=(RESOLUTION / (1 << ((MANTISSA_SIZE - exp) - i)));
	}

	char buf[BUFSIZE], *s;

	s = buf;
	if (v < 0)
		*s++ = '-';

	itoa(integer, s, 10, BUFSIZE-1);
	i = strnlen(s, BUFSIZE);
	s[i] = '.';
	itoa(fraction, &s[i+1], 10, BUFSIZE-i-1);
	if (strnlen(buf, BUFSIZE) == 1)
		buf[0] = '0';

	return prints(fd, to, buf, width, pad, maxlen);
}
#endif

static size_t print(int fd, char **to, size_t limit, int *args)
{
	const char *format;
	size_t width, len, maxlen;
	int pad;

	format = getarg(args, char *);
	maxlen = limit;
	len = 0;

	while (*format && limit) {
		if (*format == '%') {
			format++;
			pad = width = 0;

			if (*format == '-') {
				format++;
				pad = PAD_RIGHT;
			}
			if (*format == '0') {
				format++;
				pad |= PAD_ZERO;
			}
			while (*format >= '0' && *format <= '9') {
				width *= 10;
				width += *format - '0';
				format++;
			}

			switch (*format) {
			case 'd': len = limit - printi(fd, to, getarg(args, int),
						  10, width, pad, limit);
				  break;
			case 'x': len = limit - printi(fd, to, getarg(args, int),
						  16, width, pad, limit);
				  break;
			case 'b': len = limit - printi(fd, to, getarg(args, int),
						  2, width, pad, limit);
				  break;
#ifdef FLOAT_POINT_TEST
			case 'f': FORWARD(args);
				  args = (int *)ALIGN_DWORD(args);
				  len = limit - printr(fd, to, getarg(args, double),
						  10, width, pad, limit);
				  FORWARD(args);
				  break;
#endif
			case 's': len = limit - prints(fd, to, getarg(args, char *)
						  , width, pad, limit);
				  break;
			case 'c': printc(fd, to, getarg(args, char));
				  len = 1;
				  break;
			case '%': printc(fd, to, *format);
				  len = 1;
				  break;
			default : format--;
				  break;
			}

			format++;
		} else {
			printc(fd, to, *format++);
			len = 1;
		}

		limit -= len;
	}

	if (to)
		printc(fd, to, '\0');

	return maxlen - limit;
}

size_t printf(const char *format, ...)
{
	return print(0, 0, -1, (int *)&format);
}

size_t sprintf(char *to, const char *format, ...)
{
	return print(0, &to, -1, (int *)&format);
}

size_t snprintf(char *to, size_t maxlen, const char *format, ...)
{
	return print(0, &to, maxlen, (int *)&format);
}

size_t fprintf(int fd, const char *format, ...)
{
	return print(fd, 0, -1, (int *)&format);
}

size_t printk(const char *format, ...)
{
	if (!stdout) return 0;

	extern void __putc_debug(int c);
	size_t ret;
	void (*tmp)(int) = putchar;

	putchar = __putc_debug;
	ret = print(0, 0, -1, (int *)&format);
	putchar = tmp;

	return ret;
}

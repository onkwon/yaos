#include <foundation.h>
#include <string.h>

#define BUF_SIZE		(WORD_SIZE * 8 + 1) /* max length of binary */

#define FORWARD(addr)		((int *)(addr)++)
#define BACKWARD(addr)		((int *)(addr)--)
#define getarg(args, type)	(*(type *)FORWARD(args))

#define PAD_RIGHT		1
#define PAD_ZERO		2

void __putchar(int c)
{
	write(stdout, &c, 1);

	if (c == '\n')
		__putchar('\r');
}

void (*putchar)(int c) = __putchar;

void puts(const char *s)
{
	while (*s) putchar(*s++);
}

int getc()
{
	int c;

	if (!read(stdin, &c, 1))
		c = -1;

	return c;
}

static void printc(char **s, int c)
{
	if (s)
		*(*s)++ = c;
	else
		putchar(c);
}

static size_t prints(char **out, const char *s, size_t width, int pad,
		size_t maxlen)
{
	int len;
	char padchar = ' ';

	if (pad & PAD_ZERO)
		padchar = '0';

	for (len = 0; s[len]; len++) ;

	if (pad & PAD_RIGHT)
		for (; *s && maxlen; s++, maxlen--) printc(out, *s);

	if (width > len) {
		for (len = width - len; len && maxlen; len--, maxlen--)
			printc(out, padchar);
	} else
		width = len;

	if (!(pad & PAD_RIGHT))
		for (; *s && maxlen; s++, maxlen--) printc(out, *s);

	return maxlen;
}

static size_t printi(char **out, int v, unsigned int base, size_t width,
		int pad, size_t maxlen)
{
	char buf[BUF_SIZE], *s;

	s = itoa(v, buf, base, BUF_SIZE);

	if (*s == '-') {
		if ((pad & PAD_ZERO) && !(pad & PAD_RIGHT)) {
			printc(out, '-');
			if (width > 0) width--;
			maxlen--;
			s++;
		}
	}

	if (strlen(s) == 0) {
		*s = '0';
		s[1] = '\0';
	}

	return prints(out, s, width, pad, maxlen);
}

/* n = s * 1.m * 2^(e-127)
 * ex) -6.25(-110.01)
 *     = -1 * 1.1001 * 2^2
 *     s = 1,
 *     e = 2 + 127 = 10000001,
 *     m = [1.]1001
 */
/*
#define EXPONENT_SIZE	8
#define MANTISSA_SIZE	23
#define BIAS		127
#define RESOLUTION	1000000
#define GETEXP(r)	(r.i.e - BIAS)
static size_t printr(char **out, double v, unsigned int base, size_t width,
		int pad, size_t maxlen)
{
	unsigned int integer, decimal, i;
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
	integer = 0;
	decimal = 0;

	if ((exp = GETEXP(r)) >= 0) {
		integer = (1 << exp) | (r.i.m >> (MANTISSA_SIZE - exp));
	} else {
		r.i.m = ((1 << MANTISSA_SIZE) | r.i.m) >> -exp;
		exp = 0;
	}

	for (i = 0; i < (MANTISSA_SIZE - exp); i++) {
		if (r.i.m & (1 << i))
			decimal +=(RESOLUTION / (1 << ((MANTISSA_SIZE - exp) - i)));
	}

	char buf[BUF_SIZE], *s;

	s = buf;
	if (v < 0)
		*s++ = '-';

	itoa(integer, s, 10);
	i = strlen(s);
	s[i] = '.';
	itoa(decimal, &s[i+1], 10);
	if (strlen(buf) == 1)
		buf[0] = '0';

	return prints(out, buf, width, pad, maxlen);
}
*/

static size_t print(char **out, size_t limit, int *args)
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
			case 'd': len = limit - printi(out, getarg(args, int),
						  10, width, pad, limit);
				  break;
			case 'x': len = limit - printi(out, getarg(args, int),
						  16, width, pad, limit);
				  break;
			case 'b': len = limit - printi(out, getarg(args, int),
						  2, width, pad, limit);
				  break;
			/*
			case 'f': FORWARD(args);
				  args = (int *)ALIGN_DWORD(args);
				  len = limit - printr(out, getarg(args, double),
						  10, width, pad, limit);
				  FORWARD(args);
				  break;
			*/
			case 's': len = limit - prints(out, getarg(args, char *)
						  , width, pad, limit);
				  break;
			case 'c': printc(out, getarg(args, char));
				  len = 1;
				  break;
			default : format--;
				  break;
			}

			format++;
		} else {
			printc(out, *format++);
			len = 1;
		}

		limit -= len;
	}

	if (out)
		printc(out, '\0');

	return maxlen - limit;
}

size_t printf(const char *format, ...)
{
	putchar = __putchar;
	return print(0, -1, (int *)&format);
}

size_t sprintf(char *out, const char *format, ...)
{
	return print(&out, -1, (int *)&format);
}

size_t snprintf(char *out, size_t maxlen, const char *format, ...)
{
	return print(&out, maxlen, (int *)&format);
}

#include <driver/console.h>

size_t printk(const char *format, ...)
{
#ifdef CONFIG_SYSCALL
	putchar = console_putc;
#endif
	return print(0, -1, (int *)&format);
}

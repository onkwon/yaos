#include <io.h>
#include <string.h>

#define BUFSIZE				\
	(WORD_SIZE * 8 + 1) /* max length of binary */

#define FORWARD(addr)			((int *)(addr)++)
#define BACKWARD(addr)			((int *)(addr)--)
#define getarg(args, type)		(*(type *)FORWARD(args))

#define PAD_RIGHT			(unsigned int)(1 << (WORD_SIZE * 8 - 1))
#define PAD_ZERO			(unsigned int)(1 << (WORD_SIZE * 8 - 2))

#define set_padding_dir(v, dir)		(v |= dir)
#define get_padding_dir(v)		((v) & PAD_RIGHT)
#define set_padding_zero(v)		(v |= PAD_ZERO)
#define is_padding_zero(v)		((v) & PAD_ZERO)
#define get_padding_width(v)		((v) & ~(PAD_RIGHT | PAD_ZERO))

#define tok2base(x)			\
	((x == 'd')? 10 : (x == 'x')? 16 : (x == 'b')? 2 : 10)

static void printc(int fd, void **s, int c)
{
	if (s)
		*(*(char **)s)++ = c;
	else if (fd)
		fputc(fd, c);
	else
		putchar(c);
}

static size_t prints(int fd, void **to, const char *s, size_t padding,
		size_t maxlen)
{
	size_t len, i;
	bool is_right;
	char padchar = ' ';

	len = strnlen(s, maxlen);
	is_right = get_padding_dir(padding)? true : false;

	if (is_padding_zero(padding))
		padchar = '0';

	padding = get_padding_width(padding);
	padding = padding - min(len, get_padding_width(padding));
	len = min(len + padding, maxlen);

	for (i = 0; i < len; i++) {
		if (!*s || (is_right && i < padding)) {
			printc(fd, to, padchar);
			continue;
		}

		printc(fd, to, *s++);
	}

	return i;
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

static size_t print(int fd, void **to, size_t limit, int *args)
{
	const char *format;
	size_t padding, width, printed, maxlen;
	bool op;
	char buf[BUFSIZE];

	format = getarg(args, char *);
	maxlen = limit;
	printed = 0;

	for (op = false; *format && limit; format++) {
		if (!op) {
			if (*format != '%') {
				printc(fd, to, *format);
				limit--;
				continue;
			}

			op = true;
			padding = width = 0;
			format++;
		}

		switch (*format) {
		case '-':
			set_padding_dir(padding, PAD_RIGHT);
			continue;
		case '0' ... '9':
			width *= 10;
			width += *format - '0';
			if (!width) /* if the leading zero */
				set_padding_zero(padding);
			continue;
		case 'd':
		case 'x':
		case 'b':
			itos(buf, getarg(args, int), tok2base(*format),
					BUFSIZE);
			printed = prints(fd, to, buf, padding | width, limit);
			break;
		case 'f':
			break;
		case 's':
			printed = prints(fd, to, getarg(args, char *),
					padding | width, limit);
			break;
		case 'c':
			printc(fd, to, getarg(args, char));
			printed = 1;
			break;
		case '%':
			printc(fd, to, *format);
			printed = 1;
			break;
		default:
			format--;
			break;
		}

		limit -= printed;
		op = false;
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
	return print(0, (void **)&to, -1, (int *)&format);
}

size_t snprintf(char *to, size_t maxlen, const char *format, ...)
{
	return print(0, (void **)&to, maxlen, (int *)&format);
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

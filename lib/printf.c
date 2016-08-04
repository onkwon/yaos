#include <io.h>
#include <string.h>

#define BUFSIZE				\
	(WORD_SIZE * 8 + 1) /* max length of binary */

#define getarg(args, type)		({ \
	args = (char *)ALIGN_BLOCK(args, sizeof(type)); \
	args = (char *)((unsigned int)(args) + sizeof(type)); \
	*(type *)((unsigned int)(args) - sizeof(type)); \
})

#define PAD_RIGHT			(1 << (WORD_SIZE * 8 - 1))
#define PAD_ZERO			(1 << (WORD_SIZE * 8 - 2))

#define set_padding_dir(v, dir)		(v |= dir)
#define get_padding_dir(v)		((v) & PAD_RIGHT)
#define set_padding_zero(v)		(v |= PAD_ZERO)
#define is_padding_zero(v)		((v) & PAD_ZERO)
#define get_padding_width(v)		((v) & ~(PAD_RIGHT | PAD_ZERO))

#define tok2base(x)			\
	((x == 'd')? 10 : (x == 'x')? 16 : (x == 'b')? 2 : (x == 'p')? 16 : 10)

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
		if (!*s || (!is_right && i < padding)) {
			printc(fd, to, padchar);
			continue;
		}

		printc(fd, to, *s++);
	}

	return i;
}

static size_t print(int fd, void **to, size_t limit, void *args)
{
	const char *fmt;
	size_t padding, width, printed, maxlen;
	bool op;
	char buf[BUFSIZE];

	fmt = getarg(args, char *);
	maxlen = limit;
	printed = 0;

	for (op = false; *fmt && limit; fmt++) {
		if (!op) {
			if (*fmt != '%') {
				printc(fd, to, *fmt);
				limit--;
				continue;
			}

			op = true;
			padding = width = 0;
			fmt++;
		}

		switch (*fmt) {
		case '-':
			set_padding_dir(padding, PAD_RIGHT);
			continue;
		case '0' ... '9':
			width *= 10;
			width += *fmt - '0';
			if (!width) /* if the leading zero */
				set_padding_zero(padding);
			continue;
		case 'd':
		case 'x':
		case 'p': /* TODO: add "0x" prefix */
		case 'b':
			itos(getarg(args, int), buf, tok2base(*fmt), BUFSIZE);
			printed = prints(fd, to, buf, padding | width, limit);
			break;
#ifdef CONFIG_FLOAT
		case 'f':
			ftos(getarg(args, double), buf, BUFSIZE);
			printed = prints(fd, to, buf, padding | width, limit);
			break;
#endif
		case 's':
			printed = prints(fd, to, getarg(args, char *),
					padding | width, limit);
			break;
		case 'c':
			printc(fd, to, getarg(args, char));
			printed = 1;
			break;
		case '%':
			printc(fd, to, *fmt);
			printed = 1;
			break;
		default:
			fmt--;
			break;
		}

		limit -= printed;
		op = false;
	}

	if (to)
		printc(fd, to, '\0');

	return maxlen - limit;
}

size_t printf(const char *fmt, ...)
{
	return print(0, 0, -1, &fmt);
}

size_t sprintf(char *to, const char *fmt, ...)
{
	return print(0, (void **)&to, -1, &fmt);
}

size_t snprintf(char *to, size_t maxlen, const char *fmt, ...)
{
	return print(0, (void **)&to, maxlen, &fmt);
}

size_t fprintf(int fd, const char *fmt, ...)
{
	return print(fd, 0, -1, &fmt);
}

size_t printk(const char *fmt, ...)
{
	if (!stdout) return 0;

	extern void __putc_debug(int c);
	size_t ret;
	void (*tmp)(int) = putchar;

	putchar = __putc_debug;
	ret = print(0, 0, -1, &fmt);
	putchar = tmp;

	return ret;
}

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

#define NR_BITS_WLEN			7
#define WLAN_MASK			((1 << NR_BITS_WLEN) - 1)
#define TLAN_MASK			\
	(((1 << NR_BITS_WLEN) - 1) << NR_BITS_WLEN)

#define set_padding_dir(x, dir)		(x |= dir)
#define get_padding_dir(x)		((x) & PAD_RIGHT)
#define fill_padding_zero(x)		(x |= PAD_ZERO)
#define get_padding_type(x)		((x) & PAD_ZERO)
#define get_padding_wlen(x)		((x) & WLAN_MASK)
#define get_padding_tlen(x)		(((x) & TLAN_MASK) >> NR_BITS_WLEN)
#define combine(tlen, wlen)		(((tlen) << NR_BITS_WLEN) | wlen)

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

static size_t prints(int fd, void **to, const char *s, int opt, size_t maxlen)
{
	int padding, len, i;
	char padchar;
	bool is_right = false;

	len = strnlen(s, maxlen);
	padding = get_padding_wlen(opt);
	i = 0;

	if (padding) {
		is_right = get_padding_dir(opt)? true : false;
		padchar = get_padding_type(opt)? '0' : ' ';
		padding = ((padding - len) < 0)? 0 : padding - len;
		len = min(get_padding_wlen(opt), maxlen);

		if (*s == '-' && padchar == '0') {
			printc(fd, to, *s++);
			padding++;
			i++;
		}
	}

	for (; i < len; i++) {
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
	size_t printed, maxlen, wlen, tlen;
	int padding;
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
			padding = wlen = tlen = 0;
			fmt++;
		}

		switch (*fmt) {
		case '0' ... '9':
			wlen *= 10;
			wlen += *fmt - '0';
			if (!wlen) /* if the leading zero */
				fill_padding_zero(padding);
			continue;
		case '#': /* TODO: add "0x" prefix */
			break;
		case 'd':
		case 'x':
		case 'p':
		case 'b':
			itos(getarg(args, int), buf, tok2base(*fmt), BUFSIZE);
			printed = prints(fd, to, buf, padding | wlen, limit);
			break;
#ifdef CONFIG_FLOAT
		case 'f':
			ftos(getarg(args, double), buf, wlen, BUFSIZE);
			printed = prints(fd, to, buf,
					padding | ((tlen)? tlen : wlen),
					limit);
			break;
#endif
		case 's':
			printed = prints(fd, to, getarg(args, char *),
					padding | wlen, limit);
			break;
		case 'c':
			printc(fd, to, getarg(args, char));
			printed = 1;
			break;
		case '%':
			printc(fd, to, *fmt);
			printed = 1;
			break;
		case '.':
			if (fmt[-1] == '-' || fmt[-1] == '%' || wlen) {
				tlen = wlen;
				wlen = 0;
				continue;
			}
			fmt--;
			break;
		case '-':
			if (fmt[-1] == '%') {
				set_padding_dir(padding, PAD_RIGHT);
				continue;
			}
			/* fall through */
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

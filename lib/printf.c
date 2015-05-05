/* IMPORTANT: Make sure getarg() works properly on the target system.
 * Each compilers may have its own way to place parameters when calling
 * a function, depending on the architecture.
 * Don't suppose it works the way you expect before checking yourself.
 *
 * - Support floating point output
 *   how FPU and its registers are used?
 *   (don't understand the way of passing float-point variable
 *   to a function looking at disassembled codes.)
 */

#define PAD_RIGHT		1
#define PAD_ZERO		2

/* architecture specific: parameters are INTSIZE-aligned for optimization */
#define INTSIZE			4
#define PRINT_BUFSIZE		(INTSIZE * 8 + 1)
#define getarg(args, type)	(*(type *)args++)
#define align_dword(args)	(args += ((int)args & ((INTSIZE<<1)-1))? 1 : 0)

#include <io.h>

static void printc(char **s, int c)
{
	if (s) {
		*(*s)++ = c;
	} else {
		write(stdout, &c, 1);

		if (c == '\n') {
			c = '\r';
			write(stdout, &c, 1);
		}
	}
}

static int prints(char **out, const char *s, int width, int pad)
{
	int len;
	char padchar = ' ';

	if (pad & PAD_ZERO) padchar = '0';

	for (len = 0; s[len]; len++);

	if (pad & PAD_RIGHT)
		for (; *s; s++) printc(out, *s);

	if (width > len)
		for (len = width - len; len; len--) printc(out, padchar);
	else
		width = len;

	if (!(pad & PAD_RIGHT))
		for (; *s; s++) printc(out, *s);

	return width;
}

static int printi(char **out, int v, int base, int width, int pad)
{
	char buf[PRINT_BUFSIZE], *s;
	int t, neg = 0;
	unsigned u;

	s  = &buf[PRINT_BUFSIZE-1];
	*s = '\0';

	if ((v < 0) && (base == 10)) {
		neg = 1;
		v = -v;
	}

	for (u = v; u; u /= base) {
		t    = u % base;
		*--s = "0123456789abcdef"[t];
	}

	if (neg) {
		if ((pad & PAD_ZERO) && !(pad & PAD_RIGHT)) {
			printc(out, '-');
			if (width > 0) width--;
		} else {
			*--s = '-';
			neg  = 0;
		}
	}

	return prints(out, s, width, pad) + neg;
}

static int print(char **out, int *args)
{
	const char *format = getarg(args, char *);
	int pad, width;
	int len = 0;

	while (*format) {
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
			if (*format == 'l') {
				format++;
				align_dword(args);
			}

			switch (*format) {
			case 'd': len += printi(out, getarg(args, int), 10, width, pad);
				  if (*(format-1) == 'l') args++;
				  break;
			case 'x': len += printi(out, getarg(args, int), 16, width, pad);
				  break;
			case 'b': len += printi(out, getarg(args, int),  2, width, pad);
				  break;
			case 's': len += prints(out, getarg(args, char *),  width, pad);
				  break;
			case 'c': /*len += prints(out, (const char *)args++, width, pad);*/
				  printc(out, getarg(args, char));
				  len++;
				  break;
			default : format--;
				  break;
			}

			format++;
		} else
			printc(out, *format++);
	}

	return len;
}

int printf(const char *format, ...)
{
	return print(0, (int *)&format);
}

int sprintf(char *out, const char *format, ...)
{
	return print(&out, (int *)&format);
}

int printk(const char *format, ...)
{
	return print(0, (int *)&format);
}

#ifdef PRINTF_TEST
int main()
{
	int a, c;
	char b = 'C';
	int d;
	long long r = 123456789;

	a = -12345;
	c = -56789;
	d = 12345;

	ptf("Hello, World! %d, %-8c, %08d, %x\n", a, b, c, d);
	ptf("%s - %010s\n", "12345678", "abcde");
	ptf("%% asf %d\n", a);
	ptf("%d %8d %-8d %08d %-08d\n", c, c, c, c, c);
	ptf("%b\n", c);
	ptf("%d %ld %d\n", a, r, a);
	ptf("%ld %d\n", r, a);
	ptf("%d %ld %d %ld %ld\n", a, r, a, r, r);
	ptf("%ld %d %ld %d\n", r, a, r, a);
	ptf("%c %d %c %c %d\n", b, a, b, b, a);

	char buf[50];

	spf(buf, "SPRINTF TEST\n %d %ld %-8d %c", a, r, d, b);
	printf("%s\n", buf);

	float f = 9876;
	ptf("size: %d, %d %f %d\n", sizeof(float), d, f, d);
	ptf("size: %d, %d%d\n", sizeof(float), d, d);

	return 0;
}
#endif

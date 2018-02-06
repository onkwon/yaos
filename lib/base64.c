#include <lib/base64.h>

struct base64_t {
	union {
		struct {
			unsigned char c3, c2, c1;
			unsigned char _pad;
		};

		struct {
			unsigned int e4: 6;
			unsigned int e3: 6;
			unsigned int e2: 6;
			unsigned int e1: 6;
			unsigned int _pad2: 8;
		};
	};
} __attribute__((packed, aligned(4)));

static const unsigned char enctbl[] = {
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
	'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
	'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
	'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
	'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
	'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
	'w', 'x', 'y', 'z', '0', '1', '2', '3',
	'4', '5', '6', '7', '8', '9', '+', '/',
};

static const unsigned char dectbl[] = {
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF,   62, 0xFF, 0xFF, 0xFF,   63,
	  52,   53,   54,   55,   56,   57,   58,   59,
	  60,   61, 0xFF, 0xFF, 0xFF,   64, 0xFF, 0xFF,
	0xFF,    0,    1,    2,    3,    4,    5,    6,
	   7,    8,    9,   10,   11,   12,   13,   14,
	  15,   16,   17,   18,   19,   20,   21,   22,
	  23,   24,   25, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF,   26,   27,   28,   29,   30,   31,   32,
	  33,   34,   35,   36,   37,   38,   39,   40,
	  41,   42,   43,   44,   45,   46,   47,   48,
	  49,   50,   51, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
};
 
size_t base64_encode(unsigned char *dst, const unsigned char *src, size_t slen)
{
	struct base64_t t;
	unsigned char *p;
	int n, i;

	if (!slen || dst == NULL || src == NULL)
		return 0;

	p = dst;
	n = (slen / 3) * 3;

	for (i = 0; i < n; i += 3) {
		t.c1 = *src++;
		t.c2 = *src++;
		t.c3 = *src++;
		*p++ = enctbl[t.e1];
		*p++ = enctbl[t.e2];
		*p++ = enctbl[t.e3];
		*p++ = enctbl[t.e4];
	}

	if (i < slen) {
		t.c1 = *src++;
		t.c2 = (slen - (i + 1)) * *src++;
		t.c3 = 0;
		*p++ = enctbl[t.e1];
		*p++ = enctbl[t.e2];
		*p++ = t.e3? enctbl[t.e3] : '=';
		*p++ = '=';
	}

	*p = '\0';

	return (size_t)((unsigned int)p - (unsigned int)dst);
}

size_t base64_decode(unsigned char *dst, const unsigned char *src, size_t slen)
{
	struct base64_t t;
	unsigned char *p;
	int n, r, i;

	if (!slen || dst == NULL || src == NULL)
		return 0;

	p = dst;
	for (n = slen; src[n - 1] == '='; n--);
	r = slen - n;
	n = (n / 4) * 4;

	for (i = 0; i < n; i += 4) {
		t.e1 = dectbl[*src++];
		t.e2 = dectbl[*src++];
		t.e3 = dectbl[*src++];
		t.e4 = dectbl[*src++];
		*p++ = t.c1;
		*p++ = t.c2;
		*p++ = t.c3;
	}

	if (i < slen) {
		t.e1 = dectbl[*src++];
		t.e2 = dectbl[*src++];
		t.e3 = ((i + 1) < slen)? dectbl[*src++] : 0;
		t.e4 = 0;
		*p++ = t.c1;
		*p++ = t.c2;
		*p++ = t.c3;
	}

	*p = '\0';

	return (size_t)((unsigned int)p - (unsigned int)dst - r);
}

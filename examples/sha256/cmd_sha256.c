#include "sha256.h"
#include "../../tasks/shell/shell.h"
#include <foundation.h>
#include <string.h>
#include <stdlib.h>

static int sha256(int argc, char **argv)
{
	SHA256_CTX ctx;
	unsigned int addr, size;
	unsigned char result[SHA256_BLOCK_SIZE];
	int q, r, i;

	if (argc != 3)
		return -EINVAL;

	addr = (unsigned int)atoi(argv[1]);
	size = (unsigned int)atoi(argv[2]);

	q = size / SHA256_BLOCK_SIZE;
	r = size % SHA256_BLOCK_SIZE;

	//debug("addr %x size %d, q %d r %d", addr, size, q, r);

	sha256_init(&ctx);
	for (i = 0; i < q; i++)
		sha256_update(&ctx,
				(unsigned char *)(addr + (i * SHA256_BLOCK_SIZE)),
				SHA256_BLOCK_SIZE);
	if (r)
		sha256_update(&ctx,
				(unsigned char *)(addr + (i * SHA256_BLOCK_SIZE)),
				r);
	sha256_final(&ctx, result);

	printf("sha256sum: ");
	for (i = 0; i < SHA256_BLOCK_SIZE; i++)
		printf("%02x", result[i]);
	putchar('\n');

	return 0;
}
REGISTER_CMD(sha256, sha256, "sha256 addr size");

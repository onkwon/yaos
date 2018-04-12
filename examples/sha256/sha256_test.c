#include "sha256.h"
#include <foundation.h>

static void sha256()
{
	SHA256_CTX ctx;
	unsigned char result[SHA256_BLOCK_SIZE];
	unsigned int start, end, q, r;
	int i;
	extern int _etext, _edata, _rom_start, _ram_start;

	start = (unsigned int)&_rom_start;
	end = (unsigned int)&_etext;
	end += (unsigned int)&_edata & ~((unsigned int)&_ram_start);
	q = (end - start) / SHA256_BLOCK_SIZE;
	r = (end - start) % SHA256_BLOCK_SIZE;
	debug("end %x q %d, r %d", end, q, r);

	sha256_init(&ctx);
	for (i = 0; i < q; i++)
		sha256_update(&ctx,
				(unsigned char *)(start + (i * SHA256_BLOCK_SIZE)),
				SHA256_BLOCK_SIZE);
	if (r)
		sha256_update(&ctx,
				(unsigned char *)(start + (i * SHA256_BLOCK_SIZE)),
				r);
	sha256_final(&ctx, result);

	printk("SHA256(FW): ");
	for (i = 0; i < SHA256_BLOCK_SIZE; i++)
		printk("%02x", result[i]);
	printk("\n");
}
REGISTER_TASK(sha256, 0, DEFAULT_PRIORITY, STACK_SIZE_DEFAULT);

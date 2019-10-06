#include "sha256.h"

#include "syslog.h"
#include "kernel/task.h"

extern int _etext, _edata, _rom_start, _ram_start;

static void sha256(void)
{
	SHA256_CTX ctx;
	unsigned char result[SHA256_BLOCK_SIZE];
	unsigned int start, end, q, r, i;

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

	dprintf(SYSLOG_FD_DEBUG, "SHA256(FW): ");
	for (i = 0; i < SHA256_BLOCK_SIZE; i++)
		dprintf(SYSLOG_FD_DEBUG, "%02x", result[i]);
	dprintf(SYSLOG_FD_DEBUG, "\r\n");
}
REGISTER_TASK(sha256, TASK_USER, 0, STACK_SIZE_MIN);

#ifndef __SHELL_H__
#define __SHELL_H__

#include "compiler.h"

#define SHELL_EXIT			0xdead

#define RC_STR_SHELL_WELCOME		"Type 'help' for help on commands.\n"
#define RC_STR_SHELL_UNKNOWN		"unknown command\n"

struct shell_cmd {
	const char * const name;
	int (*run)(int argc, char **argv);
	const char * const usage;
};

#if defined(__APPLE__) && defined(__MACH__)
#define SHELL_CMD_SECTION		\
	__attribute__((section(".__DUMMY,__dummy")))
#else
#define SHELL_CMD_SECTION		\
	__attribute__((section(".shell_cmdlist"), aligned(WORD_SIZE), used))
#endif

#define REGISTER_CMD(n, f, u)						\
	STATIC struct shell_cmd shell_cmd_##n SHELL_CMD_SECTION	= {	\
		.name  = #n,						\
		.run   = f,						\
		.usage = u						\
	}

void shell_set(int (*rxc)(void), void (*txc)(int c));

#endif /* __SHELL_H__ */

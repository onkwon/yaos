#ifndef __SHELL_H__
#define __SHELL_H__

#define SHELL_EXIT		0xdeaf

struct shell_cmd {
	char *name;
	int (*run)(int argc, char **argv);
	char *usage;
};

#define REGISTER_CMD(n, f, u) \
		static struct shell_cmd shell_cmd_##n \
		__attribute__((section(".shell_cmdlist"), \
					aligned(4), used)) = { \
			.name  = #n, \
			.run   = f, \
			.usage = u }

void shell();

#endif /* __SHELL_H__ */

#include <foundation.h>
#include <shell.h>

static int help(int argc, char **argv)
{
	extern char _shell_cmdlist;
	struct shell_cmd *cmd = (struct shell_cmd *)&_shell_cmdlist;

	while (cmd->name) {
		printf("%s\t\t%s\n", cmd->name, cmd->usage);
		cmd++;
	}

	return 0;
}
REGISTER_CMD(help, help, "display this information");

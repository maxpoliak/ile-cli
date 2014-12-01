/***************************************************************************\
#                                test/main.c                                #
\***************************************************************************/

#include <ile-cli-api.h>
#include <ile-cli-config.h>

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <stdbool.h>
#include <stdlib.h>
#include <getopt.h>

/*
 * my_test_exec()
 */
static int my_test_exec(node_t self, const int argc, char **const argv)
{
	if (argc) {
		for(register int i = 1; i < argc && strcmp(argv[i], "<echo>"); ++i) {
			cli_info_print(ILE_CLI_WHITE_COLOUR, "%s ", argv[i]);
		}
	}
	return 0;
}

/*
 * ile_cli_cmd_tree_build()
 */
int ile_cli_cmd_tree_build(void)
{
	unsigned int i;
	node_t root = ile_command_root_node_get();
	node_t node, echo = ile_cli_cmd_exec_node_add(root, "echo", "echo", my_test_exec);
	for (i = 0, node = echo; node && i < ILE_CLI_MAX_NUM_ARGS; ++i) {
		node = ile_cli_cmd_exec_node_flags_add(node, "<echo>", "<echo>", my_test_exec,
		                                       ILE_CMD_FLAG(UNCHECKED));
	}
	if (!node)
		return -1;
	else
		return 0;
}

/*
 * test_linux_char_get()
 */
unsigned short ile_cli_char_get(void)
{
	struct termios oldt, newt;
	unsigned short ch;
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	ch = getchar();
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	return ch;
}

/*
 * test_linux_console_output()
 * @text
 */
void ile_cli_console_output(const char* text)
{
	fprintf(stdout, "%s", text);
}

struct ile_cli_operations op = {
	.char_get = ile_cli_char_get,
	.char_output = ile_cli_console_output,
	.tree_build = ile_cli_cmd_tree_build,
};

/*
 * main()
 */
int main(int argc, char **argv)
{
	int opt, indx = 0;
	const char *command;
	static struct option options[] = {
		{ "command", required_argument,  0,  'C' },
		{ "help",    no_argument,        0,  'h' },
		{ 0,         0,                  0,   0  }
	};

	cli_vterm_init(&op);
	while ((opt = getopt_long(argc, argv, "h::C:", options, &indx)) != -1) {
	switch (opt) {
		case 'C':
			command = optarg;
			if (cli_vterm_exec_command(command) != 0)
				printf("... Error!\n");
			return 0;
		case 'h':
			printf("Help!\n");
			return 0;
		}
	}
	cli_vterm_char_proc();
	exit(0);
}

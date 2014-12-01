/***************************************************************************\
#                               ile-cli-api.h                               #
\***************************************************************************/

#ifndef ILE_CLI_API_H
#define ILE_CLI_API_H

#include <unistd.h>

#define ILE_CMD_FLAG(x)				(1 << ILE_CMD_FLAG_##x)
#define ILE_CMD_FLAG_TST(x, node)	(((node)->flags) & (1 << ILE_CMD_FLAG_##x))
#define ILE_CMD_FLAG_SET(x, node)	(((node)->flags) |= (1 << ILE_CMD_FLAG_##x))

enum ile_cmd_flag {
	ILE_CMD_FLAG_UNCHECKED = 0,
	ILE_CMD_FLAG_IS_ROOT   = 1,
};

enum colour_type {
	ILE_CLI_RESET_COLOUR   = 0,
	ILE_CLI_BLACK_COLOUR   = 30,
	ILE_CLI_RED_COLOUR     = 31,
	ILE_CLI_GREEN_COLOUR   = 32,
	ILE_CLI_YELLOW_COLOUR  = 33,
	ILE_CLI_BLUE_COLOUR    = 34,
	ILE_CLI_MAGENTA_COLOUR = 35,
	ILE_CLI_CYAN_COLOUR    = 36,
	ILE_CLI_WHITE_COLOUR   = 37,
};

struct ile_cli_operations {
	unsigned short (*char_get)(void);
	void (*char_output)(const char* );
	int (*tree_build)(void);
	void* (*node_allocator)(size_t size);
};

typedef void* node_t;

node_t ile_command_root_node_get(void);
node_t ile_command_root_node_set(node_t root);
node_t ile_cli_cmd_root_node_add(node_t parent, const char* banner_name, const char* info);
node_t ile_cli_cmd_node_add(node_t parent, const char* name, const char* info);
node_t ile_cli_cmd_node_flags_add(node_t parent, const char* name, const char* info, const unsigned char flags);
node_t ile_cli_cmd_exec_node_add(node_t parent_node, const char* name, const char* info, int (*exec)(node_t self, const int argc, char **const argv));
node_t ile_cli_cmd_exec_node_flags_add(node_t parent_node, const char* name, const char* info, int (*exec)(node_t self, const int argc, char **const argv), const unsigned char flags);

int cli_info_print(enum colour_type colour, const char *format, ...);
void cli_vterm_init(struct ile_cli_operations *op);
void cli_vterm_char_proc(void);
int cli_vterm_exec_command(const char *command);

#endif

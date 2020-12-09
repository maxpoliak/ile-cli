/***************************************************************************\
#                                ile-vterm.с                                #
\***************************************************************************/

#include <ile-cli-core.h>
#include <ile-cli-cmd-tree.h>
#include <ile-history.h>
#include <ile-debug.h>

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#ifndef ILE_CLI_BANNER_NAME_LENGHT
#define ILE_CLI_BANNER_NAME_LENGHT	16
#endif

#define CLI_MAIN_BANNER				0

static char banner_name[ILE_CLI_BANNER_NAME_LENGHT];

unsigned short (*char_get)(void);

/*
 * cli_vterm_parse_args()
 */
static void cli_vterm_parse_args(void)
{
	struct ile_cli *cli = cli_data_get();
	register unsigned int i;
	const char *separator = " ";
	if (!cli->len || !cli->buffer[0])
		return;
	cli->argv[0] = strtok(cli->buffer, separator);
	for(i = 1; i < ILE_CLI_MAX_NUM_ARGS; ++i) {
		cli->argv[i] = strtok(NULL, separator);
		if (cli->argv[i] == NULL)
			break;
	}
	cli->argc = i;
}

/*
 * cli_vterm_restore_line_args()
 */
static void cli_vterm_restore_line_args(void)
{
	struct ile_cli *cli = cli_data_get();
	register unsigned int i;
	for(i = 0; i < cli->len; ++i) {
		if (!cli->buffer[i])
			cli->buffer[i] = ' ';
	}
}

/*
 * cli_vterm_question()
 */
void cli_vterm_question(void)
{
	struct ile_cli *cli = cli_data_get();
	int is_after_whitespace = cli->cursor && cli->buffer[cli->cursor - 1] == ' ' ? 1 : 0;
	cli_vterm_parse_args();
	ile_cli_cmd_candidate_lookup(cli->argc, cli->argv, is_after_whitespace);
	cli_banner_print();
	cli_vterm_restore_line_args();
	cli->argc = 0;
	cli_buffer_print(0);
	return;
}

/*
 * cli_vterm_tab()
 */
void cli_vterm_tab(void)
{
	struct ile_cli *cli = cli_data_get();
	int is_after_whitespace;
	const char *candidate;
	unsigned int delta = 0;
	struct ile_cmd_candidates_attr candidates_attr = {
		.offset = 0, .candidate_counter = 0, .common_length = ILE_CLI_MAX_CMD_LEN,
	};
	if (!cli->cursor)
		return;
	is_after_whitespace = cli->buffer[cli->cursor - 1] == ' ' ? 1 : 0;
	cli_vterm_parse_args();
	candidate = ile_cli_cmd_candidate_completion(cli->argc, cli->argv, &candidates_attr, is_after_whitespace);
	cli_vterm_restore_line_args();
	cli->argc = 0;
	if (candidate) {
		delta = candidates_attr.common_length;
		if (!is_after_whitespace)
			delta -= candidates_attr.offset;
		cli_text_print(&candidate[candidates_attr.offset], delta);
	} else if (candidates_attr.candidate_counter >= 2) {
		cli_banner_print();
		cli_buffer_print(0);
	}
	return;
}

/*
 * cli_vterm_history_restore()
 * @dir : the direction of movement through the history
 * return
 *   length of history entry
 */
void cli_vterm_history_restore(unsigned short dir)
{
	struct ile_cli *cli = cli_data_get();
	unsigned int hist_entry_len =
		(dir ? ile_hist_command_read_prev : ile_hist_command_read_next)(cli->buffer);
	if (hist_entry_len) {
		cli_move_cursor_left(cli->cursor);
		cli_remove_from_cursore();
		cli->len = hist_entry_len;
		cli->cursor = cli->len;
		cli->buffer[cli->cursor] = 0;
		cli_buffer_print(0);
	} else if (!dir && cli->len) {
		cli_move_cursor_left(cli->cursor);
		cli_remove_from_cursore();
		cli_buffer_clear();
	}
}

/*
 * cli_vterm_reset()
 */
static void cli_vterm_reset(void)
{
	cli_buffer_clear();
	cli_reset_cursor();
}

/*
 * cli_vterm_upload_command_into_buffer()
 * @command
 * return : error status
 */
int cli_vterm_upload_command_into_buffer(const char *command)
{
	struct ile_cli *cli = cli_data_get();
	size_t command_len = strlen(command);

	if (command_len >= sizeof(cli->buffer))
		return -1;

	strcpy(cli->buffer, command);
	cli->len = (unsigned int)command_len;
	cli->cursor = cli->len;
	return 0;
}

//#define ILE_CLI_SHOW_DEBUG_EXE
/*
 * cli_vterm_exec_command()
 * @command
 * return : error status
 */
int cli_vterm_exec_command(const char *command)
{
	int rv, quiet = 0;
	unsigned int invalid_arg_index, node_counter = 0;
	struct ile_cli *cli = cli_data_get();

	if (command != NULL) {
		quiet = 1;
		if (cli_vterm_upload_command_into_buffer(command) != 0)
			return -1;
	}

	if (!cli->len)
		return -1;

#ifdef ILE_CLI_SHOW_DEBUG_EXE
	cli_info_print(ILE_CLI_MAGENTA_COLOUR, "\n\n[ile-cli]:");
	cli_info_print(ILE_CLI_WHITE_COLOUR, "term execution test!\n");
	cli_info_print(ILE_CLI_RED_COLOUR, "[ile-cli]:");
	cli_info_print(ILE_CLI_WHITE_COLOUR,
	               "cursore : %d, len : %d!\n",
	               cli->cursor,
	               cli->len);

	cli_info_print(ILE_CLI_MAGENTA_COLOUR, "[ile-cli]:");
	cli_info_print(ILE_CLI_WHITE_COLOUR, "buffer : %s ;\n", cli->buffer);
#endif

	cli_vterm_parse_args();
	ile_hist_command_write(cli->buffer, cli->len);
	rv = ile_cli_cmd_tree_branch_exe(cli->argc, cli->argv, &node_counter);
	if (!quiet && rv == 1) {
		invalid_arg_index = cli_banner_len_get();
		if (node_counter) {
			if (node_counter == cli->argc)
				invalid_arg_index += cli->argv[node_counter - 1] - cli->argv[0];
			else if (node_counter < cli->argc)
				invalid_arg_index += cli->argv[node_counter] - cli->argv[0];
		}
		cli_cursor_new_line(invalid_arg_index);
		cli_info_print(ILE_CLI_WHITE_COLOUR, "^ Error in commands line argument!");
	}
	cli_info_print(ILE_CLI_MAGENTA_COLOUR, "\n");

#ifdef ILE_CLI_SHOW_DEBUG_EXE
	if (cli->argc) {
		cli_info_print(ILE_CLI_MAGENTA_COLOUR, "[ile-cli]: argc = %d \n", cli->argc);
		for(register int i = 0; i < cli->argc; ++i) {
			cli_info_print(ILE_CLI_MAGENTA_COLOUR, "[ile-cli]:");
			cli_info_print(ILE_CLI_WHITE_COLOUR, "%d - %s ;\n", i + 1, cli->argv[i]);
		}
	}
#endif

	ile_hist_reset_index();
	cli_vterm_reset();
	if (!quiet)
		cli_banner_print();
	return 0;
}

/*
 * ile_clean_exec()
 */
static int ile_clean_exec(node_t self, const int argc, char **const argv)
{
	ile_hist_clean();
	return 0;
}

/*
 * cli_clear_exec()
 */
static int cli_clear_exec(node_t self, const int argc, char **const argv)
{
	ile_cli_clear();
	return 0;
}

/*
 * ile_hist_print()
 */
static int ile_hist_print(void)
{
	char buf[ILE_CLI_MAX_CMD_LEN];
	unsigned int len = 1, i = 0;
	ile_hist_reset_index();
	for(i = 0; i < ILE_HISTORY_BUFFER_SIZE/2; ++i) {
		len = ile_hist_command_read_prev(buf);
		if (!len)
			break;
		buf[len] = 0;
		cli_info_print(ILE_CLI_WHITE_COLOUR, "%d : %s\n", i, buf);
	}
	return 0;
}

/*
 * ile_cli_hist_print_exec()
 */
static int ile_cli_hist_print_exec(node_t self, const int argc, char **const argv)
{
	return ile_hist_print();
}

/*
 * ile_cli_banner_name_exec()
 */
static int ile_cli_banner_name_exec(node_t self, const int argc, char **const argv)
{
	if (strlen(argv[3]) <= sizeof(banner_name)) {
		strncpy(banner_name, argv[3], sizeof(banner_name));
		cli_banner_name_set(0, banner_name);
		return 0;
	}
	cli_info_print(ILE_CLI_WHITE_COLOUR,
	               "[ile-cli] Invalid argument length : %d; limit - %d\n",
	               strlen(argv[3]), sizeof(banner_name));
	return -2;
}

/*
 * ile_cli_banner_name_exec()
 */
static int ile_cli_about_exec(node_t self, const int argc, char **const argv)
{
	cli_info_print(ILE_CLI_MAGENTA_COLOUR,
	               "\n    Improved Lightweight Embedded Command Line Interface\n\n");
	cli_info_print(ILE_CLI_WHITE_COLOUR,
	               "Console user interface for MCU, RTOS, and C applications with\n");
	cli_info_print(ILE_CLI_WHITE_COLOUR,
	               "tab completions for each of the command tree nodes. Use it to\n");
	cli_info_print(ILE_CLI_WHITE_COLOUR,
	               "create your complex management plane with many command branches.\n");
	return 0;
}

/*
 * ile_cli_debug_lvl_exec()
 */
static int ile_cli_debug_lvl_exec(node_t self, const int argc, char **const argv)
{
	unsigned int i, lvl;
	int rv = -1, print_all = 0;
	struct debug_level_label {
		unsigned int lvl;
		const char *label;
	} debug_lvl_lbl[] = {
		{DBG_FATAL,   "FATAL"},
		{DBG_ERROR,   "ERROR"},
		{DBG_WARNING, "WARNING"},
		{DBG_LOG,     "LOG"},
		{DBG_INFO,    "INFO"},
	};

	if (argc == 2) {
		lvl = ile_debug_level_get();
		rv = 0;
	} else if (argc == 3) {
		lvl = (unsigned int) atol(argv[2]);
		print_all = 1;
		if (lvl > MAX_DEBUG_LEVEL) {
			cli_info_print(ILE_CLI_WHITE_COLOUR, "Invalid argument. Use the following values:\n\n");
		} else {
			ile_debug_level_set((enum debug_level)lvl);
			cli_info_print(ILE_CLI_WHITE_COLOUR, "Set %d level.\n\n", lvl);
			rv = 0;
		}
	} else {
		return -2;
	}
	for(i = 0; i < (sizeof(debug_lvl_lbl)/sizeof(debug_lvl_lbl[0])); ++i) {
		if (lvl == debug_lvl_lbl[i].lvl || print_all) {
			cli_info_print(ILE_CLI_WHITE_COLOUR, "%d : %s\n", debug_lvl_lbl[i].lvl, debug_lvl_lbl[i].label);
		}
	}
	return rv;
}

/*
 * ile_cli_banner_colour_exec()
 */
static int ile_cli_banner_colour_set_exec(node_t self, const int argc, char **const argv)
{
	register unsigned int i;
	struct banner_colour_cmd_label {
		enum colour_type colour;
		const char *label;
	} debug_banner_label[] = {
		{ILE_CLI_RED_COLOUR,     "red"    },
		{ILE_CLI_GREEN_COLOUR,   "green"  },
		{ILE_CLI_YELLOW_COLOUR,  "yellow" },
		{ILE_CLI_BLUE_COLOUR,    "blue"   },
		{ILE_CLI_MAGENTA_COLOUR, "magenta"},
		{ILE_CLI_CYAN_COLOUR,    "cyan"   },
		{ILE_CLI_WHITE_COLOUR,   "white"  },
	};
	if (!strcmp("reset", argv[3])) {
		cli_banner_colour_set(0, ILE_CLI_RESET_COLOUR);
		return 0;
	}
	for(i = 0; i < (sizeof(debug_banner_label)/sizeof(debug_banner_label[0])); ++i) {
		if (!strcmp(debug_banner_label[i].label, argv[3])) {
			cli_banner_colour_set(0, debug_banner_label[i].colour);
			return 0;
		}
	}
	cli_info_print(ILE_CLI_WHITE_COLOUR,
	               "[ile-cli] Invalid argument : %s\n%s\n",
				   argv[3], ile_cli_cmd_node_info_get(self));
	return -1;
}

/*
 * ile_cli_cmd_minimal_tree_build()
 * return error status
 */
static int ile_cli_cmd_minimal_tree_build(void)
{
	node_t cli_root = ile_cli_cmd_root_node_add(NULL, " > ", "ile-cli");
	ile_command_root_node_set(cli_root);
	node_t line = ile_cli_cmd_node_add(cli_root, "line", "Console configuration");
	node_t banner = ile_cli_cmd_node_add(line, "banner", "Banner configuration");
	node_t bnr_colour = ile_cli_cmd_node_add(banner, "colour", "Banner colour");
	node_t bnr_colour_set = ile_cli_cmd_exec_node_flags_add(bnr_colour,
		"<colour>", "Enter <red | green | yellow | blue | magenta | cyan | white>",
		ile_cli_banner_colour_set_exec, ILE_CMD_FLAG(UNCHECKED));
	node_t bnr_colour_reset = ile_cli_cmd_exec_node_add(bnr_colour, "reset", "Reset banner colour",
		ile_cli_banner_colour_set_exec);

	node_t bnr_name = ile_cli_cmd_node_add(banner, "name", "Banner name");
	node_t bnr_name_set = ile_cli_cmd_exec_node_flags_add(bnr_name, "<name>", "Enter banner name",
		ile_cli_banner_name_exec, ILE_CMD_FLAG(UNCHECKED));

	node_t history = ile_cli_cmd_exec_node_add(line, "history", "Show history", ile_cli_hist_print_exec);
	node_t hist_clear = ile_cli_cmd_exec_node_add(history, "clear", "Clear history", ile_clean_exec);
	node_t cli_clear = ile_cli_cmd_exec_node_add(line, "clear", "Clear console screen", cli_clear_exec);

	node_t cli_about = ile_cli_cmd_exec_node_add(line, "about", "Show information about line", ile_cli_about_exec);

	node_t debug = ile_cli_cmd_node_add(cli_root, "debug", "Debug configuration");
	node_t debug_level = ile_cli_cmd_exec_node_add(debug, "level", "Show/Set debug level",ile_cli_debug_lvl_exec);
	node_t debug_level_set = ile_cli_cmd_exec_node_flags_add(debug_level, "<level>", "Enter debug level <0 - 5>",
		ile_cli_debug_lvl_exec, ILE_CMD_FLAG(UNCHECKED));

	if (!bnr_colour_set || !bnr_name_set || !bnr_colour_reset || !hist_clear || !cli_clear ||
	    !cli_about || !debug_level_set) {
		cli_info_print(ILE_CLI_RED_COLOUR, "Error: The command tree has not been built!\n");
		return -1;
	}

	return 0;
}


/* ANSI escape code status */
enum escape_code_status {
	ILE_CLI_ESC_DONE = 0,
	ILE_CLI_ESC_BRACKET,
	ILE_CLI_ESC_FN_O,
	ILE_CLI_ESC_HOME,
	ILE_CLI_ESC_END
};

/*
 * cli_vterm_escape_code_proc()
 * @escape_code
 */
static void cli_vterm_escape_proc(unsigned short escape, int *status)
{
	struct ile_cli *cli = cli_data_get();
	switch (escape) {
	case '[':
		*status = ILE_CLI_ESC_BRACKET;
		break;

	case 'O':
		*status = ILE_CLI_ESC_FN_O;
		break;

	case 'A':  /* '[' + UP */
		if (*status == ILE_CLI_ESC_BRACKET) {
			cli_vterm_history_restore(1);
		}
		*status = ILE_CLI_ESC_DONE;
		break;
	case 'B':  /* '[' + DOWN */
		if (*status == ILE_CLI_ESC_BRACKET) {
			cli_vterm_history_restore(0);
		}
		*status = ILE_CLI_ESC_DONE;
		break;

	case 'C':  /* '[' + right */
		if (*status == ILE_CLI_ESC_BRACKET) {
			cli_move_cursor_right(1);
		}
		*status = ILE_CLI_ESC_DONE;
		break;

	case 'D':  /* '[' + left */
		if (*status == ILE_CLI_ESC_BRACKET) {
			cli_move_cursor_left(1);
		}
		*status = ILE_CLI_ESC_DONE;
		break;

	case 'H':  /* HOME */
		if (*status == ILE_CLI_ESC_FN_O) {
			cli_move_cursor_left(cli->cursor);
		}
		*status = ILE_CLI_ESC_DONE;
		break;

	case 'F':  /* END */
		if (*status == ILE_CLI_ESC_FN_O) {
			cli_move_cursor_right(cli->len - cli->cursor);
		}
		*status = ILE_CLI_ESC_DONE;
		break;

	case '7':  /* HOME */
		if (*status == ILE_CLI_ESC_BRACKET) {
			*status = ILE_CLI_ESC_HOME;
		}
		break;

	case '8':  /* END */
		if (*status == ILE_CLI_ESC_BRACKET) {
			*status = ILE_CLI_ESC_END;
		}
		break;

	case '~':
		if (*status == ILE_CLI_ESC_HOME) {
			cli_move_cursor_left(cli->cursor);
		} else if (*status == ILE_CLI_ESC_END) {
			cli_move_cursor_right(cli->len - cli->cursor);
		}
		break;

	default:
		*status = ILE_CLI_ESC_DONE;
		break;
	}
	return;
}

/*
 * cli_vterm_init()
 * @op
 */
void cli_vterm_init(struct ile_cli_operations *op)
{
	strncpy(banner_name, ILE_CLI_MAIN_BANNER_DEFAULT_NAME, sizeof(banner_name));
	cli_banner_set(0, ILE_CLI_GREEN_COLOUR, banner_name);
	cli_banner_set(1, ILE_CLI_WHITE_COLOUR, ILE_CLI_MAIN_ROOT_NODE_LABEL);

	if (!op->char_get || !op->char_output) {
		cli_info_print(ILE_CLI_RED_COLOUR, "Error initializing operations!\n");
		return;
	}
	ile_cli_cmd_minimal_tree_build();
	if (op->tree_build)
		op->tree_build();
	char_get = op->char_get;
	ile_cmd_node_allocator_override(op);
	console_output_op_set(op->char_output);
}

/*
 * cli_vterm_char_proc()
 */
void cli_vterm_char_proc(void)
{
	char c;
	unsigned short ascii_char;
	bool is_escape_seq = false;
	int escape_status = ILE_CLI_ESC_DONE;

	cli_banner_print();
	while(1) {
		ascii_char = char_get();

		if (is_escape_seq) {
			cli_vterm_escape_proc(ascii_char, &escape_status);
			if (escape_status == ILE_CLI_ESC_DONE)
				is_escape_seq = false;
			continue;
		}

		switch (ascii_char) {
		case ILE_CLI_CMD_KEY_ESC:
			is_escape_seq = true;
			break;

		case ILE_CLI_CMD_KEY_CR:
			break;
		case ILE_CLI_CMD_KEY_LF:
			cli_vterm_exec_command(NULL);
			break;

		case ILE_CLI_CMD_KEY_DEL:
		case ILE_CLI_CMD_KEY_BS:
			cli_text_remove(1);
			break;

		case ILE_CLI_CMD_KEY_ETX:
			break;

		case ILE_CLI_CMD_KEY_HT:
			cli_vterm_tab();
			break;

		case '?':
			cli_info_print(ILE_CLI_MAGENTA_COLOUR, "?\n");
			cli_vterm_question();
			break;

		default:
			c = (char)ascii_char;
			cli_text_print(&c, 1);
		}
	}
}

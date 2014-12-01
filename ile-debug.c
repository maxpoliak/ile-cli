/***************************************************************************\
#                               ile-debug.c                                 #
\***************************************************************************/

#include <ile-debug.h>
#include <ile-cli-core.h>
#include <stdio.h>
#include <stdarg.h>

#ifdef ILE_DEBUG_CONSOLE_SET

#define ILE_CLI_DEBUG_BUFFER_SIZE	256

struct debug_banner_label {
	enum colour_type colour;
	const char *label;
};

static struct debug_banner_label debug_banner_label[MAX_DEBUG_LEVEL] = {
	{ILE_CLI_YELLOW_COLOUR, "FATAL"  },
	{ILE_CLI_RED_COLOUR,    "ERROR"  },
	{ILE_CLI_CYAN_COLOUR,   "WARNING"},
	{ILE_CLI_GREEN_COLOUR,  "LOG"    },
	{ILE_CLI_BLUE_COLOUR,   "INFO"   },
};

static enum debug_level dbg_lvl = DBG_ERROR;

/*
 * ile_debug_level_set()
 * @level
 * return: status
 */
void ile_debug_level_set(enum debug_level level)
{
	if (level >= MAX_DEBUG_LEVEL)
		dbg_lvl = DBG_INFO;
	dbg_lvl = level;
}

/*
 * ile_debug_level_get()
 * return: debug level
 */
enum debug_level ile_debug_level_get(void)
{
	return dbg_lvl;
}

/*
 * ile_debug_print()
 * return: debug level
 */
int ile_debug_print(enum debug_level lvl, const char *debug_message, ...)
{
	int rc = -1;
	char buff[ILE_CLI_DEBUG_BUFFER_SIZE];
	enum colour_type cli_current_colour = cli_colour_get();

	if (lvl > ile_debug_level_get())
		return rc;

	cli_info_print(debug_banner_label[MAX_DEBUG_LEVEL].colour,
	               "[%s] ",
	               debug_banner_label[MAX_DEBUG_LEVEL].label);

	va_list args;
	va_start(args, debug_message);
	rc = vsprintf(buff, debug_message, args);
	va_end(args);
	cli_info_print(cli_current_colour, "%s", buff);
	return rc;
}
#endif

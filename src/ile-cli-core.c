/***************************************************************************\
#                               ile-cli-core.с                              #
\***************************************************************************/

#include <ile-cli-api.h>
#include <ile-history.h>
#include <ile-cli-core.h>


#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>

#ifndef ILE_CLI_MAX_BANER_NUM
#define ILE_CLI_MAX_BANER_NUM	3
#endif

#ifndef ILE_CLI_MAX_NUM_ARGS
#define ILE_CLI_MAX_NUM_ARGS	15
#endif

#ifndef ILE_CLI_MESSAGE_BUFFER_SIZE
#define ILE_CLI_MESSAGE_BUFFER_SIZE	256
#endif

#define OUTPUT_SYSTEM_BUFFER		16

#ifndef CMD_ENDL
#define CMD_ENDL "\n"
#endif

struct ile_cli cli;

static void console_null(const char* c) {
	return;
}

static void (*console) (const char*) = console_null;

void console_output_op_set(void (*console_output) (const char*)) {
	console = console_output;
}

struct ile_cli* cli_data_get(void)
{
	return &cli;
}

static inline void new_line(void)
{
	console(CMD_ENDL);
}

static inline void backspace(void)
{
	console("\033[D \033[D");
}

void cli_remove_from_cursore(void)
{
	console("\033[K");
}

void ile_cli_clear(void)
{
    console("\33[H\33[2J");
}

/*
 * remove_before_curs() - deletes all characters after the cursor
 * @count: number of characters
 */
static void remove_before_curs(unsigned int count)
{
	register unsigned int i;
	if (count >= ILE_CLI_MAX_CMD_LEN)
		count = ILE_CLI_MAX_CMD_LEN - 1;
	for(i = 0; i < count; backspace(), ++i);
}

/*
 * colour_set() - set the terminal character color
 * @colour
 */
static void colour_set(enum colour_type colour)
{
#ifdef ILE_COLOUR_SET
	char buff[OUTPUT_SYSTEM_BUFFER];
	snprintf(buff, OUTPUT_SYSTEM_BUFFER, "\033[%dm", colour);
	console(buff);
#endif
}

/*
 * move_cursor() - move the cursor carriage
 * @offset: carriage offset
 * @dir: direction
 */
static void move_cursor(unsigned int offset, enum cur_direction dir)
{
	char buff[OUTPUT_SYSTEM_BUFFER];
	if (!offset)
		return;
	snprintf(buff, OUTPUT_SYSTEM_BUFFER, "\033[%d%c", offset, dir);
	console(buff);
}

/*
 * cur_pos_set() - set the cursor carriage to the specified coordinates
 * @row
 * @column
 */
static void cur_pos_set(unsigned char row, unsigned char column)
{
	char buff[OUTPUT_SYSTEM_BUFFER];
	snprintf(buff, OUTPUT_SYSTEM_BUFFER, "\033[%d;%dH", row, column);
	console(buff);
}

/*
 * cli_reset_cursor()
 */
void cli_reset_cursor(void)
{
	// TODO: \r
	move_cursor(1, ILE_CLI_CUR_DIR_UP);
	new_line();
}


/*
 * move_cursor_right()
 * @offset: carriage offset
 */
void cli_move_cursor_right(unsigned int offset)
{
	struct ile_cli *cli = cli_data_get();
	if (cli->cursor + offset <= cli->len) {
		move_cursor(offset, ILE_CLI_CUR_DIR_FORWARD);
		cli->cursor += offset;
	}
}

/*
 * move_cursor_left()
 * @offset: carriage offset
 */
void cli_move_cursor_left(unsigned int offset)
{
	struct ile_cli *cli = cli_data_get();
	if (cli->cursor >= offset) {
		move_cursor(offset, ILE_CLI_CUR_DIR_BACKWARD);
		cli->cursor -= offset;
	}
}

/*
 * move_cursor_left()
 * @offset: carriage offset
 */
void cli_cursor_new_line(unsigned int offset)
{
	new_line();
	move_cursor(offset, ILE_CLI_CUR_DIR_FORWARD);
}

/*
 * cli_colour_get() - set cli character color
 */
enum colour_type cli_colour_get()
{
	return cli_data_get()->console_colour;
}

/*
 * cli_colour_set() - return cli character color
 */
void cli_colour_set(enum colour_type colour)
{
	cli_data_get()->console_colour = colour;
	colour_set(colour);
}

/*
 * cli_info_print() - print string
 * @colour
 * @format
 */
int cli_info_print(enum colour_type colour, const char *format, ...)
{
	enum colour_type cli_current_colour;
	if (colour != ILE_CLI_WHITE_COLOUR) {
		cli_current_colour = cli_colour_get();
		colour_set(colour);
	}
	char buff[ILE_CLI_MESSAGE_BUFFER_SIZE];
	va_list args;
	int rc;
	va_start(args, format);
	rc = vsprintf(buff, format, args);
	va_end(args);
	console(buff);
	if (colour != ILE_CLI_WHITE_COLOUR)
		colour_set(cli_current_colour);
	return rc;
}

/*
 * cli_banner_name_set() - set cli banner
 * @banner_number: baner number
 * @name
 */
void cli_banner_name_set(unsigned int banner_number,
                         const char *name)
{
	if (banner_number > ILE_CLI_MAX_BANER_NUM)
		return;
	cli_data_get()->banner[banner_number].name = name;
}

/*
 * cli_banner_colour_set() - set cli banner
 * @banner_number: baner number
 * @colour
 */
void cli_banner_colour_set(unsigned int banner_number,
                           enum colour_type colour)
{
	struct ile_cli *cli = cli_data_get();
	if (banner_number > ILE_CLI_MAX_BANER_NUM)
		return;
	if (cli->banner[banner_number].name)
		cli->banner[banner_number].colour = colour;
}

/*
 * cli_banner_set() - set cli banner
 * @banner_number: baner number
 * @colour
 * @name
 */
void cli_banner_set(unsigned int banner_number,
                    enum colour_type colour,
                    const char *name)
{
	cli_banner_name_set(banner_number, name);
	cli_banner_colour_set(banner_number, colour);
}


/*
 * cli_banner_remove() - remove cli banner
 * @banner_number: baner number
 */
void cli_banner_remove(unsigned int banner_number)
{
	if (banner_number > ILE_CLI_MAX_BANER_NUM)
		return;
	cli_data_get()->banner[banner_number].name = NULL;
}

/*
 * cli_banner_print() - print command line banner
 */
void cli_banner_print(void)
{
	const struct ile_cli *cli = cli_data_get();
	for(unsigned int i = 0; i < ILE_CLI_MAX_BANER_NUM; ++i) {
		if (cli->banner[i].name) {
			colour_set(cli->banner[i].colour);
			console(cli->banner[i].name);
			colour_set(cli->console_colour);
		}
	}
}

/*
 * cli_banner_len_get()
 */
unsigned int cli_banner_len_get(void)
{
	const struct ile_cli *cli = cli_data_get();
	unsigned int banner_len = 0;
	for (unsigned int i = 0; i < ILE_CLI_MAX_BANER_NUM; i++) {
		if (cli->banner[i].name) {
			banner_len += strlen(cli->banner[i].name);
		}
	}
	return banner_len;
}

/*
 * cli_buffer_clear() - reset cli params to clear its buffer
 */
void cli_buffer_clear(void)
{
	struct ile_cli *cli = cli_data_get();
	memset(cli->buffer, 0, sizeof(cli->buffer));
	cli->cursor = 0;
	cli->len = 0;
	memset(cli->argv, 0, sizeof(cli->argv));
	cli->argc = 0;
}

/*
 * cli_buffer_text_insert() - inserts text into the terminal's virtual buffer.
 * @text: text string;
 * @len:  length of text string.
 */
static void cli_buffer_text_insert(const char *text, uint16_t len)
{
	struct ile_cli *cli = cli_data_get();
	memmove(cli->buffer + cli->cursor + len,
	        cli->buffer + cli->cursor,
	        cli->len - cli->cursor);
	memcpy(cli->buffer + cli->cursor, text, len);
	cli->cursor += len;
	cli->len += len;
	cli->buffer[cli->len] = '\0';
}

/*
 * cli_buffer_text_remove() - remove text into the terminal's virtual buffer.
 * @len: length of text string for removing.
 */
static void cli_buffer_text_remove(unsigned short len)
{
	struct ile_cli *cli = cli_data_get();
	memmove(cli->buffer + cli->cursor - len,
	        cli->buffer + cli->cursor,
	        cli->len - cli->cursor);
	cli->cursor -= len;
	cli->len -= len;
	cli->buffer[cli->len] = '\0';
}

/*
 * cli_buffer_print() - print text from terminal's virtual buffer to screen.
 * @offset: offset in terminal's virtual buffer.
 */
void cli_buffer_print(unsigned int offset)
{
	const struct ile_cli *cli = cli_data_get();
	console(&cli->buffer[offset]);
	move_cursor(cli->len - cli->cursor, ILE_CLI_CUR_DIR_BACKWARD);
}

/*
 * cli_text_print()
 * @text
 * @len
 */
void cli_text_print(const char *text, unsigned int len)
{
	const struct ile_cli *cli = cli_data_get();
	if (cli->len + len >= ILE_CLI_MAX_CMD_LEN)
		return;
	cli_buffer_text_insert(text, len);
	cli_buffer_print(cli->cursor - len);
}

/*
 * cli_text_remove()
 * @len
 */
void cli_text_remove(unsigned int len)
{
	const struct ile_cli *cli = cli_data_get();
	if (!cli->cursor && cli->cursor < len)
		return;
	move_cursor(len, ILE_CLI_CUR_DIR_BACKWARD);
	cli_remove_from_cursore();
	cli_buffer_text_remove(len);
	cli_buffer_print(cli->cursor);
}

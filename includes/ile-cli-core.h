/***************************************************************************\
#                               ile-cli-core.h                              #
\***************************************************************************/

#ifndef ILE_CLI_CORE_H
#define ILE_CLI_CORE_H

#include <ile-cli-config.h>
#include <ile-cli-api.h>

/* http://www.physics.udel.edu/~watson/scen103/ascii.html */
#define ILE_CLI_CMD_KEY_NUL		0   /* ctrl-@ */
#define ILE_CLI_CMD_KEY_SOH		1   /* start of heading (ctrl-A) */
#define ILE_CLI_CMD_KEY_STX		2   /* start of text (ctrl-B) */
#define ILE_CLI_CMD_KEY_ETX		3   /* end of text (ctrl-C) */
#define ILE_CLI_CMD_KEY_EOT		4   /* end of xmit (ctrl-D) */
#define ILE_CLI_CMD_KEY_ENQ		5   /* enquiry (ctrl-E) */
#define ILE_CLI_CMD_KEY_ACK		6   /* acknowledge (ctrl-F) */
#define ILE_CLI_CMD_KEY_BEL		7   /* bell (ctrl-G) */
#define ILE_CLI_CMD_KEY_BS		8   /* backspace (ctrl-H) */
#define ILE_CLI_CMD_KEY_HT		9   /* horizontal tab (ctrl-I) */
#define ILE_CLI_CMD_KEY_LF		10  /* line feed (ctrl-J) */
#define ILE_CLI_CMD_KEY_VT		11  /* vertical tab (ctrl-K) */
#define ILE_CLI_CMD_KEY_FF		12  /* form feed (ctrl-L) */
#define ILE_CLI_CMD_KEY_CR		13  /* carriage feed (ctrl-M) */
#define ILE_CLI_CMD_KEY_SO		14  /* shift out (ctrl-N) */
#define ILE_CLI_CMD_KEY_SI		15  /* shift in (ctrl-O) */
#define ILE_CLI_CMD_KEY_DLE		16  /* data line escape (ctrl-P) */
#define ILE_CLI_CMD_KEY_DC1		17  /* device control 1 (ctrl-Q) */
#define ILE_CLI_CMD_KEY_DC2		18  /* device control 2 (ctrl-R) */
#define ILE_CLI_CMD_KEY_DC3		19  /* device control 3 (ctrl-S) */
#define ILE_CLI_CMD_KEY_DC4		20  /* device control 4 (ctrl-T) */
#define ILE_CLI_CMD_KEY_NAK		21  /* neg acknowledge	(ctrl-U) */
#define ILE_CLI_CMD_KEY_SYN		22  /* synchronous idel (ctrl-V) */
#define ILE_CLI_CMD_KEY_ETB		23  /* end of xmit block (ctrl-W) */
#define ILE_CLI_CMD_KEY_CAN		24  /* cancel (ctrl-X) */
#define ILE_CLI_CMD_KEY_EM		25  /* end of medium (ctrl-Y) */
#define ILE_CLI_CMD_KEY_SUB		26  /* substitute (ctrl-Z) */
#define ILE_CLI_CMD_KEY_ESC		27  /* escape (ctrl-[) */
#define ILE_CLI_CMD_KEY_FS		28  /* file separator (ctrl-\) */
#define ILE_CLI_CMD_KEY_GS		29  /* group separator (ctrl-]) */
#define ILE_CLI_CMD_KEY_RS		30  /* record separator (ctrl-^) */
#define ILE_CLI_CMD_KEY_US		31  /* unit separator (ctrl-_) */

#define ILE_CLI_CMD_KEY_DEL		127 /* delete */

#ifndef ILE_CLI_MAX_CMD_LEN
#define ILE_CLI_MAX_CMD_LEN		64
#endif

enum cur_direction {
	ILE_CLI_CUR_DIR_UP       = 'A',
	ILE_CLI_CUR_DIR_DOWN     = 'B',
	ILE_CLI_CUR_DIR_FORWARD  = 'C',
	ILE_CLI_CUR_DIR_BACKWARD = 'D',
};

struct cli_banner {
	const char*       name;
	enum colour_type  colour;
};

struct ile_cli {
	char              buffer[ILE_CLI_MAX_CMD_LEN];
	unsigned int      cursor;
	int               argc;
	char*             argv[ILE_CLI_MAX_NUM_ARGS];
	unsigned int      len;
	enum colour_type  console_colour;
	struct cli_banner banner[ILE_CLI_MAX_BANER_NUM];
};

struct ile_cli* cli_data_get(void);
void console_output_op_set(void (*console_output) (const char*));
enum colour_type cli_colour_get(void);
void ile_cli_clear(void);
void cli_remove_from_cursore(void);
void cli_move_cursor_right(unsigned int offset);
void cli_move_cursor_left(unsigned int offset);
void cli_reset_cursor(void);
void cli_cursor_new_line(unsigned int offset);
void cli_buffer_print(unsigned int offset);
void cli_buffer_clear(void);
unsigned int cli_banner_len_get(void);
void cli_banner_print(void);
void cli_banner_name_set(unsigned int banner_number, const char *name);
void cli_banner_colour_set(unsigned int banner_number, enum colour_type colour);
void cli_banner_set(unsigned int banner_number, enum colour_type colour, const char *name);
void cli_text_print(const char *text, unsigned int len);
void cli_text_remove(unsigned int len);

#endif /* ILE_CLI_CORE_H */

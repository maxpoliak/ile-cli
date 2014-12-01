/***************************************************************************\
#                               ile-history.с                               #
\***************************************************************************/

#include <ile-history.h>
#include <string.h>

#define ILE_CLI_RING_BUF_MOVE_NEXT(index) \
        (index = index < (ILE_HISTORY_BUFFER_SIZE - 1) ? index + 1 : 0)

#define ILE_CLI_RING_BUF_MOVE_PREV(index) \
        (index = index ? index - 1 : (ILE_HISTORY_BUFFER_SIZE - 1))

struct ile_history {
	char         ring_buffer[ILE_HISTORY_BUFFER_SIZE];
	unsigned int index;
	unsigned int last;
	unsigned int head;
	unsigned int tail;
	int          overflowed;
} hist;

/*
 * ile_history_data_get()
 */
inline static struct ile_history* ile_hist_data_get()
{
	return &hist;
}

/*
 * ile_hist_ring_buf_is_empty()
 * return: status
 */
inline static int ile_hist_ring_buf_is_empty(void)
{
	const struct ile_history *hist = ile_hist_data_get();
	if (hist->head == hist->tail)
		return 1;
	return 0;
}

/*
 * ile_hist_ring_buf_is_idx_at_first()
 * return: status
 */
inline static int ile_hist_ring_buf_is_idx_at_first(void)
{
	const struct ile_history *hist = ile_hist_data_get();
	if (hist->index == hist->head)
		return 1;
	return 0;
}

/*
 * ile_hist_ring_buf_is_idx_at_tail()
 * return: status
 */
inline static int ile_hist_ring_buf_is_idx_at_tail(void)
{
	const struct ile_history *hist = ile_hist_data_get();
	if (hist->index == hist->tail)
		return 1;
	return 0;
}

/*
 * ile_hist_ring_buf_move_next()
 * @idx
 * return: new index
 */
static
unsigned int
ile_hist_ring_buf_move_next(unsigned int idx)
{
	const struct ile_history *hist = ile_hist_data_get();
	register unsigned int step = 0;
	int through = 1;
	/*
	 * |------------------------------->|
	 * +-------+--+--+--+--+--+---------+-----------+--+
	 * | [idx] |  |  |  |  |  |    0    | new index |  |
	 * +-------+--+--+--+--+--+---------+-----------+--+
	 *                        | through |
	 */
	while(step++ < ILE_CLI_MAX_CMD_LEN) {
		ILE_CLI_RING_BUF_MOVE_NEXT(idx);
		if (!through)
			break;
		if (hist->ring_buffer[idx] == 0)
			through = 0;
	}
	return idx;
}

/*
 * ile_hist_ring_buf_move_prev()
 * @idx
 */
static
unsigned int
ile_hist_ring_buf_move_prev(unsigned int idx)
{
	const struct ile_history *hist = ile_hist_data_get();
	register unsigned int step = 0;
	int through = 1;
	unsigned int previous;
	/*
	 *               |<------------------------------|
	 * +--+----------+---------+--+--+--+--+---------+-------+--+
	 * |  |    0     | new idx |  |  |  |  |    0    | [idx] |  |
	 * +--+----------+---------+--+--+--+--+---------+-------+--+
	 *    | previous |                     | through |
	 */
	while(step++ < ILE_CLI_MAX_CMD_LEN) {
		previous = idx;
		ILE_CLI_RING_BUF_MOVE_PREV(previous);
		if (hist->ring_buffer[previous] == 0) {
			if (!through) {
				break;
			}
			through = 0;
		}
		idx = previous;
	}
	return idx;
}

/*
 * ile_hist_ring_buf_write()
 * @cli_command
 * @len
 */
static int
ile_hist_ring_buf_write(const char* cli_command, const unsigned int len)
{
	struct ile_history *hist = ile_hist_data_get();
	int rc = 0;
	register unsigned int i, idx;
	register char temp;
	/*
	 *        |------------(tail)--------->|
	 * +--+---+------+--+--+--+--+--+--+---+-------+--+
	 * |  | 0 | tail |  |  |  |  |  |  | 0 | [idx] |  |
	 * +--+---+------+--+--+--+--+--+--+---+-------+--+
	 */
	for(i = 0, idx = hist->tail; i <= len; ++i) {
		temp = (i == len) || cli_command[i] ? cli_command[i] : ' ';
		hist->ring_buffer[idx++] = temp;
		if (idx >= ILE_HISTORY_BUFFER_SIZE) {
			hist->overflowed = 1;
			idx = 0;
		}
	}
	if (hist->overflowed && idx >= hist->head) {
		/*
		 *                    | ------------(X)-------------->|
		 * +--+--+------+--+--+------+--+--+---+-------+--+---+---+
		 * |  |  | tail |  |  | head |  |  | 0 | [idx] |  | 0 | X |
		 * +--+--+------+--+--+------+--+--+---+-------+--+---+---+
		 *       |<------- cli command ------->|
		 */
		hist->head = ile_hist_ring_buf_move_next(idx);
		rc = 1;
	}
	hist->last = hist->tail;
	hist->tail = idx;
	hist->index = hist->tail;
	return rc;
}

/*
 * ile_hist_ring_buf_read()
 * @cli_command
 * return: len
 */
static
unsigned int
ile_hist_ring_buf_read(char *cmd_buf)
{
	const struct ile_history *hist = ile_hist_data_get();
	register unsigned int i, idx;
	/*
	 *                     |---- <read> ---->|
	 * +--+------+--+--+---+-------+--+--+---+------+--+
	 * |  | head |  |  | 0 | index |  |  | 0 | tail |  |
	 * +--+------+--+--+---+-------+--+--+---+------+--+
	 *
	 * ----->|                              |-- <read> -
	 * +--+--+---+------+--+------+--+--+---+-------+--+
	 * |  |  | 0 | tail |  | head |  |  | 0 | index |  |
	 * +--+--+---+------+--+------+--+--+---+-------+--+
	 */
	for(i = 0, idx = hist->index; hist->ring_buffer[idx];
	    ++i, ILE_CLI_RING_BUF_MOVE_NEXT(idx)) {
		cmd_buf[i] = hist->ring_buffer[idx];
		if (i >= ILE_CLI_MAX_CMD_LEN) {
			cmd_buf[ILE_CLI_MAX_CMD_LEN - 1] = 0;
			break;
		}
	}
	return i;
}

/*
 * ile_hist_command_read_prev()
 * @cli_buf
 * return len
 */
unsigned int ile_hist_command_read_prev(char* cli_buf)
{
	struct ile_history *hist = ile_hist_data_get();
	if (ile_hist_ring_buf_is_empty() || ile_hist_ring_buf_is_idx_at_first())
		return 0;
	/*
	 *    |<---- <move prev> ----|
	 * +--+------+--+--+--+--+---+-------+--+--+---+------+
	 * |  | head |  |  |  |  | 0 | index |  |  | 0 | tail |
	 * +--+------+--+--+--+--+---+-------+--+--+---+------+
	 */
	hist->index = ile_hist_ring_buf_move_prev(hist->index);
	return ile_hist_ring_buf_read(cli_buf);
}

/*
 * ile_hist_command_read_prev()
 * @cli_buf
 * return len
 */
unsigned int ile_hist_command_read_next(char* cli_buf)
{
	struct ile_history *hist = ile_hist_data_get();
	if (ile_hist_ring_buf_is_empty() || ile_hist_ring_buf_is_idx_at_tail())
		return 0;
	/*
	 *                     |----- <move next> ---->|
	 * +--+------+--+--+---+-------+--+--+--+--+---+--------+
	 * |  | head |  |  | 0 | index |  |  |  |  | 0 | (tail) |
	 * +--+------+--+--+---+-------+--+--+--+--+---+--------+
	 */
	hist->index = ile_hist_ring_buf_move_next(hist->index);
	if (!ile_hist_ring_buf_is_idx_at_tail())
		return ile_hist_ring_buf_read(cli_buf);
	else
		return 0;
}

/*
 * ile_hist_command_cmp()
 * @cli_command
 * @len
 * return: status
 */
static int
ile_hist_command_cmp(const char* cli_command, const unsigned int len)
{
	const struct ile_history *hist = ile_hist_data_get();
	register int i, idx = hist->last;
	char reference;
	/*
	 *                     |----- <compare> ----->|
	 * +--+------+--+--+---+------+--+--+--+--+---+------+
	 * |  | head |  |  | 0 | last |  |  |  |  | 0 | tail |
	 * +--+------+--+--+---+------+--+--+--+--+---+------+
	 */
	for (i = 0; i <= len; ++i, ILE_CLI_RING_BUF_MOVE_NEXT(idx)) {
		reference = (i == len) || cli_command[i] ? cli_command[i] : ' ';
		if (reference != hist->ring_buffer[idx])
			return 1;
	}
	return 0;
}

/*
 * ile_hist_command_write()
 * @cli_command
 * @len
 */
void ile_hist_command_write(const char* cli_command, const unsigned int len)
{
	if (!ile_hist_ring_buf_is_empty() && !ile_hist_command_cmp(cli_command, len))
		return; /* entry already exists */
	ile_hist_ring_buf_write(cli_command, len);
}

/*
 * ile_hist_reset_index()
 */
void ile_hist_reset_index(void)
{
	struct ile_history *hist = ile_hist_data_get();
	hist->index = hist->tail;
}

/*
 * ile_hist_reset_index()
 */
void ile_hist_clean(void)
{
	struct ile_history *hist = ile_hist_data_get();
	memset(hist->ring_buffer, 0, sizeof(hist->ring_buffer));
	hist->index = 0;
	hist->head = 0;
	hist->tail = 0;
	hist->last = 0;
	hist->overflowed = 0;
}

/***************************************************************************\
#                               ile-history.h                               #
\***************************************************************************/

#ifndef ILE_HISTORY_H
#define ILE_HISTORY_H

#include <ile-cli-config.h>

#ifndef ILE_HISTORY_BUFFER_SIZE
#define ILE_HISTORY_BUFFER_SIZE		96
#endif

void ile_hist_command_write(const char* cli_command, const unsigned int len);
unsigned int ile_hist_command_read_prev(char* cli_buf);
unsigned int ile_hist_command_read_next(char* cli_buf);
void ile_hist_reset_index(void);
void ile_hist_clean(void);

#endif /* ILE_HISTORY_H */

/***************************************************************************\
#                                 debug.h                                   #
\***************************************************************************/
#ifndef ILE_DEBUG_H
#define ILE_DEBUG_H

#include <ile-cli-config.h>

enum debug_level {
	DBG_FATAL = 0,
	DBG_ERROR,
	DBG_WARNING,
	DBG_LOG,
	DBG_INFO,
	MAX_DEBUG_LEVEL
};

#ifdef ILE_DEBUG_CONSOLE_SET

void ile_debug_level_set(enum debug_level level);
enum debug_level ile_debug_level_get(void);
int ile_debug_print(enum debug_level lvl, const char *debug_message, ...);

#define DEBUG(l, s, args...)  ile_debug_print(l, s, ##args)

#else

static inline void ile_debug_level_set(enum debug_level level)
{
	return;
}

static inline enum debug_level ile_debug_level_get(void)
{
	return DBG_FATAL;
}

int ile_debug_print(enum debug_level lvl, const char *debug_message, ...)
{
	return 0;
}

#define DEBUG(l, s, args...)  {}

#endif /* ILE_DEBUG_CONSOLE_SET */

#endif /* ILE_DEBUG_H */

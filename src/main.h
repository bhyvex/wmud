#ifndef __WMUD_MAIN_H__
# define __WMUD_MAIN_H__

# include <glib.h>

extern GMainContext *game_context;
extern guint32 elapsed_seconds;
extern GRand *main_rand;
extern gchar *database_file;
extern GQuark WMUD_DB_ERROR;
extern gchar *admin_email;

/**
 * random_number:
 * @min: Minimum value for random number
 * @max: Maximum value for random number
 *
 * Generates a random number between min and max
 */
#define random_number(min, max) g_rand_int_range(main_rand, (min), (max) + 1)

#endif /* __WMUD_MAIN_H__ */


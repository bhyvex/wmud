/* wMUD - Yet another MUD codebase by W00d5t0ck
 * Copyright (C) 2012 - Gergely POLONKAI
 *
 * configuration.c: configuration file related functions
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <glib.h>

#include "wmud-types.h"
#include "configuration.h"

/**
 * SECTION:configuration
 * @short_description: Configuration file handling functions
 * @title: Configuration file handling functions
 *
 */

/**
 * WMUD_CONFIG_ERROR:
 *
 * the GQuark for the config error GError
 */
GQuark WMUD_CONFIG_ERROR = 0;
/**
 * active_config:
 *
 * the currently active configuration directives
 */
ConfigData *active_config = NULL;

/**
 * wmud_configdata_free:
 * @config_data: pointer to the ConfigData struct to free
 *
 * Correctly free a ConfigData struct with all its members
 */
void
wmud_configdata_free(ConfigData **config_data)
{
	if ((*config_data)->admin_email)
		g_free((*config_data)->admin_email);

	if ((*config_data)->database_file)
		g_free((*config_data)->database_file);

	g_free(*config_data);
	*config_data = NULL;
}

/**
 * wmud_config_init:
 * @config_data: a pointer to a ConfigData struct. This will be filled with the
 *               configuration file's data
 * @err: The GError in which the config handling status should be returned
 *
 * Parses the default configuration file, and sets different variables
 * according to it.
 *
 * Return value: %TRUE if parsing was successful. %FALSE otherwise.
 */
gboolean
wmud_config_init(ConfigData **config_data, GError **err)
{
	GString *config_file = g_string_new(WMUD_CONFDIR);
	GKeyFile *config;
	GError *in_err = NULL;

	if (!config_data)
		return FALSE;

	if (!*config_data)
	{
		g_clear_error(err);
		g_set_error(err, WMUD_CONFIG_ERROR, WMUD_CONFIG_ERROR_REUSE, "Configuration pointer reuse. Please file a bug report!");
		return FALSE;
	}

	*config_data = g_new0(ConfigData, 1);

	g_string_append(config_file, "/wmud.conf");

	config = g_key_file_new();
	/* TODO: Error checking */
	g_key_file_load_from_file(config, config_file->str, 0, &in_err);

	if (!g_key_file_has_group(config, "global"))
	{
		g_set_error(err, WMUD_CONFIG_ERROR, WMUD_CONFIG_ERROR_NOGLOBAL, "Config file (%s) does not contain a [global] group", config_file->str);
		g_key_file_free(config);
		g_string_free(config_file, TRUE);

		return FALSE;
	}

	g_clear_error(&in_err);
	(*config_data)->port = g_key_file_get_integer(config, "global", "port", &in_err);
	if (in_err)
	{
		if (g_error_matches(in_err, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_KEY_NOT_FOUND))
		{
			(*config_data)->port = DEFAULT_PORT;
		}
		else if (g_error_matches(in_err, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_INVALID_VALUE))
		{
			g_set_error(err, WMUD_CONFIG_ERROR, WMUD_CONFIG_ERROR_BADPORT, "Config file (%s) contains an invalid port number", config_file->str);
			g_key_file_free(config);
			g_string_free(config_file, TRUE);
			(*config_data)->port = 0;

			return FALSE;
		}

		return FALSE;
	}

	g_clear_error(&in_err);
	(*config_data)->database_file = g_key_file_get_string(config, "global", "world file", &in_err);
	if (in_err)
	{
		if (g_error_matches(in_err, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_KEY_NOT_FOUND))
		{
			g_set_error(err, WMUD_CONFIG_ERROR, WMUD_CONFIG_ERROR_NOWORLD, "Config file (%s) does not contain a world file path", config_file->str);
			g_key_file_free(config);
			g_string_free(config_file, TRUE);
			wmud_configdata_free(config_data);

			return FALSE;
		}
	}

	g_clear_error(&in_err);
	(*config_data)->admin_email = g_key_file_get_string(config, "global", "admin email", &in_err);
	if (in_err)
	{
		if (g_error_matches(in_err, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_KEY_NOT_FOUND))
		{
			g_set_error(err, WMUD_CONFIG_ERROR, WMUD_CONFIG_ERROR_NOEMAIL, "Config file (%s) does not contain an admin e-mail address", config_file->str);
			g_key_file_free(config);
			g_string_free(config_file, TRUE);
			wmud_configdata_free(config_data);

			return FALSE;
		}
	}

	g_key_file_free(config);
	g_string_free(config_file, TRUE);

	return TRUE;
}

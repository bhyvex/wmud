/* wMUD - Yet another MUD codebase by W00d5t0ck
 * Copyright (C) 2012 - Gergely POLONKAI
 *
 * menu.c: menu handling routines
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

#include <glib.h>

#include "menu.h"
#include "db.h"

#include "menu.h"

/**
 * SECTION:menu
 * @short_description: Menu handling
 * @title: Menu handling routines
 *
 */

GQuark
wmud_menu_error_quark()
{
	return g_quark_from_static_string("wmud-menu-error");
}

gboolean
wmud_menu_items_check(GSList *menu_items, GError **err)
{
	/* TODO: Check for duplicate menuchars */
	/* TODO: Check for duplicate menu texts */
	/* TODO: Check for duplicate placements */
	return TRUE;
}

void
menu_item_free(wmudMenu *menu_item)
{
	if (menu_item->text)
		g_free(menu_item->text);
	if (menu_item->display_text)
		g_free(menu_item->display_text);
	if (menu_item->display_text_ansi)
		g_free(menu_item->display_text_ansi);
	if (menu_item->func)
		g_free(menu_item->func);

	g_free(menu_item);
}

void
wmud_menu_items_free(GSList **menu_items)
{
	if (menu_items)
	{
#if GLIB_CHECK_VERSION(2, 28, 0)
		g_slist_free_full(*menu_items, (GDestroyNotify)menu_item_free);
#else
		g_slist_foreach(*menu_items, (GFunc)menu_item_free, NULL);
		g_slist_free(*menu_items);
#endif
		*menu_items = NULL;
	}
}

void
menu_item_prepare(wmudMenu *item, gpointer data)
{
	gchar m1, m2;
	gchar *a,
	      *found = NULL;
	GString *ds, *dsa;

	g_debug("Preparing menu item %s", item->text);
	m1 = g_ascii_tolower(item->menuchar);
	m2 = g_ascii_toupper(item->menuchar);
	for (a = item->text; *a; a++)
		if ((*a == m1) || (*a == m2))
		{
			found = a;
			break;
		}

	if (found)
	{
		gchar *tmp;

		tmp = g_ascii_strdown(item->text, -1);
		ds = g_string_new(tmp);
		dsa = g_string_new(tmp);
		g_free(tmp);

		ds->str[found - item->text] = g_ascii_toupper(item->menuchar);
		dsa->str[found - item->text] = g_ascii_toupper(item->menuchar);
	}
	else
	{
		found = item->text;
		ds = g_string_new(item->text);
		dsa = g_string_new(item->text);

		g_string_prepend_c(ds, ' ');
		g_string_prepend_c(ds, g_ascii_toupper(item->menuchar));
		g_string_prepend_c(dsa, ' ');
		g_string_prepend_c(dsa, g_ascii_toupper(item->menuchar));
	}
	
	g_string_insert_c(ds, found - item->text, '(');
	g_string_insert_c(ds, found - item->text + 2, ')');

	g_string_insert(dsa, found - item->text, "\x1b[31;1m");
	g_string_insert(dsa, found - item->text + 8, "\x1b[0m");
	item->display_text = g_string_free(ds, FALSE);
	item->display_text_ansi = g_string_free(dsa, FALSE);
	g_debug("Prepared as %s %s", item->display_text, item->display_text_ansi);
}

gboolean
wmud_menu_init(GSList **menu)
{
	GSList *menu_items = NULL;
	GError *in_err = NULL;

	if (!wmud_db_load_menu(&menu_items, &in_err))
	{
		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "Unable to load menu items from the database: %s", in_err->message);
		wmud_menu_items_free(&menu_items);

		return FALSE;
	}

	if (!menu_items)
	{
		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "No menu items were found in the database!");

		return FALSE;
	}

	if (!wmud_menu_items_check(menu_items, &in_err))
	{
		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "Menu items pre-flight check error: %s", in_err->message);
		wmud_menu_items_free(&menu_items);

		return FALSE;
	}

	/* TODO: free previous menu list, if *menu is not NULL */
	*menu = menu_items;

	g_slist_foreach(*menu, (GFunc)menu_item_prepare, NULL);

	return TRUE;
}


#include <glib.h>
#include <gio/gio.h>
#include <string.h>

#include "interpreter.h"
#include "networking.h"
#include "main.h"

#define IS_SPACE(c) (g_ascii_isspace((c)) || (!(c)))

WMUD_COMMAND(gcmd_quit);

static wmudCommand command_list[] = {
	{ "quit", gcmd_quit },
	{ NULL, NULL },
};

static void
destroy_string(GString *string)
{
	g_string_free(string, TRUE);
}

void
wmud_interpret_game_command(wmudClient *client)
{
	GSList *command_parts = NULL;
	gchar *a,
	      *start,
	      *end;
	gchar str_delim = 0;
	wmudCommand *cmd;
	int command_parts_count = 0,
	    match_count = 0;
	GSList *matches = NULL;

	if (strchr(client->buffer->str, '\r') || strchr(client->buffer->str, '\n'))
	{
		/* TODO: We should NEVER reach this point! */
		return;
	}

	a = client->buffer->str;

	GString *token;

	while (*a)
	{
		for (start = a; *start; start++)
		{
			if (!str_delim)
			{
				if ((*start == '"') || (*start == '\''))
				{
					str_delim = *start;
					start++;
					break;
				}
				else if (!IS_SPACE(*start))
				{
					break;
				}
			}
		}

		for (end = start; *end; end++)
		{
			if (!str_delim && strchr("'\" \t", *end))
			{
				break;
			}
			else if (str_delim && (*end == str_delim))
			{
				str_delim = 0;
				break;
			}
			else if (!*end)
			{
				break;
			}
		}

		if (*start)
		{
			token = g_string_new_len(start, end - start);
			command_parts = g_slist_prepend(command_parts, token);
			command_parts_count++;
		}

		a = end;
		if (((*a == '"') || (*a == '\'')) && str_delim)
			a++;
	}

	if (str_delim)
	{
		wmud_client_send(client, "You should close quotes of any kind, like %c, shouldn't you?\r\n", str_delim);
#if GLIB_CHECK_VERSION(2, 28, 0)
		g_slist_free_full(command_parts, (GDestroyNotify)destroy_string);
#else
		g_slist_foreach(command_parts, (GFunc)destroy_string, NULL);
		g_slist_free(command_parts);
#endif
		return;
	}

	if (command_parts_count == 0)
	{
		/* TODO: handle empty command */
		return;
	}

	command_parts = g_slist_reverse(command_parts);

	for (cmd = command_list; cmd->command; cmd++)
	{
		GString *input = (GString *)(command_parts->data);
		gint cmp;

		if (((cmp = g_ascii_strncasecmp(input->str, cmd->command, input->len)) == 0) && !cmd->command[input->len])
		{
			g_slist_free(matches);
			match_count = 1;
			matches = NULL;
			matches = g_slist_prepend(matches, cmd);
			break;
		}
		else if (cmp == 0)
		{
			matches = g_slist_prepend(matches, cmd);
			match_count++;
		}
	}

	switch (match_count)
	{
		case 0:
			switch (random_number(1, 3))
			{
				case 1:
					wmud_client_send(client, "Huh?\r\n");
					break;
				case 2:
					wmud_client_send(client, "What?\r\n");
					break;
				case 3:
					wmud_client_send(client, "I can hardly understand you...\r\n");
					break;
			}
			break;
		case 1:
			((wmudCommand *)(matches->data))->commandFunc(client, ((GString *)(command_parts->data))->str, command_parts->next);
			break;
		default:
			wmud_client_send(client, "This command could mean several things, please try a more exact form!\r\n");
	}

	g_slist_free(matches);
}

WMUD_COMMAND(gcmd_quit)
{
	wmud_client_send(client, "Are you sure you want to get back to that freaky other reality? [y/N] ");
	client->state = WMUD_CLIENT_STATE_QUITWAIT;
}

/*
 * e-shell-migrate.c
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with the program; if not, see <http://www.gnu.org/licenses/>  
 *
 *
 * Copyright (C) 1999-2008 Novell, Inc. (www.novell.com)
 *
 */

#include "e-shell-migrate.h"

#include <string.h>
#include <unistd.h>
#include <glib/gi18n.h>
#include <glib/gstdio.h>
#include <libedataserver/e-xml-utils.h>

#include "e-util/e-bconf-map.h"
#include "e-util/e-error.h"
#include "e-util/e-fsutils.h"
#include "e-util/e-util.h"

#define GCONF_VERSION_KEY	"/apps/evolution/version"
#define GCONF_LAST_VERSION_KEY	"/apps/evolution/last_version"

static const gchar *
shell_migrate_get_old_data_dir (void)
{
	static gchar *old_data_dir = NULL;

	if (G_UNLIKELY (old_data_dir == NULL))
		old_data_dir = g_build_filename (
			g_get_home_dir (), "evolution", NULL);

	return old_data_dir;
}

static gboolean
shell_migrate_attempt (EShell *shell,
                       gint major,
                       gint minor,
                       gint micro)
{
	GList *modules;
	gboolean success = TRUE;

	modules = e_shell_get_shell_modules (shell);

	while (success && modules != NULL) {
		EShellModule *shell_module = modules->data;
		GError *error = NULL;

		success = e_shell_module_migrate (
			shell_module, major, minor, micro, &error);

		if (error != NULL) {
			gint response;

			response = e_error_run (
				NULL, "shell:upgrade-failed",
				error->message, NULL);

			if (response == GTK_RESPONSE_CANCEL)
				success = FALSE;

			g_error_free (error);
		}

		modules = g_list_next (modules);
	}

	return success;
}

static void
shell_migrate_get_version (gint *major,
                           gint *minor,
                           gint *micro)
{
	GConfClient *client;
	const gchar *key;
	const gchar *old_data_dir;
	gchar *string;

	old_data_dir = shell_migrate_get_old_data_dir ();

	key = GCONF_VERSION_KEY;
	client = gconf_client_get_default ();
	string = gconf_client_get_string (client, key, NULL);
	g_object_unref (client);

	if (string != NULL) {
		/* Since 1.4.0 we've kept the version key in GConf. */
		sscanf (string, "%d.%d.%d", major, minor, micro);
		g_free (string);

	} else if (!g_file_test (old_data_dir, G_FILE_TEST_IS_DIR)) {
		/* If the old data directory does not exist,
		 * it must be a new installation. */
		*major = 0;
		*minor = 0;
		*micro = 0;

	} else {
		xmlDocPtr doc;
		xmlNodePtr source;
		gchar *filename;

		filename = g_build_filename (
			old_data_dir, "config.xmldb", NULL);
		doc = e_xml_parse_file (filename);
		g_free (filename);

		if (doc == NULL)
			return;

		source = e_bconf_get_path (doc, "/Shell");
		if (source != NULL) {
			key = "upgrade_from_1_0_to_1_2_performed";
			string = e_bconf_get_value (source, key);
		}

		if (string != NULL && *string == '1') {
			*major = 1;
			*minor = 2;
			*micro = 0;
		} else {
			*major = 1;
			*minor = 0;
			*micro = 0;
		}

		g_free (string);

		if (doc != NULL)
			xmlFreeDoc (doc);
	}
}

static gint
shell_migrate_remove_dir (const gchar *root,
                          const gchar *path)
{
	GDir *dir;
	const gchar *basename;
	gchar *filename;
	gint result = -1;

	/* Recursively removes a directory and its contents. */

	dir = g_dir_open (path, 0, NULL);
	if (dir == NULL)
		return -1;

	while ((basename = g_dir_read_name (dir)) != NULL) {
		filename = g_build_filename (path, basename, NULL);

		/* Make sure we haven't strayed from the evolution dir. */
		g_return_val_if_fail (strlen (path) >= strlen (root), -1);
		g_return_val_if_fail (g_str_has_prefix (path, root), -1);

		if (g_file_test (filename, G_FILE_TEST_IS_DIR)) {
			if (shell_migrate_remove_dir (root, filename) < 0)
				goto fail;
		} else {
			if (g_unlink (filename) < 0)
				goto fail;
		}

		g_free (filename);
		filename = NULL;
	}

	result = g_rmdir (path);

fail:
	g_free (filename);
	g_dir_close (dir);

	return result;
}

gboolean
e_shell_migrate_attempt (EShell *shell)
{
	GConfClient *client;
	const gchar *key;
	const gchar *old_data_dir;
	gint major, minor, micro;
	gint last_major, last_minor, last_micro;
	gint curr_major, curr_minor, curr_micro;
	gboolean migrated = FALSE;
	gchar *string;

	g_return_val_if_fail (E_IS_SHELL (shell), FALSE);

	old_data_dir = shell_migrate_get_old_data_dir ();

	if (sscanf (BASE_VERSION, "%d.%d", &curr_major, &curr_minor) != 2) {
		g_warning ("Could not parse BASE_VERSION (%s)", BASE_VERSION);
		return TRUE;
	}

	curr_micro = atoi (UPGRADE_REVISION);

	shell_migrate_get_version (&major, &minor, &micro);

	if (!(curr_major > major ||
		(curr_major == major && curr_minor > minor) ||
		(curr_minor == minor && curr_micro > micro)))
		goto check_old;

	/* If upgrading from < 1.5, we need to copy most data from
	 * ~/evolution to ~/.evolution.  Make sure we have the disk
	 * space for it before proceeding. */
	if (major == 1 && minor < 5) {
		glong avail;
		glong usage;

		usage = e_fsutils_usage (old_data_dir);
		avail = e_fsutils_avail (g_get_home_dir ());
		if (usage >= 0 && avail >= 0 && avail < usage) {
			gchar *need;
			gchar *have;

			need = g_strdup_printf (_("%ld KB"), usage);
			have = g_strdup_printf (_("%ld KB"), avail);

			e_error_run (
				NULL, "shell:upgrade-nospace",
				need, have, NULL);

			g_free (need);
			g_free (have);

			_exit (EXIT_SUCCESS);
		}
	}

	if (!shell_migrate_attempt (shell, major, minor, micro))
		_exit (EXIT_SUCCESS);

	/* Record a successful migration. */
	client = gconf_client_get_default ();
	string = g_strdup_printf ("%d.%d.%d", major, minor, micro);
	gconf_client_set_string (client, GCONF_VERSION_KEY, string, NULL);
	g_object_unref (client);
	g_free (string);

	migrated = TRUE;

check_old:

	key = GCONF_LAST_VERSION_KEY;
	client = gconf_client_get_default ();

	/* Try to retrieve the last migrated version from GConf. */
	string = gconf_client_get_string (client, key, NULL);
	if (migrated || string == NULL || sscanf (string, "%d.%d.%d",
		&last_major, &last_minor, &last_micro) != 3) {
		last_major = major;
		last_minor = minor;
		last_micro = micro;
	}
	g_free (string);

	/* If the last migrated version was old, check for stuff to remove. */
	if (last_major == 1 && last_minor < 5 &&
		g_file_test (old_data_dir, G_FILE_TEST_IS_DIR)) {

		gint response;

		string = g_strdup_printf (
			"%d.%d.%d", last_major, last_minor, last_micro);
		response = e_error_run (
			NULL, "shel:upgrade-remove-1-4", string, NULL);
		g_free (string);

		switch (response) {
			case GTK_RESPONSE_OK:  /* delete */
				response = e_error_run (
					NULL,
					"shell:upgrade-remove-1-4-confirm",
					NULL);
				if (response == GTK_RESPONSE_OK)
					shell_migrate_remove_dir (
						old_data_dir, old_data_dir);
				else
					break;
				/* fall through */

			case GTK_RESPONSE_ACCEPT:  /* keep */
				last_major = curr_major;
				last_minor = curr_minor;
				last_micro = curr_micro;
				break;

			default:
				break;
		}
	} else {
		last_major = curr_major;
		last_minor = curr_minor;
		last_micro = curr_micro;
	}

	string = g_strdup_printf (
		"%d.%d.%d", last_major, last_minor, last_micro);
	gconf_client_set_string (client, key, string, NULL);
	g_free (string);

	g_object_unref (client);

	return TRUE;
}

GQuark
e_shell_migrate_error_quark (void)
{
	static GQuark quark = 0;

	if (G_UNLIKELY (quark == 0))
		quark = g_quark_from_static_string (
			"e-shell-migrate-error-quark");

	return quark;
}

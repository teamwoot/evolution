/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* e-lang-utils.c - Utility functions for multi-language support.
 *
 * Copyright (C) 2002 Ximian, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Author: Ettore Perazzoli <ettore@ximian.com>
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "e-lang-utils.h"

#include <string.h>


GSList *
e_get_language_list (void)
{
	const char *env;
	const char *p;

	env = g_getenv ("LANGUAGE");
	if (env == NULL) {
		env = g_getenv ("LANG");
		if (env == NULL)
			return NULL;
	}

	p = strchr (env, '=');
	if (p != NULL)
		return g_slist_prepend (NULL, (void *) (p + 1));
	else
		return g_slist_prepend (NULL, (void *) env);
}

void
e_free_language_list (GSList *list)
{
	g_return_if_fail (list != NULL);

	g_slist_free (list);
}

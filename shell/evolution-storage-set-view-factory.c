/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* evolution-storage-set-view-factory.c
 *
 * Copyright (C) 2000  Ximian, Inc.
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
 * Author: Ettore Perazzoli
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "e-storage-set-view.h"
#include "e-shell.h"
#include "evolution-storage-set-view.h"

#include "evolution-storage-set-view-factory.h"

#include <gal/widgets/e-scroll-frame.h>


BonoboControl *
evolution_storage_set_view_factory_new_view (EShell *shell)
{
	EStorageSet *storage_set;
	GtkWidget *storage_set_view;
	BonoboControl *control;
	EvolutionStorageSetView *storage_set_view_interface;
	GtkWidget *scroll_frame;

	g_return_val_if_fail (shell != NULL, NULL);
	g_return_val_if_fail (E_IS_SHELL (shell), NULL);

	storage_set = e_shell_get_storage_set (shell);
	storage_set_view = e_storage_set_new_view (storage_set, NULL /*XXX*/);
	e_storage_set_view_set_allow_dnd (E_STORAGE_SET_VIEW (storage_set_view), FALSE);

	storage_set_view_interface = evolution_storage_set_view_new (E_STORAGE_SET_VIEW (storage_set_view));
	if (storage_set_view_interface == NULL) {
		gtk_widget_destroy (storage_set_view);
		return NULL;
	}

	scroll_frame = e_scroll_frame_new (NULL, NULL);
	e_scroll_frame_set_policy (E_SCROLL_FRAME (scroll_frame),
				   GTK_POLICY_AUTOMATIC,
				   GTK_POLICY_AUTOMATIC);
	e_scroll_frame_set_shadow_type (E_SCROLL_FRAME (scroll_frame),
					GTK_SHADOW_IN);

	gtk_container_add (GTK_CONTAINER (scroll_frame), storage_set_view);

	gtk_widget_show (scroll_frame);
	gtk_widget_show (storage_set_view);

	control = bonobo_control_new (scroll_frame);
	bonobo_object_add_interface (BONOBO_OBJECT (control), BONOBO_OBJECT (storage_set_view_interface));

	return control;
}

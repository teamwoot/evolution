/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  Authors: Jeffrey Stedfast <fejj@ximian.com>
 *
 *  Copyright 2002 Ximian, Inc. (www.ximian.com)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Street #330, Boston, MA 02111-1307, USA.
 *
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-ui-component.h>
#include <bonobo/bonobo-ui-container.h>
#include <bonobo/bonobo-ui-util.h>

#include <gal/util/e-util.h>

#include "folder-browser-window.h"

#include "folder-browser-ui.h"

#define PARENT_TYPE (bonobo_window_get_type ())

static BonoboWindowClass *folder_browser_window_parent_class = NULL;

static void
folder_browser_window_destroy (GtkObject *object)
{
	FolderBrowserWindow *folder_browser_window;
	
	folder_browser_window = FOLDER_BROWSER_WINDOW (object);
	
	if (folder_browser_window->folder_browser) {
		gtk_widget_destroy (GTK_WIDGET (folder_browser_window->folder_browser));
		folder_browser_window->folder_browser = NULL;
	}
	
	if (GTK_OBJECT_CLASS (folder_browser_window_parent_class)->destroy)
		(GTK_OBJECT_CLASS (folder_browser_window_parent_class)->destroy) (object);
}

static void
folder_browser_window_class_init (GtkObjectClass *object_class)
{
	object_class->destroy = folder_browser_window_destroy;
	
	folder_browser_window_parent_class = gtk_type_class (PARENT_TYPE);
}

static void
folder_browser_window_init (GtkObject *object)
{
	;
}


GtkWidget *
folder_browser_window_new (FolderBrowser *fb)
{
	FolderBrowserWindow *new;
	BonoboUIContainer *uicont;
	BonoboUIComponent *uic;
	CORBA_Environment ev;
	
	g_return_val_if_fail (IS_FOLDER_BROWSER (fb), NULL);
	
	new = gtk_type_new (FOLDER_BROWSER_WINDOW_TYPE);
	new = (FolderBrowserWindow *) bonobo_window_construct (BONOBO_WINDOW (new), "Ximian Evolution", "");
	if (!new)
		return NULL;
	
	new->folder_browser = fb;
	bonobo_window_set_contents (BONOBO_WINDOW (new), GTK_WIDGET (fb));
	
	uicont = bonobo_ui_container_new ();
	bonobo_ui_container_set_win (uicont, BONOBO_WINDOW (new));
	
	uic = bonobo_ui_component_new_default ();
	bonobo_ui_component_set_container (uic, BONOBO_OBJREF (uicont));
	
	folder_browser_set_ui_component (FOLDER_BROWSER (fb), uic);
	folder_browser_ui_add_global (fb);
	folder_browser_ui_add_list (fb);
	folder_browser_ui_add_message (fb);
	/*folder_browser_set_shell_view (fb, fb_get_svi (control));*/
	
	return GTK_WIDGET (new);
}

E_MAKE_TYPE (folder_browser_window, "FolderBrowserWindow", FolderBrowserWindow, folder_browser_window_class_init, folder_browser_window_init, PARENT_TYPE);

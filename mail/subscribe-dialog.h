/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  Authors: Chris Toshok <toshok@ximian.com>
 *           Peter Williams <peterw@ximian.com>
 *
 *  Copyright 2000 Ximian, Inc. (www.ximian.com)
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


#ifndef _SUBSCRIBE_DIALOG_H_
#define _SUBSCRIBE_DIALOG_H_

#include <gtk/gtktable.h>
#include <bonobo/bonobo-control.h>
#include <bonobo/bonobo-property-bag.h>
#include <gal/e-table/e-tree-model.h>
#include <gal/e-table/e-table-model.h>
#include "shell/evolution-storage.h"
#include "mail-types.h"
#include "camel/camel-store.h"

#define SUBSCRIBE_DIALOG_TYPE        (subscribe_dialog_get_type ())
#define SUBSCRIBE_DIALOG(o)          (GTK_CHECK_CAST ((o), SUBSCRIBE_DIALOG_TYPE, SubscribeDialog))
#define SUBSCRIBE_DIALOG_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), SUBSCRIBE_DIALOG_TYPE, SubscribeDialogClass))
#define IS_SUBSCRIBE_DIALOG(o)       (GTK_CHECK_TYPE ((o), SUBSCRIBE_DIALOG_TYPE))
#define IS_SUBSCRIBE_DIALOG_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), SUBSCRIBE_DIALOG_TYPE))

typedef struct _SubscribeDialogPrivate SubscribeDialogPrivate;
struct  _SubscribeDialog {
	GtkObject               parent;

	GtkWidget              *app;
	SubscribeDialogPrivate *priv;
};


typedef struct {
	GtkObjectClass parent_class;
} SubscribeDialogClass;

GtkType    subscribe_dialog_get_type         (void);
GtkWidget *subscribe_dialog_new              (void);
void       subscribe_dialog_run_and_close    (SubscribeDialog *dialog);

#endif /* _SUBSCRIBE_DIALOG_H_ */

/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* camelStore.c : Abstract class for an email store */

/* 
 *
 * Author : 
 *  Bertrand Guiheneuf <bertrand@helixcode.com>
 *
 * Copyright 1999, 2000 HelixCode (http://www.helixcode.com) .
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License as 
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */
#include <config.h>
#include "camel-store.h"
#include "camel-log.h"

static CamelServiceClass *parent_class = NULL;

/* Returns the class for a CamelStore */
#define CS_CLASS(so) CAMEL_STORE_CLASS (GTK_OBJECT(so)->klass)

static void _set_separator(CamelStore *store, gchar sep, CamelException *ex);
static CamelFolder *_get_root_folder(CamelStore *store, CamelException *ex);
static CamelFolder *_get_default_folder(CamelStore *store, CamelException *ex);
static void _init(CamelStore *store, CamelSession *session, const gchar *url_name, CamelException *ex);
static CamelFolder *_get_folder (CamelStore *store, const gchar *folder_name, CamelException *ex);
static gchar _get_separator (CamelStore *store, CamelException *ex);

static void _finalize (GtkObject *object);

static void
camel_store_class_init (CamelStoreClass *camel_store_class)
{
	GtkObjectClass *gtk_object_class = GTK_OBJECT_CLASS (camel_store_class);

	parent_class = gtk_type_class (camel_service_get_type ());
	
	/* virtual method definition */
	camel_store_class->init = _init;
	camel_store_class->set_separator = _set_separator;
	camel_store_class->get_separator = _get_separator;
	camel_store_class->get_folder = _get_folder;
	camel_store_class->get_root_folder = _get_root_folder;
	camel_store_class->get_default_folder = _get_default_folder;

	/* virtual method overload */
	gtk_object_class->finalize = _finalize;
}







GtkType
camel_store_get_type (void)
{
	static GtkType camel_store_type = 0;
	
	if (!camel_store_type)	{
		GtkTypeInfo camel_store_info =	
		{
			"CamelStore",
			sizeof (CamelStore),
			sizeof (CamelStoreClass),
			(GtkClassInitFunc) camel_store_class_init,
			(GtkObjectInitFunc) NULL,
				/* reserved_1 */ NULL,
				/* reserved_2 */ NULL,
			(GtkClassInitFunc) NULL,
		};
		
		camel_store_type = gtk_type_unique (CAMEL_SERVICE_TYPE, &camel_store_info);
	}
	
	return camel_store_type;
}





/**
 * camel_store_init: call store's init method
 * @store: the store to initialize
 * @session: session which instantiates the store
 * @url_name: URL defining the store
 *
 * This routine is called by the session object from which this 
 * store is created. It must not be called directly.
 * 
 **/
void 
camel_store_init (CamelStore *store, CamelSession *session, const gchar *url_name, CamelException *ex)
{
	g_assert(store);
	CS_CLASS(store)->init (store, session, url_name, ex);
}


/**
 * init: method called by a session object to  initialize a store object
 * @store: the store to initialize
 * @session: session which instantiates the store
 * @url_name: URL defining the store
 * 
 * This routine is called by the session object from which this 
 * store is created. Be careful, @url_name is used as a private field
 * of the store object. 
 * 
 **/
static void 
_init (CamelStore *store, CamelSession *session, const gchar *url_name, CamelException *ex)
{
	
#warning re-enable assertion here.
	g_assert(session);
	g_assert(url_name);

	store->session = session;
	gtk_object_ref (GTK_OBJECT (session));
	/*store->url_name = url_name;*/
}


static void           
_finalize (GtkObject *object)
{
	CamelStore *camel_store = CAMEL_STORE (object);
	CAMEL_LOG_FULL_DEBUG ("Entering CamelStore::finalize\n");

	/*  if (camel_store->url_name) g_free (camel_store->url_name); */
	if (camel_store->session) gtk_object_unref (GTK_OBJECT (camel_store->session));
	
	GTK_OBJECT_CLASS (parent_class)->finalize (object);
	CAMEL_LOG_FULL_DEBUG ("Leaving CamelStore::finalize\n");
}



/** 
 * camel_store_set_separator: set the character which separates this folder path from the folders names in a lower level of hierarchy.
 *
 * @store:
 * @sep:
 *
 **/
static void
_set_separator (CamelStore *store, gchar sep, CamelException *ex)
{
    store->separator = sep;
}





static gchar
_get_separator (CamelStore *store, CamelException *ex)
{
	g_assert(store);
	return store->separator;
}



/**
 * camel_store_get_separator: return the character which separates this folder path from the folders names in a lower level of hierarchy.
 * @store: store
 * 
 * 
 * 
 * Return value: the separator
 **/
gchar
camel_store_get_separator (CamelStore *store, CamelException *ex)
{
	return  CS_CLASS(store)->get_separator (store, ex);
}







static CamelFolder *
_get_folder (CamelStore *store, const gchar *folder_name, CamelException *ex)
{
	return NULL;
}


/** 
 * camel_store_get_folder: return the folder corresponding to a path.
 * @store: store
 * @folder_name: name of the folder to get
 * 
 * Returns the folder corresponding to the path "name". 
 * If the path begins with the separator caracter, it 
 * is relative to the root folder. Otherwise, it is
 * relative to the default folder.
 * The folder does not necessarily exist on the store.
 * To make sure it already exists, use its "exists" method.
 * If it does not exist, you can create it with its 
 * "create" method.
 *
 *
 * Return value: the folder
 **/
CamelFolder *
camel_store_get_folder (CamelStore *store, const gchar *folder_name, CamelException *ex)
{
	return CS_CLASS(store)->get_folder (store, folder_name, ex);
}


/**
 * camel_store_get_root_folder : return the toplevel folder
 * 
 * Returns the folder which is at the top of the folder
 * hierarchy. This folder is generally different from
 * the default folder. 
 * 
 * @Return value: the tolevel folder.
 **/
static CamelFolder *
_get_root_folder (CamelStore *store, CamelException *ex)
{
    return NULL;
}

/** 
 * camel_store_get_default_folder : return the store default folder
 *
 * The default folder is the folder which is presented 
 * to the user in the default configuration. The default
 * is often the root folder.
 *
 *  @Return value: the default folder.
 **/
static CamelFolder *
_get_default_folder (CamelStore *store, CamelException *ex)
{
    return NULL;
}




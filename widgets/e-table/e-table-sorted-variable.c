/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * E-table-sorted.c: Implements a table that sorts another table
 *
 * Author:
 *   Miguel de Icaza (miguel@gnu.org)
 *
 * (C) 1999 Helix Code, Inc.
 */
#include <config.h>
#include <stdlib.h>
#include <gtk/gtksignal.h>
#include <string.h>
#include "e-util/e-util.h"
#include "e-table-sorted-variable.h"

#define PARENT_TYPE E_TABLE_SUBSET_VARIABLE_TYPE

#define INCREMENT_AMOUNT 100

static ETableSubsetVariableClass *etsv_parent_class;

static void etsv_proxy_model_changed      (ETableModel *etm, ETableSortedVariable *etsv);
static void etsv_proxy_model_row_changed  (ETableModel *etm, int row, ETableSortedVariable *etsv);
static void etsv_proxy_model_cell_changed (ETableModel *etm, int col, int row, ETableSortedVariable *etsv);
static void etsv_sort_info_changed        (ETableSortInfo *info, ETableSortedVariable *etsv);
static void etsv_sort                     (ETableSortedVariable *etsv);
static void etsv_add                      (ETableSubsetVariable *etssv, gint                  row);

static void
etsv_destroy (GtkObject *object)
{
	ETableSortedVariable *etsv = E_TABLE_SORTED_VARIABLE (object);
	ETableSubset *etss = E_TABLE_SUBSET (object);

	gtk_signal_disconnect (GTK_OBJECT (etss->source),
			       etsv->table_model_changed_id);
	gtk_signal_disconnect (GTK_OBJECT (etss->source),
			       etsv->table_model_row_changed_id);
	gtk_signal_disconnect (GTK_OBJECT (etss->source),
			       etsv->table_model_cell_changed_id);
	gtk_signal_disconnect (GTK_OBJECT (etsv->sort_info),
			       etsv->sort_info_changed_id);

	if (etsv->sort_idle_id) {
		g_source_remove(etsv->sort_idle_id);
	}

	etsv->table_model_changed_id = 0;
	etsv->table_model_row_changed_id = 0;
	etsv->table_model_cell_changed_id = 0;
	
	if (etsv->sort_info)
		gtk_object_unref(GTK_OBJECT(etsv->sort_info));
	if (etsv->full_header)
		gtk_object_unref(GTK_OBJECT(etsv->full_header));

	GTK_OBJECT_CLASS (etsv_parent_class)->destroy (object);
}

static void
etsv_class_init (GtkObjectClass *object_class)
{
	ETableSubsetVariableClass *etssv_class = E_TABLE_SUBSET_VARIABLE_CLASS(object_class);

	etsv_parent_class = gtk_type_class (PARENT_TYPE);

	object_class->destroy = etsv_destroy;

	etssv_class->add = etsv_add;
}

static void
etsv_init (ETableSortedVariable *etsv)
{
	etsv->full_header = NULL;
	etsv->sort_info = NULL;

	etsv->table_model_changed_id = 0;
	etsv->table_model_row_changed_id = 0;
	etsv->table_model_cell_changed_id = 0;
	etsv->sort_info_changed_id = 0;

	etsv->sort_idle_id = 0;
}

E_MAKE_TYPE(e_table_sorted_variable, "ETableSortedVariable", ETableSortedVariable, etsv_class_init, etsv_init, PARENT_TYPE);

static gboolean
etsv_sort_idle(ETableSortedVariable *etsv)
{
	etsv_sort(etsv);
	etsv->sort_idle_id = 0;
	return FALSE;
}

static void
etsv_add       (ETableSubsetVariable *etssv,
		gint                  row)
{
	ETableModel *etm = E_TABLE_MODEL(etssv);
	ETableSubset *etss = E_TABLE_SUBSET(etssv);
	ETableSortedVariable *etsv = E_TABLE_SORTED_VARIABLE (etssv);
	
	if (etss->n_map + 1 > etssv->n_vals_allocated){
		etssv->n_vals_allocated += INCREMENT_AMOUNT;
		etss->map_table = g_realloc (etss->map_table, (etssv->n_vals_allocated) * sizeof(int));
	}
	etss->map_table[etss->n_map] = row;
	etss->n_map++;
	if (etsv->sort_idle_id == 0) {
		etsv->sort_idle_id = g_idle_add_full(50, (GSourceFunc) etsv_sort_idle, etsv, NULL);
	}
	if (!etm->frozen)
		e_table_model_changed (etm);
}

ETableModel *
e_table_sorted_variable_new (ETableModel *source, ETableHeader *full_header, ETableSortInfo *sort_info)
{
	ETableSortedVariable *etsv = gtk_type_new (E_TABLE_SORTED_VARIABLE_TYPE);
	ETableSubsetVariable *etssv = E_TABLE_SUBSET_VARIABLE (etsv);

	if (e_table_subset_variable_construct (etssv, source) == NULL){
		gtk_object_destroy (GTK_OBJECT (etsv));
		return NULL;
	}
	
	etsv->sort_info = sort_info;
	gtk_object_ref(GTK_OBJECT(etsv->sort_info));
	etsv->full_header = full_header;
	gtk_object_ref(GTK_OBJECT(etsv->full_header));

	etsv->table_model_changed_id = gtk_signal_connect (GTK_OBJECT (source), "model_changed",
							   GTK_SIGNAL_FUNC (etsv_proxy_model_changed), etsv);
	etsv->table_model_row_changed_id = gtk_signal_connect (GTK_OBJECT (source), "model_row_changed",
							       GTK_SIGNAL_FUNC (etsv_proxy_model_row_changed), etsv);
	etsv->table_model_cell_changed_id = gtk_signal_connect (GTK_OBJECT (source), "model_cell_changed",
								GTK_SIGNAL_FUNC (etsv_proxy_model_cell_changed), etsv);
	etsv->sort_info_changed_id = gtk_signal_connect (GTK_OBJECT (sort_info), "sort_info_changed",
							 GTK_SIGNAL_FUNC (etsv_sort_info_changed), etsv);
	
	return E_TABLE_MODEL(etsv);
}

static void
etsv_proxy_model_changed (ETableModel *etm, ETableSortedVariable *etsv)
{
	if (!E_TABLE_MODEL(etsv)->frozen){
		/*		FIXME: do_resort (); */
	}
}

static void
etsv_proxy_model_row_changed (ETableModel *etm, int row, ETableSortedVariable *etsv)
{
	ETableSubsetVariable *etssv = E_TABLE_SUBSET_VARIABLE(etsv);
	if (!E_TABLE_MODEL(etsv)->frozen){
		if (e_table_subset_variable_remove(etssv, row))
			e_table_subset_variable_add (etssv, row);
	}
}

static void
etsv_proxy_model_cell_changed (ETableModel *etm, int col, int row, ETableSortedVariable *etsv)
{
	ETableSubsetVariable *etssv = E_TABLE_SUBSET_VARIABLE(etsv);
	if (!E_TABLE_MODEL(etsv)->frozen){
		if (e_table_subset_variable_remove(etssv, row))
			e_table_subset_variable_add (etssv, row);
	}
}

static void
etsv_sort_info_changed (ETableSortInfo *info, ETableSortedVariable *etsv)
{
	etsv_sort(etsv);
}

static ETableSortedVariable *etsv_closure;

static int
qsort_callback(const void *data1, const void *data2)
{
	gint row1 = *(int *)data1;
	gint row2 = *(int *)data2;
	static ETableCol *last_col = NULL;
	static int last_row = -1;
	static void *val = NULL;
	ETableSubset *etss = E_TABLE_SUBSET(etsv_closure);
	int j;
	int sort_count = e_table_sort_info_sorting_get_count(etsv_closure->sort_info);
	int comp_val = 0;
	int ascending = 1;
	while(gtk_events_pending())
		gtk_main_iteration();
	for (j = 0; j < sort_count; j++) {
		ETableSortColumn column = e_table_sort_info_sorting_get_nth(etsv_closure->sort_info, j);
		ETableCol *col;
		if (column.column > e_table_header_count (etsv_closure->full_header))
			col = e_table_header_get_columns (etsv_closure->full_header)[e_table_header_count (etsv_closure->full_header) - 1];
		else
			col = e_table_header_get_columns (etsv_closure->full_header)[column.column];
		if (last_col != col || last_row != row1)
			val = e_table_model_value_at (etss->source, col->col_idx, row1);
		last_col = col;
		comp_val = (*col->compare)(val, e_table_model_value_at (etss->source, col->col_idx, row2));
		ascending = column.ascending;
		if (comp_val != 0)
			break;
	}
	if (comp_val == 0) {
		if (row1 < row2)
			comp_val = -1;
		if (row1 > row2)
			comp_val = 1;
	}
	if (!ascending)
		comp_val = -comp_val;
	return comp_val;
}


static void
etsv_sort(ETableSortedVariable *etsv)
{
	etsv_closure = etsv;
	qsort(E_TABLE_SUBSET(etsv)->map_table, E_TABLE_SUBSET(etsv)->n_map, sizeof(int), qsort_callback);
	e_table_model_changed (E_TABLE_MODEL(etsv));
}

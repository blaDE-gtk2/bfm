/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2006 Benedikt Meurer <benny@xfce.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __FMB_COLUMN_MODEL_H__
#define __FMB_COLUMN_MODEL_H__

#include <fmb/fmb-enum-types.h>

G_BEGIN_DECLS;

typedef struct _FmbColumnModelClass FmbColumnModelClass;
typedef struct _FmbColumnModel      FmbColumnModel;

#define FMB_TYPE_COLUMN_MODEL            (fmb_column_model_get_type ())
#define FMB_COLUMN_MODEL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), FMB_TYPE_COLUMN_MODEL, FmbColumnModel))
#define FMB_COLUMN_MODEL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), FMB_TYPE_COLUMN_MODEL, FmbColumnModelClass))
#define FMB_IS_COLUMN_MODEL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FMB_TYPE_COLUMN_MODEL))
#define FMB_IS_COLUMN_MODEL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), FMB_TYPE_COLUMN_MODEL))
#define FMB_COLUMN_MODEL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), FMB_TYPE_COLUMN_MODEL, FmbColumnModelClass))

/**
 * FmbColumnModelColumn:
 * @FMB_COLUMN_MODEL_COLUMN_NAME    : the name of the column.
 * @FMB_COLUMN_MODEL_COLUMN_MUTABLE : TRUE if the visibility can be changed.
 * @FMB_COLUMN_MODEL_COLUMN_VISIBLE : whether the column is visible.
 *
 * The #FmbColumnModel columns used by the #FmbColumnEditor.
 **/
typedef enum
{
  FMB_COLUMN_MODEL_COLUMN_NAME,
  FMB_COLUMN_MODEL_COLUMN_MUTABLE,
  FMB_COLUMN_MODEL_COLUMN_VISIBLE,
  FMB_COLUMN_MODEL_N_COLUMNS,
} FmbColumnModelColumn;

GType               fmb_column_model_get_type            (void) G_GNUC_CONST;

FmbColumnModel  *fmb_column_model_get_default         (void);

void                fmb_column_model_exchange            (FmbColumnModel *column_model,
                                                             GtkTreeIter       *iter1,
                                                             GtkTreeIter       *iter2);

FmbColumn        fmb_column_model_get_column_for_iter (FmbColumnModel *column_model,
                                                             GtkTreeIter       *iter);

const FmbColumn *fmb_column_model_get_column_order    (FmbColumnModel *column_model);

const gchar        *fmb_column_model_get_column_name     (FmbColumnModel *column_model,
                                                             FmbColumn       column);

gboolean            fmb_column_model_get_column_visible  (FmbColumnModel *column_model,
                                                             FmbColumn       column);
void                fmb_column_model_set_column_visible  (FmbColumnModel *column_model,
                                                             FmbColumn       column,
                                                             gboolean           visible);

gint                fmb_column_model_get_column_width    (FmbColumnModel *column_model,
                                                             FmbColumn       column);
void                fmb_column_model_set_column_width    (FmbColumnModel *column_model,
                                                             FmbColumn       column,
                                                             gint               width);

G_END_DECLS;

#endif /* !__FMB_COLUMN_MODEL_H__ */

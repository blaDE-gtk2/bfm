/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2005-2006 Benedikt Meurer <benny@xfce.org>
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

#ifndef __FMB_RENAMER_MODEL_H__
#define __FMB_RENAMER_MODEL_H__

#include <fmb/fmb-enum-types.h>
#include <fmb/fmb-file.h>

G_BEGIN_DECLS;

typedef struct _FmbRenamerModelClass FmbRenamerModelClass;
typedef struct _FmbRenamerModel      FmbRenamerModel;

#define FMB_TYPE_RENAMER_MODEL             (fmb_renamer_model_get_type ())
#define FMB_RENAMER_MODEL(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), FMB_TYPE_RENAMER_MODEL, FmbRenamerModel))
#define FMB_RENAMER_MODEL_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), FMB_TYPE_RENAMER_MODEL, FmbRenamerModelClass))
#define FMB_IS_RENAMER_MODEL(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FMB_TYPE_RENAMER_MODEL))
#define FMB_IS_RENAMER_MODEL_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), FMB_TYPE_RENAMER_MODEL))
#define FMB_RENAMER_MODEL_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), FMB_TYPE_RENAMER_MODEL, FmbRenamerModelClass))

/**
 * FmbRenamerModelColumn:
 * @FMB_RENAMER_MODEL_COLUMN_CONFLICT        : the column which tells whether there's a name conflict.
 * @FMB_RENAMER_MODEL_COLUMN_CONFLICT_WEIGHT : Use to set the text to bold in case of a conflict
 * @FMB_RENAMER_MODEL_COLUMN_FILE            : the column with the #FmbFile.
 * @FMB_RENAMER_MODEL_COLUMN_NEWNAME         : the column with the new name.
 * @FMB_RENAMER_MODEL_COLUMN_OLDNAME         : the column with the old name.
 *
 * The column ids provided by #FmbRenamerModel instances.
 **/
typedef enum
{
  FMB_RENAMER_MODEL_COLUMN_CONFLICT,
  FMB_RENAMER_MODEL_COLUMN_CONFLICT_WEIGHT,
  FMB_RENAMER_MODEL_COLUMN_FILE,
  FMB_RENAMER_MODEL_COLUMN_NEWNAME,
  FMB_RENAMER_MODEL_COLUMN_OLDNAME,
  FMB_RENAMER_MODEL_N_COLUMNS,
} FmbRenamerModelColumn;

GType                fmb_renamer_model_get_type       (void) G_GNUC_CONST;

FmbRenamerModel  *fmb_renamer_model_new            (void) G_GNUC_MALLOC;

FmbRenamerMode    fmb_renamer_model_get_mode       (FmbRenamerModel *renamer_model);

FmbxRenamer      *fmb_renamer_model_get_renamer    (FmbRenamerModel *renamer_model);
void                 fmb_renamer_model_set_renamer    (FmbRenamerModel *renamer_model,
                                                          FmbxRenamer     *renamer);

void                 fmb_renamer_model_insert         (FmbRenamerModel *renamer_model,
                                                          FmbFile         *file,
                                                          gint                position);
void                 fmb_renamer_model_reorder        (FmbRenamerModel *renamer_model,
                                                          GList              *tree_paths,
                                                          gint                position);
void                 fmb_renamer_model_sort           (FmbRenamerModel *renamer_model,
                                                          GtkSortType         sort_order);
void                 fmb_renamer_model_clear          (FmbRenamerModel *renamer_model);
void                 fmb_renamer_model_remove         (FmbRenamerModel *renamer_model,
                                                          GtkTreePath        *path);


/**
 * fmb_renamer_model_append:
 * @model : a #FmbRenamerModel.
 * @file  : a #FmbFile instance.
 *
 * Appends the @file to the @renamer_model.
 **/
#define fmb_renamer_model_append(model,file) fmb_renamer_model_insert (model, file, -1)

G_END_DECLS;

#endif /* !__FMB_RENAMER_MODEL_H__ */

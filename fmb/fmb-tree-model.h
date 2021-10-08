/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2006 Benedikt Meurer <benny@xfce.org>.
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

#ifndef __FMB_TREE_MODEL_H__
#define __FMB_TREE_MODEL_H__

#include <fmb/fmb-file.h>

G_BEGIN_DECLS;

typedef struct _FmbTreeModelClass FmbTreeModelClass;
typedef struct _FmbTreeModel      FmbTreeModel;

typedef gboolean (* FmbTreeModelVisibleFunc) (FmbTreeModel *model,
                                                 FmbFile      *file,
                                                 gpointer         data);

#define FMB_TYPE_TREE_MODEL            (fmb_tree_model_get_type ())
#define FMB_TREE_MODEL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), FMB_TYPE_TREE_MODEL, FmbTreeModel))
#define FMB_TREE_MODEL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), FMB_TYPE_TREE_MODEL, FmbTreeModelClass))
#define FMB_IS_TREE_MODEL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FMB_TYPE_TREE_MODEL))
#define FMB_IS_TREE_MODEL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), FMB_TYPE_TREE_MODEL))
#define FMB_TREE_MODEL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), FMB_TYPE_TREE_MODEL, FmbTreeModelClass))

/**
 * FmbTreeModelColumn:
 * @FMB_TREE_MODEL_COLUMN_FILE   : the index of the file column.
 * @FMB_TREE_MODEL_COLUMN_NAME   : the index of the name column.
 * @FMB_TREE_MODEL_COLUMN_ATTR   : the index of the #PangoAttrList column.
 * @FMB_TREE_MODEL_COLUMN_DEVICE : the index of the #FmbDevice column.
 *
 * Columns exported by the #FmbTreeModel using the
 * #GtkTreeModel interface.
 **/
typedef enum
{
  FMB_TREE_MODEL_COLUMN_FILE,
  FMB_TREE_MODEL_COLUMN_NAME,
  FMB_TREE_MODEL_COLUMN_ATTR,
  FMB_TREE_MODEL_COLUMN_DEVICE,
  FMB_TREE_MODEL_N_COLUMNS,
} FmbTreeModelColumn;

GType            fmb_tree_model_get_type           (void) G_GNUC_CONST;

void             fmb_tree_model_set_visible_func   (FmbTreeModel            *model,
                                                       FmbTreeModelVisibleFunc  func,
                                                       gpointer                    data);
void             fmb_tree_model_refilter           (FmbTreeModel            *model);

void             fmb_tree_model_cleanup            (FmbTreeModel            *model);
gboolean         fmb_tree_model_node_has_dummy     (FmbTreeModel            *model,
                                                       GNode                      *node);
void             fmb_tree_model_add_child          (FmbTreeModel            *model,
                                                       GNode                      *node,
                                                       FmbFile                 *file);

G_END_DECLS;

#endif /* !__FMB_TREE_MODEL_H__ */

/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2005-2006 Benedikt Meurer <benny@xfce.org>
 * Copyright (c) 2009 Jannis Pohlmann <jannis@xfce.org>
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

#ifndef __FMB_CHOOSER_MODEL_H__
#define __FMB_CHOOSER_MODEL_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS;

typedef struct _FmbChooserModelClass FmbChooserModelClass;
typedef struct _FmbChooserModel      FmbChooserModel;

#define FMB_TYPE_CHOOSER_MODEL            (fmb_chooser_model_get_type ())
#define FMB_CHOOSER_MODEL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), FMB_TYPE_CHOOSER_MODEL, FmbChooserModel))
#define FMB_CHOOSER_MODEL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), FMB_TYPE_CHOOSER_MODEL, FmbChooserModelClass))
#define FMB_IS_CHOOSER_MODEL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FMB_TYPE_CHOOSER_MODEL))
#define FMB_IS_CHOOSER_MODEL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), FMB_TYPE_CHOOSER_MODEL))
#define FMB_CHOOSER_MODEL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), FMB_TYPE_CHOOSER_MODEL, FmbChooserModelClass))

/**
 * FmbChooserModelColumn:
 * @FMB_CHOOSER_MODEL_COLUMN_NAME        : the name of the application.
 * @FMB_CHOOSER_MODEL_COLUMN_ICON        : the name or absolute path of the application's icon.
 * @FMB_CHOOSER_MODEL_COLUMN_APPLICATION : the #GAppInfo object.
 * @FMB_CHOOSER_MODEL_COLUMN_STYLE       : custom font style.
 * @FMB_CHOOSER_MODEL_N_COLUMNS          : the number of columns in #FmbChooserModel.
 *
 * The identifiers for the columns provided by the #FmbChooserModel.
 **/
typedef enum
{
  FMB_CHOOSER_MODEL_COLUMN_NAME,
  FMB_CHOOSER_MODEL_COLUMN_ICON,
  FMB_CHOOSER_MODEL_COLUMN_APPLICATION,
  FMB_CHOOSER_MODEL_COLUMN_STYLE,
  FMB_CHOOSER_MODEL_COLUMN_WEIGHT,
  FMB_CHOOSER_MODEL_N_COLUMNS,
} FmbChooserModelColumn;

GType               fmb_chooser_model_get_type         (void) G_GNUC_CONST;

FmbChooserModel *fmb_chooser_model_new              (const gchar        *content_type) G_GNUC_MALLOC;
const gchar        *fmb_chooser_model_get_content_type (FmbChooserModel *model);
gboolean            fmb_chooser_model_remove           (FmbChooserModel *model,
                                                           GtkTreeIter        *iter,
                                                           GError            **error);

G_END_DECLS;

#endif /* !__FMB_CHOOSER_MODEL_H__ */

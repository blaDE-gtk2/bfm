/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2004-2007 Benedikt Meurer <benny@xfce.org>
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

#ifndef __FMB_LIST_MODEL_H__
#define __FMB_LIST_MODEL_H__

#include <fmb/fmb-folder.h>

G_BEGIN_DECLS;

typedef struct _FmbListModelClass FmbListModelClass;
typedef struct _FmbListModel      FmbListModel;

#define FMB_TYPE_LIST_MODEL            (fmb_list_model_get_type ())
#define FMB_LIST_MODEL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), FMB_TYPE_LIST_MODEL, FmbListModel))
#define FMB_LIST_MODEL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), FMB_TYPE_LIST_MODEL, FmbListModelClass))
#define FMB_IS_LIST_MODEL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FMB_TYPE_LIST_MODEL))
#define FMB_IS_LIST_MODEL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), FMB_TYPE_LIST_MODEL))
#define FMB_LIST_MODEL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), FMB_TYPE_LIST_MODEL, FmbListModelClass))

GType            fmb_list_model_get_type               (void) G_GNUC_CONST;

FmbListModel *fmb_list_model_new                    (void);

FmbFolder    *fmb_list_model_get_folder             (FmbListModel  *store);
void             fmb_list_model_set_folder             (FmbListModel  *store,
                                                           FmbFolder     *folder);

void             fmb_list_model_set_folders_first      (FmbListModel  *store,
                                                           gboolean          folders_first);

gboolean         fmb_list_model_get_show_hidden        (FmbListModel  *store);
void             fmb_list_model_set_show_hidden        (FmbListModel  *store,
                                                           gboolean          show_hidden);

gboolean         fmb_list_model_get_file_size_binary   (FmbListModel  *store);
void             fmb_list_model_set_file_size_binary   (FmbListModel  *store,
                                                           gboolean          file_size_binary);

FmbFile      *fmb_list_model_get_file               (FmbListModel  *store,
                                                           GtkTreeIter      *iter);


GList           *fmb_list_model_get_paths_for_files    (FmbListModel  *store,
                                                           GList            *files);
GList           *fmb_list_model_get_paths_for_pattern  (FmbListModel  *store,
                                                           const gchar      *pattern);

gchar           *fmb_list_model_get_statusbar_text     (FmbListModel  *store,
                                                           GList            *selected_items);

G_END_DECLS;

#endif /* !__FMB_LIST_MODEL_H__ */

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

#ifndef __FMB_FOLDER_H__
#define __FMB_FOLDER_H__

#include <fmb/fmb-file.h>

G_BEGIN_DECLS;

typedef struct _FmbFolderClass FmbFolderClass;
typedef struct _FmbFolder      FmbFolder;

#define FMB_TYPE_FOLDER            (fmb_folder_get_type ())
#define FMB_FOLDER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), FMB_TYPE_FOLDER, FmbFolder))
#define FMB_FOLDER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), FMB_TYPE_FOLDER, FmbFolderClass))
#define FMB_IS_FOLDER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FMB_TYPE_FOLDER))
#define FMB_IS_FOLDER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), FMB_TYPE_FOLDER))
#define FMB_FOLDER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), FMB_TYPE_FOLDER, FmbFolderClass))

GType         fmb_folder_get_type               (void) G_GNUC_CONST;

FmbFolder *fmb_folder_get_for_file           (FmbFile         *file);

FmbFile   *fmb_folder_get_corresponding_file (const FmbFolder *folder);
GList        *fmb_folder_get_files              (const FmbFolder *folder);
gboolean      fmb_folder_get_loading            (const FmbFolder *folder);

void          fmb_folder_reload                 (FmbFolder       *folder,
                                                    gboolean            reload_info);

G_END_DECLS;

#endif /* !__FMB_FOLDER_H__ */

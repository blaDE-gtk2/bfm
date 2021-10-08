/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2005 Benedikt Meurer <benny@xfce.org>
 * Copyright (c) 2010 Benedikt Meurer <benny@xfce.org>
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of 
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public 
 * License along with this program; if not, write to the Free 
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef __FMB_PATH_ENTRY_H__
#define __FMB_PATH_ENTRY_H__

#include <fmb/fmb-file.h>

G_BEGIN_DECLS;

typedef struct _FmbPathEntryClass FmbPathEntryClass;
typedef struct _FmbPathEntry      FmbPathEntry;

#define FMB_TYPE_PATH_ENTRY            (fmb_path_entry_get_type ())
#define FMB_PATH_ENTRY(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), FMB_TYPE_PATH_ENTRY, FmbPathEntry))
#define FMB_PATH_ENTRY_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), FMB_TYPE_PATH_ENTRY, FmbPathEntryClass))
#define FMB_IS_PATH_ENTRY(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FMB_TYPE_PATH_ENTRY))
#define FMB_IS_PATH_ENTRY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), FMB_TYPE_PATH_ENTRY))
#define FMB_PATH_ENTRY_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), FMB_TYPE_PATH_ENTRY, FmbPathEntryClass))

GType       fmb_path_entry_get_type              (void) G_GNUC_CONST;

GtkWidget  *fmb_path_entry_new                   (void);

FmbFile *fmb_path_entry_get_current_file      (FmbPathEntry *path_entry);
void        fmb_path_entry_set_current_file      (FmbPathEntry *path_entry,
                                                     FmbFile      *current_file);
void        fmb_path_entry_set_working_directory (FmbPathEntry *path_entry,
                                                     FmbFile      *directory);

G_END_DECLS;

#endif /* !__FMB_PATH_ENTRY_H__ */

/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2005-2006 Benedikt Meurer <benny@xfce.org>
 * Copyright (c) 2005      Jeff Franks <jcfranks@xfce.org>
 * Copyright (c) 2009-2011 Jannis Pohlmann <jannis@xfce.org>
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

#ifndef __FMB_APPLICATION_H__
#define __FMB_APPLICATION_H__

#include <fmb/fmb-window.h>
#include <fmb/fmb-thumbnail-cache.h>

G_BEGIN_DECLS;

typedef struct _FmbApplicationClass FmbApplicationClass;
typedef struct _FmbApplication      FmbApplication;

#define FMB_TYPE_APPLICATION             (fmb_application_get_type ())
#define FMB_APPLICATION(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), FMB_TYPE_APPLICATION, FmbApplication))
#define FMB_APPLICATION_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), FMB_TYPE_APPLICATION, FmbApplicationClass))
#define FMB_IS_APPLICATION(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FMB_TYPE_APPLICATION))
#define FMB_IS_APPLICATION_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), FMB_TYPE_APPLICATION))
#define FMB_APPLICATION_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), FMB_TYPE_APPLICATION, FmbApplicationClass))

GType                 fmb_application_get_type                   (void) G_GNUC_CONST;

FmbApplication    *fmb_application_get                        (void);

gboolean              fmb_application_get_daemon                 (FmbApplication *application);
void                  fmb_application_set_daemon                 (FmbApplication *application,
                                                                     gboolean           daemon);

GList                *fmb_application_get_windows                (FmbApplication *application);

gboolean              fmb_application_has_windows                (FmbApplication *application);

void                  fmb_application_take_window                (FmbApplication *application,
                                                                     GtkWindow         *window);

GtkWidget            *fmb_application_open_window                (FmbApplication *application,
                                                                     FmbFile        *directory,
                                                                     GdkScreen         *screen,
                                                                     const gchar       *startup_id);

gboolean              fmb_application_bulk_rename                (FmbApplication *application,
                                                                     const gchar       *working_directory,
                                                                     gchar            **filenames,
                                                                     gboolean           standalone,
                                                                     GdkScreen         *screen,
                                                                     const gchar       *startup_id,
                                                                     GError           **error);

gboolean              fmb_application_process_filenames          (FmbApplication *application,
                                                                     const gchar       *working_directory,
                                                                     gchar            **filenames,
                                                                     GdkScreen         *screen,
                                                                     const gchar       *startup_id,
                                                                     GError           **error);

gboolean              fmb_application_is_processing              (FmbApplication *application);

void                  fmb_application_rename_file                (FmbApplication *application,
                                                                     FmbFile        *file,
                                                                     GdkScreen         *screen,
                                                                     const gchar       *startup_id);
void                  fmb_application_create_file                (FmbApplication *application,
                                                                     FmbFile        *parent_directory,
                                                                     const gchar       *content_type,
                                                                     GdkScreen         *screen,
                                                                     const gchar       *startup_id);
void                  fmb_application_create_file_from_template (FmbApplication *application,
                                                                    FmbFile        *parent_directory,
                                                                    FmbFile        *template_file,
                                                                    GdkScreen         *screen,
                                                                    const gchar       *startup_id);
void                  fmb_application_copy_to                   (FmbApplication *application,
                                                                    gpointer           parent,
                                                                    GList             *source_file_list,
                                                                    GList             *target_file_list,
                                                                    GClosure          *new_files_closure);

void                  fmb_application_copy_into                 (FmbApplication *application,
                                                                    gpointer           parent,
                                                                    GList             *source_file_list,
                                                                    GFile             *target_file,
                                                                    GClosure          *new_files_closure);

void                  fmb_application_link_into                 (FmbApplication *application,
                                                                    gpointer           parent,
                                                                    GList             *source_file_list,
                                                                    GFile             *target_file,
                                                                    GClosure          *new_files_closure);

void                  fmb_application_move_into                 (FmbApplication *application,
                                                                    gpointer           parent,
                                                                    GList             *source_file_list,
                                                                    GFile             *target_file,
                                                                    GClosure          *new_files_closure);

void                  fmb_application_unlink_files              (FmbApplication *application,
                                                                    gpointer           parent,
                                                                    GList             *file_list,
                                                                    gboolean           permanently);

void                  fmb_application_trash                     (FmbApplication *application,
                                                                    gpointer           parent,
                                                                    GList             *file_list);

void                  fmb_application_creat                     (FmbApplication *application,
                                                                    gpointer           parent,
                                                                    GList             *file_list,
                                                                    GFile             *template_file,
                                                                    GClosure          *new_files_closure);

void                  fmb_application_mkdir                     (FmbApplication *application,
                                                                    gpointer           parent,
                                                                    GList             *file_list,
                                                                    GClosure          *new_files_closure);

void                  fmb_application_empty_trash               (FmbApplication *application,
                                                                    gpointer           parent,
                                                                    const gchar       *startup_id);

void                  fmb_application_restore_files             (FmbApplication *application,
                                                                    gpointer           parent,
                                                                    GList             *trash_file_list,
                                                                    GClosure          *new_files_closure);

FmbThumbnailCache *fmb_application_get_thumbnail_cache       (FmbApplication *application);

G_END_DECLS;

#endif /* !__FMB_APPLICATION_H__ */

/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2009 Jannis Pohlmann <jannis@xfce.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef __FMB_IO_JOBS_H__
#define __FMB_IO_JOBS_H__

#include <fmb/fmb-job.h>
#include <fmb/fmb-enum-types.h>

G_BEGIN_DECLS

FmbJob *fmb_io_jobs_create_files     (GList         *file_list,
                                            GFile         *template_file) G_GNUC_MALLOC G_GNUC_WARN_UNUSED_RESULT;
FmbJob *fmb_io_jobs_make_directories (GList         *file_list) G_GNUC_MALLOC G_GNUC_WARN_UNUSED_RESULT;
FmbJob *fmb_io_jobs_unlink_files     (GList         *file_list) G_GNUC_MALLOC G_GNUC_WARN_UNUSED_RESULT;
FmbJob *fmb_io_jobs_move_files       (GList         *source_file_list,
                                            GList         *target_file_list) G_GNUC_MALLOC G_GNUC_WARN_UNUSED_RESULT;
FmbJob *fmb_io_jobs_copy_files       (GList         *source_file_list,
                                            GList         *target_file_list) G_GNUC_MALLOC G_GNUC_WARN_UNUSED_RESULT;
FmbJob *fmb_io_jobs_link_files       (GList         *source_file_list,
                                            GList         *target_file_list) G_GNUC_MALLOC G_GNUC_WARN_UNUSED_RESULT;
FmbJob *fmb_io_jobs_trash_files      (GList         *file_list) G_GNUC_MALLOC G_GNUC_WARN_UNUSED_RESULT;
FmbJob *fmb_io_jobs_restore_files    (GList         *source_file_list,
                                            GList         *target_file_list) G_GNUC_MALLOC G_GNUC_WARN_UNUSED_RESULT;
FmbJob *fmb_io_jobs_change_group     (GList         *files,
                                            guint32        gid,
                                            gboolean       recursive) G_GNUC_MALLOC G_GNUC_WARN_UNUSED_RESULT;
FmbJob *fmb_io_jobs_change_mode      (GList         *files,
                                            FmbFileMode dir_mask,
                                            FmbFileMode dir_mode,
                                            FmbFileMode file_mask,
                                            FmbFileMode file_mode,
                                            gboolean       recursive) G_GNUC_MALLOC G_GNUC_WARN_UNUSED_RESULT;
FmbJob *fmb_io_jobs_list_directory   (GFile         *directory) G_GNUC_MALLOC G_GNUC_WARN_UNUSED_RESULT;
FmbJob *fmb_io_jobs_rename_file      (FmbFile    *file,
                                            const gchar   *display_name) G_GNUC_MALLOC G_GNUC_WARN_UNUSED_RESULT;

G_END_DECLS

#endif /* !__FMB_IO_JOBS_H__ */

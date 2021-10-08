/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2009 Jannis Pohlmann <jannis@xfce.org>
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

#ifndef __FMB_PROGRESS_DIALOG_H__
#define __FMB_PROGRESS_DIALOG_H__

#include <gtk/gtk.h>
#include <fmb/fmb-job.h>

G_BEGIN_DECLS;

typedef struct _FmbProgressDialogClass FmbProgressDialogClass;
typedef struct _FmbProgressDialog      FmbProgressDialog;

#define FMB_TYPE_PROGRESS_DIALOG            (fmb_progress_dialog_get_type ())
#define FMB_PROGRESS_DIALOG(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), FMB_TYPE_PROGRESS_DIALOG, FmbProgressDialog))
#define FMB_PROGRESS_DIALOG_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), FMB_TYPE_PROGRESS_DIALOG, FmbProgressDialogClass))
#define FMB_IS_PROGRESS_DIALOG(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FMB_TYPE_PROGRESS_DIALOG))
#define FMB_IS_PROGRESS_DIALOG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), FMB_TYPE_PROGRESS_DIALOG))
#define FMB_PROGRESS_DIALOG_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), FMB_TYPE_PROGRESS_DIALOG, FmbProgressDialogClass))

GType     fmb_progress_dialog_get_type  (void) G_GNUC_CONST;

GtkWidget *fmb_progress_dialog_new      (void);
void       fmb_progress_dialog_add_job  (FmbProgressDialog *dialog,
                                            FmbJob            *job,
                                            const gchar          *icon_name,
                                            const gchar          *title);
gboolean   fmb_progress_dialog_has_jobs (FmbProgressDialog *dialog);

G_END_DECLS;

#endif /* !__FMB_PROGRESS_DIALOG_H__ */

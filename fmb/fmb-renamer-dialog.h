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

#ifndef __FMB_RENAMER_DIALOG_H__
#define __FMB_RENAMER_DIALOG_H__

#include <fmb/fmb-file.h>

G_BEGIN_DECLS;

typedef struct _FmbRenamerDialogClass FmbRenamerDialogClass;
typedef struct _FmbRenamerDialog      FmbRenamerDialog;

#define FMB_TYPE_RENAMER_DIALOG            (fmb_renamer_dialog_get_type ())
#define FMB_RENAMER_DIALOG(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), FMB_TYPE_RENAMER_DIALOG, FmbRenamerDialog))
#define FMB_RENAMER_DIALOG_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), FMB_TYPE_RENAMER_DIALOG, FmbRenamerDialogClass))
#define FMB_IS_RENAMER_DIALOG(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FMB_TYPE_RENAMER_DIALOG))
#define FMB_IS_RENAMER_DIALOG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), FMB_TYPE_RENAMER_DIALOG))
#define FMB_RENAMER_DIALOG_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), FMB_TYPE_RENAMER_DIALOG, FmbRenamerDialogClass))

GType       fmb_renamer_dialog_get_type              (void) G_GNUC_CONST;

void        fmb_show_renamer_dialog                  (gpointer             parent,
                                                         FmbFile          *current_directory,
                                                         GList               *files,
                                                         gboolean             standalone,
                                                         const gchar         *startup_id);

G_END_DECLS;

#endif /* !__FMB_RENAMER_DIALOG_H__ */

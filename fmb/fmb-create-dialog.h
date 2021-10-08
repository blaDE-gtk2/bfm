/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2005-2006 Benedikt Meurer <benny@xfce.org>
 * Copyright (c) 2009-2010 Jannis Pohlmann <jannis@xfce.org>
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

#ifndef __FMB_CREATE_DIALOG_H__
#define __FMB_CREATE_DIALOG_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS;

typedef struct _FmbCreateDialogClass FmbCreateDialogClass;
typedef struct _FmbCreateDialog      FmbCreateDialog;

#define FMB_TYPE_CREATE_DIALOG             (fmb_create_dialog_get_type ())
#define FMB_CREATE_DIALOG(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), FMB_TYPE_CREATE_DIALOG, FmbCreateDialog))
#define FMB_CREATE_DIALOG_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), FMB_TYPE_CREATE_DIALOG, FmbCreateDialogClass))
#define FMB_IS_CREATE_DIALOG(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FMB_TYPE_CREATE_DIALOG))
#define FMB_IS_CREATE_DIALOG_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), FMB_TYPE_CREATE_DIALOG))
#define FMB_CREATE_DIALOG_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), FMB_TYPE_CREATE_DIALOG, FmbCreateDialogClass))

GType  fmb_create_dialog_get_type (void) G_GNUC_CONST;

gchar *fmb_show_create_dialog     (gpointer     parent,
                                      const gchar *content_type,
                                      const gchar *filename,
                                      const gchar *title) G_GNUC_MALLOC;

G_END_DECLS;

#endif /* !__FMB_CREATE_DIALOG_H__ */

/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2005-2006 Benedikt Meurer <benny@xfce.org>
 * Copyright (c) 2010 Jannis Pohlmann <jannis@xfce.org>
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


#ifndef __FMB_LOCATION_DIALOG_H__
#define __FMB_LOCATION_DIALOG_H__

#include <fmb/fmb-abstract-dialog.h>
#include <fmb/fmb-file.h>

G_BEGIN_DECLS;

typedef struct _FmbLocationDialogClass FmbLocationDialogClass;
typedef struct _FmbLocationDialog      FmbLocationDialog;

#define FMB_TYPE_LOCATION_DIALOG             (fmb_location_dialog_get_type ())
#define FMB_LOCATION_DIALOG(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), FMB_TYPE_LOCATION_DIALOG, FmbLocationDialog))
#define FMB_LOCATION_DIALOG_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), FMB_TYPE_LOCATION_DIALOG, FmbLocationDialogClass))
#define FMB_IS_LOCATION_DIALOG(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FMB_TYPE_LOCATION_DIALOG))
#define FMB_IS_LOCATION_DIALOG_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), FMB_TYPE_LOCATION_DIALOG))
#define FMB_LOCATION_DIALOG_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), FMB_TYPE_LOCATION_DIALOG, FmbLocationDialogClass))

struct _FmbLocationDialogClass
{
  FmbAbstractDialogClass __parent__;
};

struct _FmbLocationDialog
{
  FmbAbstractDialog __parent__;
  GtkWidget           *entry;
};

GType       fmb_location_dialog_get_type              (void) G_GNUC_CONST;

GtkWidget  *fmb_location_dialog_new                   (void) G_GNUC_MALLOC;

FmbFile *fmb_location_dialog_get_selected_file     (FmbLocationDialog *location_dialog);
void        fmb_location_dialog_set_selected_file     (FmbLocationDialog *location_dialog,
                                                          FmbFile           *selected_file);
void        fmb_location_dialog_set_working_directory (FmbLocationDialog *location_dialog,
                                                          FmbFile           *directory);

G_END_DECLS;

#endif /* !__FMB_LOCATION_DIALOG_H__ */

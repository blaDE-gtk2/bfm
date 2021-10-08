/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2006 Benedikt Meurer <benny@xfce.org>
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

#ifndef __FMB_ABSTRACT_DIALOG_H__
#define __FMB_ABSTRACT_DIALOG_H__

#include <blxo/blxo.h>

G_BEGIN_DECLS;

typedef struct _FmbAbstractDialogClass FmbAbstractDialogClass;
typedef struct _FmbAbstractDialog      FmbAbstractDialog;

#define FMB_TYPE_ABSTRACT_DIALOG             (fmb_abstract_dialog_get_type ())
#define FMB_ABSTRACT_DIALOG(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), FMB_TYPE_ABSTRACT_DIALOG, FmbAbstractDialog))
#define FMB_ABSTRACT_DIALOG_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), FMB_TYPE_ABSTRACT_DIALOG, FmbAbstractDialogClass))
#define FMB_IS_ABSTRACT_DIALOG(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FMB_TYPE_ABSTRACT_DIALOG))
#define FMB_IS_ABSTRACT_DIALOG_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), FMB_TYPE_ABSTRACT_DIALOG))
#define FMB_ABSTRACT_DIALOG_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), FMB_TYPE_ABSTRACT_DIALOG, FmbAbstractDialog))

struct _FmbAbstractDialogClass
{
  GtkDialogClass __parent__;
};

struct _FmbAbstractDialog
{
  GtkDialog __parent__;
};

GType fmb_abstract_dialog_get_type (void) G_GNUC_CONST;

G_END_DECLS;

#endif /* !__FMB_ABSTRACT_DIALOG_H__ */

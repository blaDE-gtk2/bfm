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

#ifndef __FMB_TEMPLATES_ACTION_H__
#define __FMB_TEMPLATES_ACTION_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS;

typedef struct _FmbTemplatesActionClass FmbTemplatesActionClass;
typedef struct _FmbTemplatesAction      FmbTemplatesAction;

#define FMB_TYPE_TEMPLATES_ACTION            (fmb_templates_action_get_type ())
#define FMB_TEMPLATES_ACTION(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), FMB_TYPE_TEMPLATES_ACTION, FmbTemplatesAction))
#define FMB_TEMPLATES_ACTION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), FMB_TYPE_TEMPLATES_ACTION, FmbTemplatesActionClass))
#define FMB_IS_TEMPLATES_ACTION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FMB_TYPE_TEMPLATES_ACTION))
#define FMB_IS_TEMPLATES_ACTION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), FMB_TYPE_TEMPLATES_ACTION))
#define FMB_TEMPLATES_ACTION_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), FMB_TYPE_TEMPLATES_ACTION, FmbTemplatesActionClass))

GType      fmb_templates_action_get_type (void) G_GNUC_CONST;

GtkAction *fmb_templates_action_new      (const gchar *name,
                                             const gchar *label) G_GNUC_MALLOC;

G_END_DECLS;

#endif /* !__FMB_TEMPLATES_ACTION_H__ */

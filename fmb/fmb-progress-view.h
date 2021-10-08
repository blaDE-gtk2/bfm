/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2005 Benedikt Meurer <benny@xfce.org>
 * Copyright (c) 2009 Jannis Pohlmann <jannis@xfce.org>
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

#ifndef __FMB_PROGRESS_VIEW_H__
#define __FMB_PROGRESS_VIEW_H__

#include <gtk/gtk.h>

#include <fmb/fmb-job.h>

G_BEGIN_DECLS;

typedef struct _FmbProgressViewClass FmbProgressViewClass;
typedef struct _FmbProgressView      FmbProgressView;

#define FMB_TYPE_PROGRESS_VIEW            (fmb_progress_view_get_type ())
#define FMB_PROGRESS_VIEW(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), FMB_TYPE_PROGRESS_VIEW, FmbProgressView))
#define FMB_PROGRESS_VIEW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), FMB_TYPE_PROGRESS_VIEW, FmbProgressViewClass))
#define FMB_IS_PROGRESS_VIEW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FMB_TYPE_PROGRESS_VIEW))
#define FMB_IS_PROGRESS_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), FMB_TYPE_PROGRESS_VIEW))
#define FMB_PROGRESS_VIEW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), FMB_TYPE_PROGRESS_VIEW, FmbProgressViewClass))

GType      fmb_progress_view_get_type      (void) G_GNUC_CONST;

GtkWidget *fmb_progress_view_new_with_job  (FmbJob          *job) G_GNUC_MALLOC;

void       fmb_progress_view_set_icon_name (FmbProgressView *view,
                                               const gchar        *icon_name);
void       fmb_progress_view_set_title     (FmbProgressView *view,
                                               const gchar        *title);

G_END_DECLS;

#endif /* !__FMB_PROGRESS_VIEW_H__ */

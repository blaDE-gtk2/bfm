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

#ifndef __FMB_WINDOW_H__
#define __FMB_WINDOW_H__

#include <fmb/fmb-enum-types.h>
#include <fmb/fmb-folder.h>

G_BEGIN_DECLS;

typedef struct _FmbWindowClass FmbWindowClass;
typedef struct _FmbWindow      FmbWindow;

#define FMB_TYPE_WINDOW            (fmb_window_get_type ())
#define FMB_WINDOW(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), FMB_TYPE_WINDOW, FmbWindow))
#define FMB_WINDOW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), FMB_TYPE_WINDOW, FmbWindowClass))
#define FMB_IS_WINDOW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FMB_TYPE_WINDOW))
#define FMB_IS_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), FMB_TYPE_WINDOW))
#define FMB_WINDOW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), FMB_TYPE_WINDOW, FmbWindowClass))

GType           fmb_window_get_type              (void) G_GNUC_CONST;

FmbFile     *fmb_window_get_current_directory (FmbWindow   *window);
void            fmb_window_set_current_directory (FmbWindow   *window,
                                                     FmbFile     *current_directory);

void            fmb_window_scroll_to_file        (FmbWindow   *window,
                                                     FmbFile     *file,
                                                     gboolean        select,
                                                     gboolean        use_align,
                                                     gfloat          row_align,
                                                     gfloat          col_align);

gchar         **fmb_window_get_directories       (FmbWindow   *window,
                                                     gint           *active_page);
gboolean        fmb_window_set_directories       (FmbWindow   *window,
                                                     gchar         **uris,
                                                     gint            active_page);
void            fmb_window_update_directories    (FmbWindow *window,
                                                     FmbFile   *old_directory,
                                                     FmbFile   *new_directory);

G_END_DECLS;

#endif /* !__FMB_WINDOW_H__ */

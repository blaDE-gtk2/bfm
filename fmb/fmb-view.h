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

#ifndef __FMB_VIEW_H__
#define __FMB_VIEW_H__

#include <fmb/fmb-component.h>
#include <fmb/fmb-enum-types.h>
#include <fmb/fmb-navigator.h>

G_BEGIN_DECLS;

typedef struct _FmbViewIface FmbViewIface;
typedef struct _FmbView      FmbView;

#define FMB_TYPE_VIEW            (fmb_view_get_type ())
#define FMB_VIEW(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), FMB_TYPE_VIEW, FmbView))
#define FMB_IS_VIEW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FMB_TYPE_VIEW))
#define FMB_VIEW_GET_IFACE(obj)  (G_TYPE_INSTANCE_GET_INTERFACE ((obj), FMB_TYPE_VIEW, FmbViewIface))

struct _FmbViewIface
{
  GTypeInterface __parent__;

  /* virtual methods */
  gboolean        (*get_loading)        (FmbView     *view);
  const gchar    *(*get_statusbar_text) (FmbView     *view);

  gboolean        (*get_show_hidden)    (FmbView     *view);
  void            (*set_show_hidden)    (FmbView     *view,
                                         gboolean        show_hidden);

  FmbZoomLevel (*get_zoom_level)     (FmbView     *view);
  void            (*set_zoom_level)     (FmbView     *view,
                                         FmbZoomLevel zoom_level);
  void            (*reset_zoom_level)   (FmbView     *view);

  void            (*reload)             (FmbView     *view,
                                         gboolean        reload_info);

  gboolean        (*get_visible_range)  (FmbView     *view,
                                         FmbFile    **start_file,
                                         FmbFile    **end_file);

  void            (*scroll_to_file)     (FmbView     *view,
                                         FmbFile     *file,
                                         gboolean        select,
                                         gboolean        use_align,
                                         gfloat          row_align,
                                         gfloat          col_align);
};

GType           fmb_view_get_type            (void) G_GNUC_CONST;

gboolean        fmb_view_get_loading         (FmbView     *view);
const gchar    *fmb_view_get_statusbar_text  (FmbView     *view);

gboolean        fmb_view_get_show_hidden     (FmbView     *view);
void            fmb_view_set_show_hidden     (FmbView     *view,
                                                 gboolean        show_hidden);

FmbZoomLevel fmb_view_get_zoom_level      (FmbView     *view);
void            fmb_view_set_zoom_level      (FmbView     *view,
                                                 FmbZoomLevel zoom_level);
void            fmb_view_reset_zoom_level    (FmbView     *view);

void            fmb_view_reload              (FmbView     *view,
                                                 gboolean        reload_info);

gboolean        fmb_view_get_visible_range   (FmbView     *view,
                                                 FmbFile    **start_file,
                                                 FmbFile    **end_file);

void            fmb_view_scroll_to_file      (FmbView     *view,
                                                 FmbFile     *file,
                                                 gboolean        select_file,
                                                 gboolean        use_align,
                                                 gfloat          row_align,
                                                 gfloat          col_align);

G_END_DECLS;

#endif /* !__FMB_VIEW_H__ */

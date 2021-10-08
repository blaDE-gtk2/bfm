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

#ifndef __FMB_SIDE_PANE_H__
#define __FMB_SIDE_PANE_H__

#include <fmb/fmb-component.h>

G_BEGIN_DECLS;

typedef struct _FmbSidePaneIface FmbSidePaneIface;
typedef struct _FmbSidePane      FmbSidePane;

#define FMB_TYPE_SIDE_PANE           (fmb_side_pane_get_type ())
#define FMB_SIDE_PANE(obj)           (G_TYPE_CHECK_INSTANCE_CAST ((obj), FMB_TYPE_SIDE_PANE, FmbSidePane))
#define FMB_IS_SIDE_PANE(obj)        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FMB_TYPE_SIDE_PANE))
#define FMB_SIDE_PANE_GET_IFACE(obj) (G_TYPE_INSTANCE_GET_INTERFACE ((obj), FMB_TYPE_SIDE_PANE, FmbSidePaneIface))

struct _FmbSidePaneIface
{
  GTypeInterface __parent__;

  /* virtual methods */
  gboolean (*get_show_hidden) (FmbSidePane *side_pane);
  void     (*set_show_hidden) (FmbSidePane *side_pane,
                               gboolean        show_hidden);
};

GType    fmb_side_pane_get_type        (void) G_GNUC_CONST;

gboolean fmb_side_pane_get_show_hidden (FmbSidePane *side_pane);
void     fmb_side_pane_set_show_hidden (FmbSidePane *side_pane,
                                           gboolean        show_hidden);

G_END_DECLS;

#endif /* !__FMB_SIDE_PANE_H__ */

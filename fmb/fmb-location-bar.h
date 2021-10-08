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

#ifndef __FMB_LOCATION_BAR_H__
#define __FMB_LOCATION_BAR_H__

#include <fmb/fmb-component.h>

G_BEGIN_DECLS;

typedef struct _FmbLocationBarIface FmbLocationBarIface;
typedef struct _FmbLocationBar      FmbLocationBar;

#define FMB_TYPE_LOCATION_BAR            (fmb_location_bar_get_type ())
#define FMB_LOCATION_BAR(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), FMB_TYPE_LOCATION_BAR, FmbLocationBar))
#define FMB_IS_LOCATION_BAR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FMB_TYPE_LOCATION_BAR))
#define FMB_LOCATION_BAR_GET_IFACE(obj)  (G_TYPE_INSTANCE_GET_INTERFACE ((obj), FMB_TYPE_LOCATION_BAR, FmbLocationBarIface))

struct _FmbLocationBarIface
{
  GTypeInterface __parent__;

  /* virtual methods */
  gboolean (*accept_focus)  (FmbLocationBar *location_bar,
                             const gchar       *initial_text);
  gboolean (*is_standalone) (FmbLocationBar *location_bar);
};

GType    fmb_location_bar_get_type     (void) G_GNUC_CONST;

gboolean fmb_location_bar_accept_focus  (FmbLocationBar *location_bar,
                                            const gchar       *initial_text);
gboolean fmb_location_bar_is_standalone (FmbLocationBar *location_bar);

G_END_DECLS;

#endif /* !__FMB_LOCATION_BAR_H__ */

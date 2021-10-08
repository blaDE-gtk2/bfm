/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2005 Benedikt Meurer <benny@xfce.org>
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

#ifndef __FMB_NAVIGATOR_H__
#define __FMB_NAVIGATOR_H__

#include <fmb/fmb-file.h>

G_BEGIN_DECLS;

typedef struct _FmbNavigatorIface FmbNavigatorIface;
typedef struct _FmbNavigator      FmbNavigator;

#define FMB_TYPE_NAVIGATOR           (fmb_navigator_get_type ())
#define FMB_NAVIGATOR(obj)           (G_TYPE_CHECK_INSTANCE_CAST ((obj), FMB_TYPE_NAVIGATOR, FmbNavigator))
#define FMB_IS_NAVIGATOR(obj)        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FMB_TYPE_NAVIGATOR))
#define FMB_NAVIGATOR_GET_IFACE(obj) (G_TYPE_INSTANCE_GET_INTERFACE ((obj), FMB_TYPE_NAVIGATOR, FmbNavigatorIface))

struct _FmbNavigatorIface
{
  GTypeInterface __parent__;

  /* methods */
  FmbFile *(*get_current_directory) (FmbNavigator *navigator);
  void        (*set_current_directory) (FmbNavigator *navigator,
                                        FmbFile      *current_directory);

  /* signals */
  void        (*change_directory)      (FmbNavigator *navigator,
                                        FmbFile      *directory);
  void        (*open_new_tab)          (FmbNavigator *navigator,
                                        FmbFile      *directory);
};

GType       fmb_navigator_get_type              (void) G_GNUC_CONST;

FmbFile *fmb_navigator_get_current_directory (FmbNavigator *navigator);
void        fmb_navigator_set_current_directory (FmbNavigator *navigator,
                                                    FmbFile      *current_directory);

void        fmb_navigator_change_directory      (FmbNavigator *navigator,
                                                    FmbFile      *directory);

void        fmb_navigator_open_new_tab          (FmbNavigator *navigator,
                                                    FmbFile      *directory);

G_END_DECLS;

#endif /* !__FMB_NAVIGATOR_H__ */

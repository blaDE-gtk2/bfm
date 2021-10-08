/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2009 Jannis Pohlmann <jannis@xfce.org>
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

#ifndef __FMB_BROWSER_H__
#define __FMB_BROWSER_H__

#include <fmb/fmb-file.h>
#include <fmb/fmb-device.h>

G_BEGIN_DECLS

#define FMB_TYPE_BROWSER           (fmb_browser_get_type ())
#define FMB_BROWSER(obj)           (G_TYPE_CHECK_INSTANCE_CAST ((obj), FMB_TYPE_BROWSER, FmbBrowser))
#define FMB_IS_BROWSER(obj)        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FMB_TYPE_BROWSER))
#define FMB_BROWSER_GET_IFACE(obj) (G_TYPE_INSTANCE_GET_INTERFACE ((obj), FMB_TYPE_BROWSER, FmbBrowserIface))

typedef struct _FmbBrowser      FmbBrowser;
typedef struct _FmbBrowserIface FmbBrowserIface;

typedef void (*FmbBrowserPokeFileFunc)     (FmbBrowser *browser,
                                               FmbFile    *file,
                                               FmbFile    *target_file,
                                               GError        *error,
                                               gpointer       user_data);

typedef void (*FmbBrowserPokeDeviceFunc)   (FmbBrowser *browser,
                                               FmbDevice  *volume,
                                               FmbFile    *mount_point,
                                               GError        *error,
                                               gpointer       user_data);

typedef void (*FmbBrowserPokeLocationFunc) (FmbBrowser *browser,
                                               GFile         *location,
                                               FmbFile    *file,
                                               FmbFile    *target_file,
                                               GError        *error,
                                               gpointer       user_data);

struct _FmbBrowserIface
{
  GTypeInterface __parent__;

  /* signals */

  /* virtual methods */
};

GType fmb_browser_get_type      (void) G_GNUC_CONST;

void  fmb_browser_poke_file     (FmbBrowser                 *browser,
                                    FmbFile                    *file,
                                    gpointer                       widget,
                                    FmbBrowserPokeFileFunc      func,
                                    gpointer                       user_data);
void  fmb_browser_poke_device   (FmbBrowser                 *browser,
                                    FmbDevice                  *device,
                                    gpointer                       widget,
                                    FmbBrowserPokeDeviceFunc    func,
                                    gpointer                       user_data);
void  fmb_browser_poke_location (FmbBrowser                 *browser,
                                    GFile                         *location,
                                    gpointer                       widget,
                                    FmbBrowserPokeLocationFunc  func,
                                    gpointer                       user_data);

G_END_DECLS

#endif /* !__FMB_BROWSER_H__ */

/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2005-2006 Benedikt Meurer <benny@xfce.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#if !defined(FMBX_INSIDE_FMBX_H) && !defined(FMBX_COMPILATION)
#error "Only <fmbx/fmbx.h> can be included directly, this file may disappear or change contents"
#endif

#ifndef __FMBX_MENU_PROVIDER_H__
#define __FMBX_MENU_PROVIDER_H__

#include <gtk/gtk.h>

#include <fmbx/fmbx-file-info.h>

G_BEGIN_DECLS;

typedef struct _FmbxMenuProviderIface FmbxMenuProviderIface;
typedef struct _FmbxMenuProvider      FmbxMenuProvider;

#define FMBX_TYPE_MENU_PROVIDER           (fmbx_menu_provider_get_type ())
#define FMBX_MENU_PROVIDER(obj)           (G_TYPE_CHECK_INSTANCE_CAST ((obj), FMBX_TYPE_MENU_PROVIDER, FmbxMenuProvider))
#define FMBX_IS_MENU_PROVIDER(obj)        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FMBX_TYPE_MENU_PROVIDER))
#define FMBX_MENU_PROVIDER_GET_IFACE(obj) (G_TYPE_INSTANCE_GET_INTERFACE ((obj), FMBX_TYPE_MENU_PROVIDER, FmbxMenuProviderIface))

struct _FmbxMenuProviderIface
{
  /*< private >*/
  GTypeInterface __parent__;

  /*< public >*/
  GList *(*get_file_actions)    (FmbxMenuProvider *provider,
                                 GtkWidget           *window,
                                 GList               *files);

  GList *(*get_folder_actions)  (FmbxMenuProvider *provider,
                                 GtkWidget           *window,
                                 FmbxFileInfo     *folder);

  GList *(*get_dnd_actions)     (FmbxMenuProvider *provider,
                                 GtkWidget           *window,
                                 FmbxFileInfo     *folder,
                                 GList               *files);

  /*< private >*/
  void (*reserved1) (void);
  void (*reserved2) (void);
  void (*reserved3) (void);
};

GType  fmbx_menu_provider_get_type           (void) G_GNUC_CONST;

GList *fmbx_menu_provider_get_file_actions   (FmbxMenuProvider *provider,
                                                 GtkWidget           *window,
                                                 GList               *files) G_GNUC_MALLOC G_GNUC_WARN_UNUSED_RESULT;

GList *fmbx_menu_provider_get_folder_actions (FmbxMenuProvider *provider,
                                                 GtkWidget           *window,
                                                 FmbxFileInfo     *folder) G_GNUC_MALLOC G_GNUC_WARN_UNUSED_RESULT;

GList *fmbx_menu_provider_get_dnd_actions    (FmbxMenuProvider *provider,
                                                 GtkWidget           *window,
                                                 FmbxFileInfo     *folder,
                                                 GList               *files) G_GNUC_MALLOC G_GNUC_WARN_UNUSED_RESULT;

G_END_DECLS;

#endif /* !__FMBX_MENU_PROVIDER_H__ */

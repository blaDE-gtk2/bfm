/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2005 Benedikt Meurer <benny@xfce.org>
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

#ifndef __FMBX_PROVIDER_PLUGIN_H__
#define __FMBX_PROVIDER_PLUGIN_H__

#include <glib-object.h>

typedef struct _FmbxProviderPluginIface FmbxProviderPluginIface;
typedef struct _FmbxProviderPlugin      FmbxProviderPlugin;

#define FMBX_TYPE_PROVIDER_PLUGIN           (fmbx_provider_plugin_get_type ())
#define FMBX_PROVIDER_PLUGIN(obj)           (G_TYPE_CHECK_INSTANCE_CAST ((obj), FMBX_TYPE_PROVIDER_PLUGIN, FmbxProviderPlugin))
#define FMBX_IS_PROVIDER_PLUGIN(obj)        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FMBX_TYPE_PROVIDER_PLUGIN))
#define FMBX_PROVIDER_PLUGIN_GET_IFACE(obj) (G_TYPE_INSTANCE_GET_INTERFACE ((obj), FMBX_TYPE_PROVIDER_PLUGIN, FmbxProviderPluginIface))

struct _FmbxProviderPluginIface
{
  /*< private >*/
  GTypeInterface __parent__;

  /*< public >*/
  gboolean (*get_resident)    (const FmbxProviderPlugin *plugin);
  void     (*set_resident)    (FmbxProviderPlugin       *plugin,
                               gboolean                     resident);

  GType    (*register_type)   (FmbxProviderPlugin       *plugin,
                               GType                        type_parent,
                               const gchar                 *type_name,
                               const GTypeInfo             *type_info,
                               GTypeFlags                   type_flags);
  void     (*add_interface)   (FmbxProviderPlugin       *plugin,
                               GType                        instance_type,
                               GType                        interface_type,
                               const GInterfaceInfo        *interface_info);
  GType    (*register_enum)   (FmbxProviderPlugin       *plugin,
                               const gchar                 *name,
                               const GEnumValue            *const_static_values);
  GType    (*register_flags)  (FmbxProviderPlugin       *plugin,
                               const gchar                 *name,
                               const GFlagsValue           *const_static_values);

  /*< private >*/
  void (*reserved1) (void);
  void (*reserved2) (void);
  void (*reserved3) (void);
  void (*reserved4) (void);
};

GType     fmbx_provider_plugin_get_type       (void) G_GNUC_CONST;

gboolean  fmbx_provider_plugin_get_resident   (const FmbxProviderPlugin *plugin);
void      fmbx_provider_plugin_set_resident   (FmbxProviderPlugin       *plugin,
                                                  gboolean                     resident);

GType     fmbx_provider_plugin_register_type  (FmbxProviderPlugin *plugin,
                                                  GType                  type_parent,
                                                  const gchar           *type_name,
                                                  const GTypeInfo       *type_info,
                                                  GTypeFlags             type_flags);
void      fmbx_provider_plugin_add_interface  (FmbxProviderPlugin *plugin,
                                                  GType                  instance_type,
                                                  GType                  interface_type,
                                                  const GInterfaceInfo  *interface_info);
GType     fmbx_provider_plugin_register_enum  (FmbxProviderPlugin *plugin,
                                                  const gchar           *name,
                                                  const GEnumValue      *const_static_values);
GType     fmbx_provider_plugin_register_flags (FmbxProviderPlugin *plugin,
                                                  const gchar           *name,
                                                  const GFlagsValue     *const_static_values);

#endif /* !__FMBX_PROVIDER_PLUGIN_H__ */

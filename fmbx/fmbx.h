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

#ifndef __FMBX_H__
#define __FMBX_H__

#define FMBX_INSIDE_FMBX_H

#include <fmbx/fmbx-config.h>
#include <fmbx/fmbx-file-info.h>
#include <fmbx/fmbx-menu-provider.h>
#include <fmbx/fmbx-preferences-provider.h>
#include <fmbx/fmbx-property-page.h>
#include <fmbx/fmbx-property-page-provider.h>
#include <fmbx/fmbx-provider-factory.h>
#include <fmbx/fmbx-provider-plugin.h>
#include <fmbx/fmbx-renamer.h>
#include <fmbx/fmbx-renamer-provider.h>

#undef FMBX_INSIDE_FMBX_H

#define FMBX_DEFINE_TYPE(TN, t_n, T_P)                         FMBX_DEFINE_TYPE_EXTENDED (TN, t_n, T_P, 0, {})
#define FMBX_DEFINE_TYPE_WITH_CODE(TN, t_n, T_P, _C_)          FMBX_DEFINE_TYPE_EXTENDED (TN, t_n, T_P, 0, _C_)
#define FMBX_DEFINE_ABSTRACT_TYPE(TN, t_n, T_P)                FMBX_DEFINE_TYPE_EXTENDED (TN, t_n, T_P, G_TYPE_FLAG_ABSTRACT, {})
#define FMBX_DEFINE_ABSTRACT_TYPE_WITH_CODE(TN, t_n, T_P, _C_) FMBX_DEFINE_TYPE_EXTENDED (TN, t_n, T_P, G_TYPE_FLAG_ABSTRACT, _C_)

#define FMBX_DEFINE_TYPE_EXTENDED(TypeName, type_name, TYPE_PARENT, flags, CODE) \
static gpointer type_name##_parent_class = NULL; \
static GType    type_name##_type = G_TYPE_INVALID; \
\
static void     type_name##_init              (TypeName        *self); \
static void     type_name##_class_init        (TypeName##Class *klass); \
static void     type_name##_class_intern_init (TypeName##Class *klass) \
{ \
  type_name##_parent_class = g_type_class_peek_parent (klass); \
  type_name##_class_init (klass); \
} \
\
GType \
type_name##_get_type (void) \
{ \
  return type_name##_type; \
} \
\
void \
type_name##_register_type (FmbxProviderPlugin *fmbx_define_type_plugin) \
{ \
  GType fmbx_define_type_id; \
  static const GTypeInfo fmbx_define_type_info = \
  { \
    sizeof (TypeName##Class), \
    NULL, \
    NULL, \
    (GClassInitFunc) type_name##_class_intern_init, \
    NULL, \
    NULL, \
    sizeof (TypeName), \
    0, \
    (GInstanceInitFunc) type_name##_init, \
    NULL, \
  }; \
  fmbx_define_type_id = fmbx_provider_plugin_register_type (fmbx_define_type_plugin, TYPE_PARENT, \
                                                                  #TypeName, &fmbx_define_type_info, flags); \
  { CODE ; } \
  type_name##_type = fmbx_define_type_id; \
}

#define FMBX_IMPLEMENT_INTERFACE(TYPE_IFACE, iface_init) \
{ \
  static const GInterfaceInfo fmbx_implement_interface_info = \
  { \
    (GInterfaceInitFunc) iface_init \
  }; \
  fmbx_provider_plugin_add_interface (fmbx_define_type_plugin, fmbx_define_type_id, TYPE_IFACE, &fmbx_implement_interface_info); \
}

#endif /* !__FMBX_H__ */

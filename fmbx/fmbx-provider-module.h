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

#ifndef __FMBX_PROVIDER_MODULE_H__
#define __FMBX_PROVIDER_MODULE_H__

#include <glib-object.h>

typedef struct _FmbxProviderModuleClass FmbxProviderModuleClass;
typedef struct _FmbxProviderModule      FmbxProviderModule;

#define FMBX_TYPE_PROVIDER_MODULE            (fmbx_provider_module_get_type ())
#define FMBX_PROVIDER_MODULE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), FMBX_TYPE_PROVIDER_MODULE, FmbxProviderModule))
#define FMBX_PROVIDER_MODULE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), FMBX_TYPE_PROVIDER_MODULE, FmbxProviderModuleClass))
#define FMBX_IS_PROVIDER_MODULE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FMBX_TYPE_PROVIDER_MODULE))
#define FMBX_IS_PROVIDER_MODULE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), FMBX_TYPE_PROVIDER_MODULE))
#define FMBX_PROVIDER_MODULE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), FMBX_TYPE_PROVIDER_MODULE, FmbxProviderModuleClass))

G_GNUC_INTERNAL
GType                  fmbx_provider_module_get_type   (void) G_GNUC_CONST;

G_GNUC_INTERNAL
FmbxProviderModule *fmbx_provider_module_new        (const gchar                 *filename) G_GNUC_MALLOC;

G_GNUC_INTERNAL
void                   fmbx_provider_module_list_types (const FmbxProviderModule *module,
                                                           const GType                **types,
                                                           gint                        *n_types);

#endif /* !__FMBX_PROVIDER_MODULE_H__ */

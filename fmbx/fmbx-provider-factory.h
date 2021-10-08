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

#ifndef __FMBX_PROVIDER_FACTORY_H__
#define __FMBX_PROVIDER_FACTORY_H__

#include <glib-object.h>

G_BEGIN_DECLS;

typedef struct _FmbxProviderFactoryClass FmbxProviderFactoryClass;
typedef struct _FmbxProviderFactory      FmbxProviderFactory;

#define FMBX_TYPE_PROVIDER_FACTORY             (fmbx_provider_factory_get_type ())
#define FMBX_PROVIDER_FACTORY(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), FMBX_TYPE_PROVIDER_FACTORY, FmbxProviderFactory))
#define FMBX_PROVIDER_FACTORY_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), FMBX_TYPE_PROVIDER_FACTORY, FmbxProviderFactoryClass))
#define FMBX_IS_PROVIDER_FACTORY(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FMBX_TYPE_PROVIDER_FACTORY))
#define FMBX_IS_PROVIDER_FACTORY_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), FMBX_TYPE_PROVIDER_FACTORY))
#define FMBX_PROVIDER_FACTORY_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), FMBX_TYPE_PROVIDER_FACTORY, FmbxProviderFactoryClass))

GType                   fmbx_provider_factory_get_type       (void) G_GNUC_CONST;

FmbxProviderFactory *fmbx_provider_factory_get_default    (void);

GList                  *fmbx_provider_factory_list_providers (FmbxProviderFactory *factory,
                                                                 GType                   type) G_GNUC_MALLOC;

G_END_DECLS;

#endif /* !__FMBX_PROVIDER_FACTORY_H__ */

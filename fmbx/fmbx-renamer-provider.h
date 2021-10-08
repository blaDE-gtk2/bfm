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

#ifndef __FMBX_RENAMER_PROVIDER_H__
#define __FMBX_RENAMER_PROVIDER_H__

#include <fmbx/fmbx-renamer.h>

G_BEGIN_DECLS;

typedef struct _FmbxRenamerProviderIface FmbxRenamerProviderIface;
typedef struct _FmbxRenamerProvider      FmbxRenamerProvider;

#define FMBX_TYPE_RENAMER_PROVIDER           (fmbx_renamer_provider_get_type ())
#define FMBX_RENAMER_PROVIDER(obj)           (G_TYPE_CHECK_INSTANCE_CAST ((obj), FMBX_TYPE_RENAMER_PROVIDER, FmbxRenamerProvider))
#define FMBX_IS_RENAMER_PROVIDER(obj)        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FMBX_TYPE_RENAMER_PROVIDER))
#define FMBX_RENAMER_PROVIDER_GET_IFACE(obj) (G_TYPE_INSTANCE_GET_INTERFACE ((obj), FMBX_TYPE_RENAMER_PROVIDER, FmbxRenamerProviderIface))

struct _FmbxRenamerProviderIface
{
  /*< private >*/
  GTypeInterface __parent__;

  /*< public >*/
  GList *(*get_renamers) (FmbxRenamerProvider *provider);

  /*< private >*/
  void (*reserved1) (void);
  void (*reserved2) (void);
  void (*reserved3) (void);
  void (*reserved4) (void);
  void (*reserved5) (void);
};

GType  fmbx_renamer_provider_get_type     (void) G_GNUC_CONST;

GList *fmbx_renamer_provider_get_renamers (FmbxRenamerProvider *provider) G_GNUC_MALLOC;

G_END_DECLS;

#endif /* !__FMBX_RENAMER_PROVIDER_H__ */

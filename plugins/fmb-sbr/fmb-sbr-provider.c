/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2006 Benedikt Meurer <benny@xfce.org>
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <fmb-sbr/fmb-sbr-case-renamer.h>
#include <fmb-sbr/fmb-sbr-insert-renamer.h>
#include <fmb-sbr/fmb-sbr-number-renamer.h>
#include <fmb-sbr/fmb-sbr-provider.h>
#include <fmb-sbr/fmb-sbr-remove-renamer.h>
#include <fmb-sbr/fmb-sbr-replace-renamer.h>
#include <fmb-sbr/fmb-sbr-date-renamer.h>



static void   fmb_sbr_provider_renamer_provider_init (FmbxRenamerProviderIface *iface);
static GList *fmb_sbr_provider_get_renamers          (FmbxRenamerProvider      *renamer_provider);



struct _FmbSbrProviderClass
{
  GObjectClass __parent__;
};

struct _FmbSbrProvider
{
  GObject __parent__;
};



FMBX_DEFINE_TYPE_WITH_CODE (FmbSbrProvider,
                               fmb_sbr_provider,
                               G_TYPE_OBJECT,
                               FMBX_IMPLEMENT_INTERFACE (FMBX_TYPE_RENAMER_PROVIDER,
                                                            fmb_sbr_provider_renamer_provider_init));



static void
fmb_sbr_provider_class_init (FmbSbrProviderClass *klass)
{
}



static void
fmb_sbr_provider_renamer_provider_init (FmbxRenamerProviderIface *iface)
{
  iface->get_renamers = fmb_sbr_provider_get_renamers;
}



static void
fmb_sbr_provider_init (FmbSbrProvider *sbr_provider)
{
}



static GList*
fmb_sbr_provider_get_renamers (FmbxRenamerProvider *renamer_provider)
{
  GList *renamers = NULL;

  renamers = g_list_prepend (renamers, fmb_sbr_replace_renamer_new ());
  renamers = g_list_prepend (renamers, fmb_sbr_remove_renamer_new ());
  renamers = g_list_prepend (renamers, fmb_sbr_number_renamer_new ());
  renamers = g_list_prepend (renamers, fmb_sbr_insert_renamer_new ());
  renamers = g_list_prepend (renamers, fmb_sbr_case_renamer_new ());
  renamers = g_list_prepend (renamers, fmb_sbr_date_renamer_new ());

  return renamers;
}

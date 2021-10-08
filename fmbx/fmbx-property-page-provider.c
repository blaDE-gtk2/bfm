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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <fmbx/fmbx-private.h>
#include <fmbx/fmbx-property-page-provider.h>



GType
fmbx_property_page_provider_get_type (void)
{
  static volatile gsize type__volatile = 0;
  GType                 type;

  if (g_once_init_enter (&type__volatile))
    {
      type = g_type_register_static_simple (G_TYPE_INTERFACE,
                                            I_("FmbxPropertyPageProvider"),
                                            sizeof (FmbxPropertyPageProviderIface),
                                            NULL,
                                            0,
                                            NULL,
                                            0);

      g_type_interface_add_prerequisite (type, G_TYPE_OBJECT);

      g_once_init_leave (&type__volatile, type);
    }

  return type__volatile;
}



/**
 * fmbx_property_page_provider_get_pages:
 * @provider : a #FmbxPropertyPageProvider.
 * @files    : the list of #FmbxFileInfo<!---->s for which a properties dialog will be displayed.
 *
 * Returns the list of #FmbxPropertyPage<!---->s that @provider has to offer for @files.
 *
 * Extensions that implement this interface, must first check whether they support all the
 * #FmbxFileInfo<!---->s in the list of @files. Most extensions will probably only support
 * #FmbxPropertyPage<!---->s for exactly one file of a certain type. For example an MP3-Tag
 * editor property page will most probably support only a single audio file, and so the method
 * would be implemented like this
 * <informalexample><programlisting>
 * GList*
 * tag_provider_get_pages (FmbxPropertyPageProvider *property_page_provider,
 *                         GList                       *files)
 * {
 *   if (g_list_length (files) != 1)
 *     return NULL;
 *   else if (!fmbx_file_info_has_mime_type (files->data, "audio/mp3"))
 *     return NULL;
 *   else
 *     return g_list_append (NULL, tag_page_new (files->data));
 * }
 * </programlisting></informalexample>
 * where tag_page_new() allocates a new #TagPage instance for a #FmbxFileInfo object
 * passed to it. See the description of the #FmbxPropertyPage class for additional
 * information about the #TagPage example class.
 *
 * As a special note, this method automatically takes a reference on the
 * @provider for every #FmbxPropertyPage object returned from the real implementation
 * of this method in @provider. This is to make sure that the extension stays
 * in memory for atleast the time that the pages are used. If the extension
 * wants to stay in memory for a longer time, it'll need to take care of this
 * itself (e.g. by taking an additional reference on the @provider itself,
 * that's released at a later time).
 *
 * The caller is responsible to free the returned list of pages using
 * something like this when no longer needed:
 * <informalexample><programlisting>
 * g_list_foreach (list, (GFunc) g_object_ref_sink, NULL);
 * g_list_free_full (list, g_object_unref);
 * </programlisting></informalexample>
 *
 * Return value: the list of #FmbxPropertyPage<!---->s that @provider has to offer
 *               for @files.
 **/
GList*
fmbx_property_page_provider_get_pages (FmbxPropertyPageProvider *provider,
                                          GList                       *files)
{
  GList *pages;

  g_return_val_if_fail (FMBX_IS_PROPERTY_PAGE_PROVIDER (provider), NULL);
  g_return_val_if_fail (files != NULL, NULL);

  if (FMBX_PROPERTY_PAGE_PROVIDER_GET_IFACE (provider)->get_pages != NULL)
    {
      /* query the property pages from the implementation */
      pages = (*FMBX_PROPERTY_PAGE_PROVIDER_GET_IFACE (provider)->get_pages) (provider, files);

      /* take a reference on the provider for each page */
      fmbx_object_list_take_reference (pages, provider);
    }
  else
    {
      pages = NULL;
    }

  return pages;
}

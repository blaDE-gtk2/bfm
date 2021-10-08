/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2005-2006 Benedikt Meurer <benny@xfce.org>
 * Copyright (c) 2009 Jannis Pohlmann <jannis@xfce.org>
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

#include <fmbx/fmbx-file-info.h>
#include <fmbx/fmbx-private.h>



/* Signal identifiers */
enum
{
  CHANGED,
  RENAMED,
  LAST_SIGNAL,
};



static guint file_info_signals[LAST_SIGNAL];



GType
fmbx_file_info_get_type (void)
{
  static volatile gsize type__volatile = 0;
  GType                 type;

  if (g_once_init_enter (&type__volatile))
    {
      type = g_type_register_static_simple (G_TYPE_INTERFACE,
                                            I_("FmbxFileInfo"),
                                            sizeof (FmbxFileInfoIface),
                                            NULL,
                                            0,
                                            NULL,
                                            0);

      g_type_interface_add_prerequisite (type, G_TYPE_OBJECT);
      
      /**
       * FmbxFileInfo::changed:
       * @file_info : a #FmbxFileInfo.
       *
       * Emitted whenever the system notices a change to @file_info.
       *
       * Fmb plugins should use this signal to stay informed about
       * changes to a @file_info for which they currently display
       * information (i.e. in a #FmbxPropertyPage), and update
       * it's user interface whenever a change is noticed on @file_info.
       **/
      file_info_signals[CHANGED] =
        g_signal_new (I_("changed"),
                      type,
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (FmbxFileInfoIface, changed),
                      NULL, NULL,
                      g_cclosure_marshal_VOID__VOID,
                      G_TYPE_NONE, 0);

      /**
       * FmbxFileInfo::renamed:
       * @file_info : a #FmbxFileInfo
       *
       * Emitted when the @file_info is renamed to another
       * name.
       *
       * For example, within Fmb, #FmbFolder uses this
       * signal to reregister it's VFS directory monitor, after
       * the corresponding file was renamed.
       **/
      file_info_signals[RENAMED] =
        g_signal_new (I_("renamed"),
                      type,
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (FmbxFileInfoIface, renamed),
                      NULL, NULL,
                      g_cclosure_marshal_VOID__VOID,
                      G_TYPE_NONE, 0);

      g_once_init_leave (&type__volatile, type);
    }

  return type__volatile;
}



/**
 * fmbx_file_info_get_name:
 * @file_info : a #FmbxFileInfo.
 *
 * Returns the real name of the file represented
 * by @file_info in the local file system encoding.
 * You can use g_filename_display_name() or similar
 * functions to generate an UTF-8 version of the
 * name, which is suitable for use in the user
 * interface.
 *
 * The caller is responsible to free the returned
 * string using g_free() when no longer needed.
 *
 * Return value: the real name of the file represented
 *               by @file_info.
 **/
gchar*
fmbx_file_info_get_name (FmbxFileInfo *file_info)
{
  g_return_val_if_fail (FMBX_IS_FILE_INFO (file_info), NULL);
  return (*FMBX_FILE_INFO_GET_IFACE (file_info)->get_name) (file_info);
}



/**
 * fmbx_file_info_get_uri:
 * @file_info : a #FmbxFileInfo.
 *
 * Returns the escaped, fully qualified URI
 * of the file object represented by @file_info.
 * You may use g_filename_from_uri() and similar
 * functions to work with the returned URI.
 *
 * The caller is responsible to free the returned
 * string using g_free() when no longer needed.
 *
 * Return value: the fully qualified URI of @file_info.
 **/
gchar*
fmbx_file_info_get_uri (FmbxFileInfo *file_info)
{
  g_return_val_if_fail (FMBX_IS_FILE_INFO (file_info), NULL);
  return (*FMBX_FILE_INFO_GET_IFACE (file_info)->get_uri) (file_info);
}



/**
 * fmbx_file_info_get_parent_uri:
 * @file_info : a #FmbxFileInfo.
 *
 * Returns the URI to the parent file of
 * @file_info or %NULL if @file_info has
 * no parent. Note that the parent URI
 * may be of a different type than the
 * URI of @file_info. For example, the
 * parent of "file:///" is "computer:///".
 *
 * The caller is responsible to free the
 * returned string using g_free() when no
 * longer needed.
 *
 * Return value: the parent URI for @file_info
 *               or %NULL.
 **/
gchar*
fmbx_file_info_get_parent_uri (FmbxFileInfo *file_info)
{
  g_return_val_if_fail (FMBX_IS_FILE_INFO (file_info), NULL);
  return (*FMBX_FILE_INFO_GET_IFACE (file_info)->get_parent_uri) (file_info);
}



/**
 * fmbx_file_info_get_uri_scheme:
 * @file_info : a #FmbxFileInfo.
 *
 * Returns the URI scheme of the file represented
 * by @file_info. E.g. if @file_info refers to the
 * file "file:///usr/home", the return value will
 * be "file".
 *
 * The caller is responsible to free the returned
 * string using g_free() when no longer needed.
 *
 * Return value: the URI scheme for @file_info.
 **/
gchar*
fmbx_file_info_get_uri_scheme (FmbxFileInfo *file_info)
{
  g_return_val_if_fail (FMBX_IS_FILE_INFO (file_info), NULL);
  return (*FMBX_FILE_INFO_GET_IFACE (file_info)->get_uri_scheme) (file_info);
}



/**
 * fmbx_file_info_get_mime_type:
 * @file_info : a #FmbxFileInfo.
 *
 * Returns the MIME-type of the file represented by
 * @file_info or %NULL if no MIME-type is known for
 * @file_info.
 *
 * The caller is responsible to free the returned
 * string using g_free() when no longer needed.
 *
 * Return value: the MIME-type for @file_info or
 *               %NULL.
 **/
gchar*
fmbx_file_info_get_mime_type (FmbxFileInfo *file_info)
{
  g_return_val_if_fail (FMBX_IS_FILE_INFO (file_info), NULL);
  return (*FMBX_FILE_INFO_GET_IFACE (file_info)->get_mime_type) (file_info);
}



/**
 * fmbx_file_info_has_mime_type:
 * @file_info : a #FmbxFileInfo.
 * @mime_type : a MIME-type (e.g. "text/plain").
 *
 * Checks whether @file_info is of the given @mime_type
 * or whether the MIME-type of @file_info is a subclass
 * of @mime_type.
 *
 * This is the preferred way for most extensions to check
 * whether they support a given file or not, and you should
 * consider using this method rather than
 * fmbx_file_info_get_mime_type(). A simple example would
 * be a menu extension that performs a certain action on
 * text files. In this case you want to check whether a given
 * #FmbxFileInfo refers to any kind of text file, not only
 * to "text/plain" (e.g. this also includes "text/xml" and
 * "application/x-desktop").
 *
 * But you should be aware that this method may take some
 * time to test whether @mime_type is valid for @file_info,
 * so don't call it too often.
 *
 * Return value: %TRUE if @mime_type is valid for @file_info,
 *               else %FALSE.
 **/
gboolean
fmbx_file_info_has_mime_type (FmbxFileInfo *file_info,
                                 const gchar     *mime_type)
{
  g_return_val_if_fail (FMBX_IS_FILE_INFO (file_info), FALSE);
  g_return_val_if_fail (mime_type != NULL, FALSE);
  return (*FMBX_FILE_INFO_GET_IFACE (file_info)->has_mime_type) (file_info, mime_type);
}



/**
 * fmbx_file_info_is_directory:
 * @file_info : a #FmbxFileInfo.
 *
 * Checks whether @file_info refers to a directory.
 *
 * Return value: %TRUE if @file_info is a directory.
 **/
gboolean
fmbx_file_info_is_directory (FmbxFileInfo *file_info)
{
  g_return_val_if_fail (FMBX_IS_FILE_INFO (file_info), FALSE);
  return (*FMBX_FILE_INFO_GET_IFACE (file_info)->is_directory) (file_info);
}



/**
 * fmbx_file_info_get_file_info:
 * @file_info : a #FmbxFileInfo.
 *
 * Returns the #GFileInfo associated with @file_info,
 * which includes additional information about the @file_info
 * as queried from GIO earlier. The caller is responsible to free the 
 * returned #GFileInfo object using g_object_unref() when 
 * no longer needed.
 *
 * Return value: the #GFileInfo object associated with @file_info,
 *               which MUST be freed using g_object_unref().
 **/
GFileInfo*
fmbx_file_info_get_file_info (FmbxFileInfo *file_info)
{
  g_return_val_if_fail (FMBX_IS_FILE_INFO (file_info), NULL);
  return (*FMBX_FILE_INFO_GET_IFACE (file_info)->get_file_info) (file_info);
}



/**
 * fmbx_file_info_get_filesystem_info:
 * @file_info : a #FmbxFileInfo.
 *
 * Returns the #GFileInfo which includes additional information about
 * the filesystem @file_info resides on. The caller is responsible to 
 * free the returned #GFileInfo object using g_object_unref() when 
 * no longer needed.
 *
 * Return value: the #GFileInfo containing information about the
 *               filesystem of @file_info or %NULL if no filesystem 
 *               information is available. It MUST be released using
 *               g_object_unref().
 **/
GFileInfo*
fmbx_file_info_get_filesystem_info (FmbxFileInfo *file_info)
{
  g_return_val_if_fail (FMBX_IS_FILE_INFO (file_info), NULL);
  return (*FMBX_FILE_INFO_GET_IFACE (file_info)->get_filesystem_info) (file_info);
}



/**
 * fmbx_file_info_get_location:
 * @file_info : a #FmbxFileInfo.
 *
 * Returns the #GFile @file_info points to. The #GFile is a more
 * powerful tool than just the URI or the path. The caller
 * is responsible to release the returned #GFile using g_object_unref()
 * when no longer needed.
 *
 * Return value: the #GFile to which @file_info points. It MUST be
 *               released using g_object_unref().
 **/
GFile*
fmbx_file_info_get_location (FmbxFileInfo *file_info)
{
  g_return_val_if_fail (FMBX_IS_FILE_INFO (file_info), NULL);
  return (*FMBX_FILE_INFO_GET_IFACE (file_info)->get_location) (file_info);
}



/**
 * fmbx_file_info_changed:
 * @file_info : a #FmbxFileInfo.
 *
 * Emits the ::changed signal on @file_info. This method should not
 * be invoked by Fmb plugins, instead the file manager itself
 * will use this method to emit ::changed whenever it notices a
 * change on @file_info.
 **/
void
fmbx_file_info_changed (FmbxFileInfo *file_info)
{
  g_return_if_fail (FMBX_IS_FILE_INFO (file_info));
  g_signal_emit (G_OBJECT (file_info), file_info_signals[CHANGED], 0);
}



/**
 * fmbx_file_info_renamed:
 * @file_info : a #FmbxFileInfo.
 *
 * Emits the ::renamed signal on @file_info. This method should
 * not be invoked by Fmb plugins, instead the file manager
 * will emit this signal whenever the user renamed the @file_info.
 *
 * The plugins should instead connect to the ::renamed signal
 * and update it's internal state and it's user interface
 * after the file manager renamed a file.
 **/
void
fmbx_file_info_renamed (FmbxFileInfo *file_info)
{
  g_return_if_fail (FMBX_IS_FILE_INFO (file_info));
  g_signal_emit (G_OBJECT (file_info), file_info_signals[RENAMED], 0);
}



GType
fmbx_file_info_list_get_type (void)
{
  static GType type = G_TYPE_INVALID;

  if (G_UNLIKELY (type == G_TYPE_INVALID))
    {
      type = g_boxed_type_register_static (I_("FmbxFileInfoList"),
                                           (GBoxedCopyFunc) fmbx_file_info_list_copy,
                                           (GBoxedFreeFunc) fmbx_file_info_list_free);
    }

  return type;
}



/**
 * fmbx_file_info_list_copy:
 * @file_infos : a #GList of #FmbxFileInfo<!---->s.
 *
 * Does a deep copy of @file_infos and returns the
 * new list.
 *
 * Return value: a copy of @file_infos.
 **/
GList*
fmbx_file_info_list_copy (GList *file_infos)
{
#if GLIB_CHECK_VERSION (2, 34, 0)
  return g_list_copy_deep (file_infos, (GCopyFunc) g_object_ref, NULL);
#else
  GList *copy;

  copy = g_list_copy (file_infos);
  g_list_foreach (copy, (GFunc) g_object_ref, NULL);

  return copy;
#endif
}



/**
 * fmbx_file_info_list_free:
 * @file_infos : a #GList of #FmbxFileInfo<!---->s.
 *
 * Frees the resources allocated for the @file_infos
 * list and decreases the reference count on the
 * #FmbxFileInfo<!---->s contained within.
 **/
void
fmbx_file_info_list_free (GList *file_infos)
{
  g_list_free_full (file_infos, g_object_unref);
}

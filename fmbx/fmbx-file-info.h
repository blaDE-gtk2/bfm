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

#if !defined(FMBX_INSIDE_FMBX_H) && !defined(FMBX_COMPILATION)
#error "Only <fmbx/fmbx.h> can be included directly, this file may disappear or change contents"
#endif

#ifndef __FMBX_FILE_INFO_H__
#define __FMBX_FILE_INFO_H__

#include <gio/gio.h>

#include <fmbx/fmbx-config.h>

G_BEGIN_DECLS;

/**
 * File information namespaces available in the #GFileInfo returned by 
 * fmbx_file_info_get_file_info().
 **/
#define FMBX_FILE_INFO_NAMESPACE \
  "access::*," \
  "id::filesystem," \
  "mountable::can-mount,standard::target-uri," \
  "preview::*," \
  "standard::type,standard::is-hidden,standard::is-backup," \
  "standard::is-symlink,standard::name,standard::display-name," \
  "standard::size,standard::symlink-target," \
  "time::*," \
  "trash::*," \
  "unix::gid,unix::uid,unix::mode," \
  "metadata::emblems"



/**
 * Filesystem information namespaces available in the #GFileInfo
 * returned by fmbx_file_info_get_filesystem_info().
 **/
#define FMBX_FILESYSTEM_INFO_NAMESPACE \
  "filesystem::*"

typedef struct _FmbxFileInfoIface FmbxFileInfoIface;
typedef struct _FmbxFileInfo      FmbxFileInfo;

#define FMBX_TYPE_FILE_INFO            (fmbx_file_info_get_type ())
#define FMBX_FILE_INFO(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), FMBX_TYPE_FILE_INFO, FmbxFileInfo))
#define FMBX_IS_FILE_INFO(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FMBX_TYPE_FILE_INFO))
#define FMBX_FILE_INFO_GET_IFACE(obj)  (G_TYPE_INSTANCE_GET_INTERFACE ((obj), FMBX_TYPE_FILE_INFO, FmbxFileInfoIface))

struct _FmbxFileInfoIface
{
  /*< private >*/
  GTypeInterface __parent__;

  /*< public >*/

  /* virtual methods */
  gchar     *(*get_name)            (FmbxFileInfo *file_info);

  gchar     *(*get_uri)             (FmbxFileInfo *file_info);
  gchar     *(*get_parent_uri)      (FmbxFileInfo *file_info);
  gchar     *(*get_uri_scheme)      (FmbxFileInfo *file_info);

  gchar     *(*get_mime_type)       (FmbxFileInfo *file_info);
  gboolean   (*has_mime_type)       (FmbxFileInfo *file_info,
                                     const gchar     *mime_type);

  gboolean   (*is_directory)        (FmbxFileInfo *file_info);

  GFileInfo *(*get_file_info)       (FmbxFileInfo *file_info);
  GFileInfo *(*get_filesystem_info) (FmbxFileInfo *file_info);
  GFile     *(*get_location)        (FmbxFileInfo *file_info);

  /*< private >*/
  void (*reserved0) (void);
  void (*reserved1) (void);
  void (*reserved2) (void);
  void (*reserved3) (void);
  void (*reserved4) (void);
  void (*reserved5) (void);
  void (*reserved6) (void);

  /*< public >*/

  /* signals */
  void (*changed) (FmbxFileInfo *file_info);
  void (*renamed) (FmbxFileInfo *file_info);

  /*< private >*/
  void (*reserved7) (void);
  void (*reserved8) (void);
  void (*reserved9) (void);
};

GType      fmbx_file_info_get_type            (void) G_GNUC_CONST;

gchar     *fmbx_file_info_get_name            (FmbxFileInfo *file_info);
gchar     *fmbx_file_info_get_uri             (FmbxFileInfo *file_info);
gchar     *fmbx_file_info_get_parent_uri      (FmbxFileInfo *file_info);
gchar     *fmbx_file_info_get_uri_scheme      (FmbxFileInfo *file_info);

gchar     *fmbx_file_info_get_mime_type       (FmbxFileInfo *file_info);
gboolean   fmbx_file_info_has_mime_type       (FmbxFileInfo *file_info,
                                                  const gchar     *mime_type);

gboolean   fmbx_file_info_is_directory        (FmbxFileInfo *file_info);

GFileInfo *fmbx_file_info_get_file_info       (FmbxFileInfo *file_info);
GFileInfo *fmbx_file_info_get_filesystem_info (FmbxFileInfo *file_info);
GFile     *fmbx_file_info_get_location        (FmbxFileInfo *file_info);

void       fmbx_file_info_changed             (FmbxFileInfo *file_info);
void       fmbx_file_info_renamed             (FmbxFileInfo *file_info);


#define FMBX_TYPE_FILE_INFO_LIST (fmbx_file_info_list_get_type ())

GType      fmbx_file_info_list_get_type       (void) G_GNUC_CONST;

GList     *fmbx_file_info_list_copy           (GList           *file_infos);
void       fmbx_file_info_list_free           (GList           *file_infos);

G_END_DECLS;

#endif /* !__FMBX_FILE_INFO_H__ */

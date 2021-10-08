/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2006-2007 Benedikt Meurer <benny@xfce.org>
 * Copyright (c) 2009 Jannis Pohlmann <jannis@xfce.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __FMB_ENUM_TYPES_H__
#define __FMB_ENUM_TYPES_H__

#include <blxo/blxo.h>

G_BEGIN_DECLS;

#define FMB_TYPE_RENAMER_MODE (fmb_renamer_mode_get_type ())

/**
 * FmbRenamerMode:
 * @FMB_RENAMER_MODE_NAME   : only the name should be renamed.
 * @FMB_RENAMER_MODE_SUFFIX : only the suffix should be renamed.
 * @FMB_RENAMER_MODE_BOTH   : the name and the suffix should be renamed.
 *
 * The rename mode for a #FmbRenamerModel instance.
 **/
typedef enum
{
  FMB_RENAMER_MODE_NAME,
  FMB_RENAMER_MODE_SUFFIX,
  FMB_RENAMER_MODE_BOTH,
} FmbRenamerMode;

GType fmb_renamer_mode_get_type (void) G_GNUC_CONST;



#define FMB_TYPE_DATE_STYLE (fmb_date_style_get_type ())

/**
 * FmbDateStyle:
 * @FMB_DATE_STYLE_SIMPLE : display only the date.
 * @FMB_DATE_STYLE_SHORT  : display date and time in a short manner.
 * @FMB_DATE_STYLE_LONG   : display date and time in a long manner.
 * @FMB_DATE_STYLE_ISO    : display date and time in ISO standard form.
 *
 * The style used to display dates (i.e. modification dates) to the user.
 **/
typedef enum
{
  FMB_DATE_STYLE_SIMPLE,
  FMB_DATE_STYLE_SHORT,
  FMB_DATE_STYLE_LONG,
  FMB_DATE_STYLE_ISO,
} FmbDateStyle;

GType fmb_date_style_get_type (void) G_GNUC_CONST;


#define FMB_TYPE_COLUMN (fmb_column_get_type ())

/**
 * FmbColumn:
 * @FMB_COLUMN_DATE_ACCESSED : last access time.
 * @FMB_COLUMN_DATE_MODIFIED : last modification time.
 * @FMB_COLUMN_GROUP         : group's name.
 * @FMB_COLUMN_MIME_TYPE     : mime type (i.e. "text/plain").
 * @FMB_COLUMN_NAME          : display name.
 * @FMB_COLUMN_OWNER         : owner's name.
 * @FMB_COLUMN_PERMISSIONS   : permissions bits.
 * @FMB_COLUMN_SIZE          : file size.
 * @FMB_COLUMN_TYPE          : file type (i.e. 'plain text document').
 * @FMB_COLUMN_FILE          : #FmbFile object.
 * @FMB_COLUMN_FILE_NAME     : real file name.
 *
 * Columns exported by #FmbListModel using the #GtkTreeModel
 * interface.
 **/
typedef enum
{
  /* visible columns */
  FMB_COLUMN_DATE_ACCESSED,
  FMB_COLUMN_DATE_MODIFIED,
  FMB_COLUMN_GROUP,
  FMB_COLUMN_MIME_TYPE,
  FMB_COLUMN_NAME,
  FMB_COLUMN_OWNER,
  FMB_COLUMN_PERMISSIONS,
  FMB_COLUMN_SIZE,
  FMB_COLUMN_TYPE,

  /* special internal columns */
  FMB_COLUMN_FILE,
  FMB_COLUMN_FILE_NAME,

  /* number of columns */
  FMB_N_COLUMNS,

  /* number of visible columns */
  FMB_N_VISIBLE_COLUMNS = FMB_COLUMN_FILE,
} FmbColumn;

GType fmb_column_get_type (void) G_GNUC_CONST;


#define FMB_TYPE_ICON_SIZE (fmb_icon_size_get_type ())

/**
 * FmbIconSize:
 * @FMB_ICON_SIZE_SMALLEST : icon size for #FMB_ZOOM_LEVEL_SMALLEST.
 * @FMB_ICON_SIZE_SMALLER  : icon size for #FMB_ZOOM_LEVEL_SMALLER.
 * @FMB_ICON_SIZE_SMALL    : icon size for #FMB_ZOOM_LEVEL_SMALL.
 * @FMB_ICON_SIZE_NORMAL   : icon size for #FMB_ZOOM_LEVEL_NORMAL.
 * @FMB_ICON_SIZE_LARGE    : icon size for #FMB_ZOOM_LEVEL_LARGE.
 * @FMB_ICON_SIZE_LARGER   : icon size for #FMB_ZOOM_LEVEL_LARGER.
 * @FMB_ICON_SIZE_LARGEST  : icon size for #FMB_ZOOM_LEVEL_LARGEST.
 *
 * Icon sizes matching the various #FmbZoomLevel<!---->s.
 **/
typedef enum
{
  FMB_ICON_SIZE_SMALLEST = 16,
  FMB_ICON_SIZE_SMALLER  = 24,
  FMB_ICON_SIZE_SMALL    = 32,
  FMB_ICON_SIZE_NORMAL   = 48,
  FMB_ICON_SIZE_LARGE    = 64,
  FMB_ICON_SIZE_LARGER   = 96,
  FMB_ICON_SIZE_LARGEST  = 128,
} FmbIconSize;

GType fmb_icon_size_get_type (void) G_GNUC_CONST;


#define FMB_TYPE_THUMBNAIL_MODE (fmb_thumbnail_mode_get_type ())

/**
 * FmbThumbnailsShow:
 * @FMB_THUMBNAIL_MODE_NEVER      : never show thumbnails.
 * @FMB_THUMBNAIL_MODE_ONLY_LOCAL : only show thumbnails on local filesystems.
 * @FMB_THUMBNAIL_MODE_ALWAYS     : always show thumbnails (everywhere).
 **/
typedef enum
{
  FMB_THUMBNAIL_MODE_NEVER,
  FMB_THUMBNAIL_MODE_ONLY_LOCAL,
  FMB_THUMBNAIL_MODE_ALWAYS
} FmbThumbnailMode;

GType fmb_thumbnail_mode_get_type (void) G_GNUC_CONST;


#define FMB_TYPE_RECURSIVE_PERMISSIONS (fmb_recursive_permissions_get_type ())

/**
 * FmbRecursivePermissionsMode:
 * @FMB_RECURSIVE_PERMISSIONS_ASK    : ask the user everytime permissions are changed.
 * @FMB_RECURSIVE_PERMISSIONS_ALWAYS : always apply the change recursively.
 * @FMB_RECURSIVE_PERMISSIONS_NEVER  : never apply the change recursively.
 *
 * Modus operandi when changing permissions.
 **/
typedef enum
{
  FMB_RECURSIVE_PERMISSIONS_ASK,
  FMB_RECURSIVE_PERMISSIONS_ALWAYS,
  FMB_RECURSIVE_PERMISSIONS_NEVER,
} FmbRecursivePermissionsMode;

GType fmb_recursive_permissions_get_type (void) G_GNUC_CONST;


#define FMB_TYPE_ZOOM_LEVEL (fmb_zoom_level_get_type ())

/**
 * FmbZoomLevel:
 * @FMB_ZOOM_LEVEL_SMALLEST : smallest possible zoom level.
 * @FMB_ZOOM_LEVEL_SMALLER  : smaller zoom level.
 * @FMB_ZOOM_LEVEL_SMALL    : small zoom level.
 * @FMB_ZOOM_LEVEL_NORMAL   : the default zoom level.
 * @FMB_ZOOM_LEVEL_LARGE    : large zoom level.
 * @FMB_ZOOM_LEVEL_LARGER   : larger zoom level.
 * @FMB_ZOOM_LEVEL_LARGEST  : largest possible zoom level.
 *
 * Lists the various zoom levels supported by Fmb's
 * folder views.
 **/
typedef enum
{
  FMB_ZOOM_LEVEL_SMALLEST,
  FMB_ZOOM_LEVEL_SMALLER,
  FMB_ZOOM_LEVEL_SMALL,
  FMB_ZOOM_LEVEL_NORMAL,
  FMB_ZOOM_LEVEL_LARGE,
  FMB_ZOOM_LEVEL_LARGER,
  FMB_ZOOM_LEVEL_LARGEST,

  /*< private >*/
  FMB_ZOOM_N_LEVELS,
} FmbZoomLevel;

GType          fmb_zoom_level_get_type     (void) G_GNUC_CONST;


#define FMB_TYPE_JOB_RESPONSE (fmb_job_response_get_type ())

/**
 * FmbJobResponse:
 * @FMB_JOB_RESPONSE_YES     :
 * @FMB_JOB_RESPONSE_YES_ALL :
 * @FMB_JOB_RESPONSE_NO      :
 * @FMB_JOB_RESPONSE_NO_ALL  :
 * @FMB_JOB_RESPONSE_CANCEL  :
 * @FMB_JOB_RESPONSE_RETRY   :
 * @FMB_JOB_RESPONSE_FORCE   :
 *
 * Possible responses for the FmbJob::ask signal.
 **/
typedef enum /*< flags >*/
{
  FMB_JOB_RESPONSE_YES     = 1 << 0,
  FMB_JOB_RESPONSE_YES_ALL = 1 << 1,
  FMB_JOB_RESPONSE_NO      = 1 << 2,
  FMB_JOB_RESPONSE_CANCEL  = 1 << 3,
  FMB_JOB_RESPONSE_NO_ALL  = 1 << 4,
  FMB_JOB_RESPONSE_RETRY   = 1 << 5,
  FMB_JOB_RESPONSE_FORCE   = 1 << 6,
} FmbJobResponse;

GType fmb_job_response_get_type (void) G_GNUC_CONST;


#define FMB_TYPE_FILE_MODE (fmb_file_mode_get_type ())

/**
 * FmbFileMode:
 *
 * Special flags and permissions of a filesystem entity.
 **/
typedef enum /*< flags >*/
{
  FMB_FILE_MODE_SUID       = 04000,
  FMB_FILE_MODE_SGID       = 02000,
  FMB_FILE_MODE_STICKY     = 01000,
  FMB_FILE_MODE_USR_ALL    = 00700,
  FMB_FILE_MODE_USR_READ   = 00400,
  FMB_FILE_MODE_USR_WRITE  = 00200,
  FMB_FILE_MODE_USR_EXEC   = 00100,
  FMB_FILE_MODE_GRP_ALL    = 00070,
  FMB_FILE_MODE_GRP_READ   = 00040,
  FMB_FILE_MODE_GRP_WRITE  = 00020,
  FMB_FILE_MODE_GRP_EXEC   = 00010,
  FMB_FILE_MODE_OTH_ALL    = 00007,
  FMB_FILE_MODE_OTH_READ   = 00004,
  FMB_FILE_MODE_OTH_WRITE  = 00002,
  FMB_FILE_MODE_OTH_EXEC   = 00001,
} FmbFileMode;

GType fmb_file_mode_get_type (void) G_GNUC_CONST;

G_END_DECLS;

#endif /* !__FMB_ENUM_TYPES_H__ */

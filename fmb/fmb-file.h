/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2005-2007 Benedikt Meurer <benny@xfce.org>
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

#ifndef __FMB_FILE_H__
#define __FMB_FILE_H__

#include <glib.h>

#include <fmbx/fmbx.h>

#include <fmb/fmb-enum-types.h>
#include <fmb/fmb-gio-extensions.h>
#include <fmb/fmb-user.h>

G_BEGIN_DECLS;

typedef struct _FmbFileClass FmbFileClass;
typedef struct _FmbFile      FmbFile;

#define FMB_TYPE_FILE            (fmb_file_get_type ())
#define FMB_FILE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), FMB_TYPE_FILE, FmbFile))
#define FMB_FILE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), FMB_TYPE_FILE, FmbFileClass))
#define FMB_IS_FILE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FMB_TYPE_FILE))
#define FMB_IS_FILE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), FMB_TYPE_FILE))
#define FMB_FILE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), FMB_TYPE_FILE, FmbFileClass))

/**
 * FmbFileDateType:
 * @FMB_FILE_DATE_ACCESSED : date of last access to the file.
 * @FMB_FILE_DATE_CHANGED  : date of last change to the file meta data or the content.
 * @FMB_FILE_DATE_MODIFIED : date of last modification of the file's content.
 *
 * The various dates that can be queried about a #FmbFile. Note, that not all
 * #FmbFile implementations support all types listed above. See the documentation
 * of the fmb_file_get_date() method for details.
 **/
typedef enum
{
  FMB_FILE_DATE_ACCESSED,
  FMB_FILE_DATE_CHANGED,
  FMB_FILE_DATE_MODIFIED,
} FmbFileDateType;

/**
 * FmbFileIconState:
 * @FMB_FILE_ICON_STATE_DEFAULT : the default icon for the file.
 * @FMB_FILE_ICON_STATE_DROP    : the drop accept icon for the file.
 * @FMB_FILE_ICON_STATE_OPEN    : the folder is expanded.
 *
 * The various file icon states that are used within the file manager
 * views.
 **/
typedef enum /*< enum >*/
{
  FMB_FILE_ICON_STATE_DEFAULT,
  FMB_FILE_ICON_STATE_DROP,
  FMB_FILE_ICON_STATE_OPEN,
} FmbFileIconState;

/**
 * FmbFileThumbState:
 * @FMB_FILE_THUMB_STATE_UNKNOWN : unknown whether there's a thumbnail.
 * @FMB_FILE_THUMB_STATE_NONE    : no thumbnail is available.
 * @FMB_FILE_THUMB_STATE_READY   : a thumbnail is available.
 * @FMB_FILE_THUMB_STATE_LOADING : a thumbnail is being generated.
 *
 * The state of the thumbnailing for a given #FmbFile.
 **/
typedef enum /*< flags >*/
{
  FMB_FILE_THUMB_STATE_UNKNOWN = 0,
  FMB_FILE_THUMB_STATE_NONE    = 1,
  FMB_FILE_THUMB_STATE_READY   = 2,
  FMB_FILE_THUMB_STATE_LOADING = 3,
} FmbFileThumbState;



#define FMB_FILE_EMBLEM_NAME_SYMBOLIC_LINK "emblem-symbolic-link"
#define FMB_FILE_EMBLEM_NAME_CANT_READ     "emblem-noread"
#define FMB_FILE_EMBLEM_NAME_CANT_WRITE    "emblem-nowrite"
#define FMB_FILE_EMBLEM_NAME_DESKTOP       "emblem-desktop"



/**
 * FmbFileGetFunc:
 *
 * Callback type for loading #FmbFile<!---->s asynchronously. If you
 * want to keep the #FmbFile, you need to ref it, else it will be
 * destroyed.
 **/
typedef void (*FmbFileGetFunc) (GFile      *location,
                                   FmbFile *file,
                                   GError     *error,
                                   gpointer    user_data);



GType             fmb_file_get_type                   (void) G_GNUC_CONST;

FmbFile       *fmb_file_get                        (GFile                  *file,
                                                          GError                **error);
FmbFile       *fmb_file_get_with_info              (GFile                  *file,
                                                          GFileInfo              *info,
                                                          gboolean                not_mounted);
FmbFile       *fmb_file_get_for_uri                (const gchar            *uri,
                                                          GError                **error);
void              fmb_file_get_async                  (GFile                  *location,
                                                          GCancellable           *cancellable,
                                                          FmbFileGetFunc       func,
                                                          gpointer                user_data);

GFile            *fmb_file_get_file                   (const FmbFile       *file) G_GNUC_PURE;

GFileInfo        *fmb_file_get_info                   (const FmbFile       *file) G_GNUC_PURE;

FmbFile       *fmb_file_get_parent                 (const FmbFile       *file,
                                                          GError                **error);

gboolean          fmb_file_check_loaded               (FmbFile             *file);

gboolean          fmb_file_execute                    (FmbFile             *file,
                                                          GFile                  *working_directory,
                                                          gpointer                parent,
                                                          GList                  *path_list,
                                                          const gchar            *startup_id,
                                                          GError                **error);

gboolean          fmb_file_launch                     (FmbFile             *file,
                                                          gpointer                parent,
                                                          const gchar            *startup_id,
                                                          GError                **error);

gboolean          fmb_file_rename                     (FmbFile             *file,
                                                          const gchar            *name,
                                                          GCancellable           *cancellable,
                                                          gboolean                called_from_job,
                                                          GError                **error);

GdkDragAction     fmb_file_accepts_drop               (FmbFile             *file,
                                                          GList                  *path_list,
                                                          GdkDragContext         *context,
                                                          GdkDragAction          *suggested_action_return);

guint64           fmb_file_get_date                   (const FmbFile       *file,
                                                          FmbFileDateType      date_type) G_GNUC_PURE;

gchar            *fmb_file_get_date_string            (const FmbFile       *file,
                                                          FmbFileDateType      date_type,
                                                          FmbDateStyle         date_style) G_GNUC_MALLOC G_GNUC_WARN_UNUSED_RESULT;
gchar            *fmb_file_get_mode_string            (const FmbFile       *file) G_GNUC_MALLOC G_GNUC_WARN_UNUSED_RESULT;
gchar            *fmb_file_get_size_string            (const FmbFile       *file) G_GNUC_MALLOC G_GNUC_WARN_UNUSED_RESULT;
gchar            *fmb_file_get_size_string_formatted  (const FmbFile       *file, 
                                                          const gboolean          file_size_binary);

GVolume          *fmb_file_get_volume                 (const FmbFile       *file);

FmbGroup      *fmb_file_get_group                  (const FmbFile       *file);
FmbUser       *fmb_file_get_user                   (const FmbFile       *file);

const gchar      *fmb_file_get_content_type           (FmbFile             *file);
gboolean          fmb_file_load_content_type          (FmbFile             *file);
const gchar      *fmb_file_get_symlink_target         (const FmbFile       *file);
const gchar      *fmb_file_get_basename               (const FmbFile       *file) G_GNUC_CONST;
gboolean          fmb_file_is_symlink                 (const FmbFile       *file);
guint64           fmb_file_get_size                   (const FmbFile       *file);
GAppInfo         *fmb_file_get_default_handler        (const FmbFile       *file);
GFileType         fmb_file_get_kind                   (const FmbFile       *file) G_GNUC_PURE;
GFile            *fmb_file_get_target_location        (const FmbFile       *file);
FmbFileMode    fmb_file_get_mode                   (const FmbFile       *file);
gboolean          fmb_file_is_mounted                 (const FmbFile       *file);
gboolean          fmb_file_exists                     (const FmbFile       *file);
gboolean          fmb_file_is_directory               (const FmbFile       *file) G_GNUC_PURE;
gboolean          fmb_file_is_shortcut                (const FmbFile       *file) G_GNUC_PURE;
gboolean          fmb_file_is_mountable               (const FmbFile       *file) G_GNUC_PURE;
gboolean          fmb_file_is_local                   (const FmbFile       *file);
gboolean          fmb_file_is_parent                  (const FmbFile       *file,
                                                          const FmbFile       *child);
gboolean          fmb_file_is_gfile_ancestor          (const FmbFile       *file, 
                                                          GFile                  *ancestor);
gboolean          fmb_file_is_ancestor                (const FmbFile       *file, 
                                                          const FmbFile       *ancestor);
gboolean          fmb_file_is_executable              (const FmbFile       *file);
gboolean          fmb_file_is_writable                (const FmbFile       *file);
gboolean          fmb_file_is_hidden                  (const FmbFile       *file);
gboolean          fmb_file_is_home                    (const FmbFile       *file);
gboolean          fmb_file_is_regular                 (const FmbFile       *file) G_GNUC_PURE;
gboolean          fmb_file_is_trashed                 (const FmbFile       *file);
gboolean          fmb_file_is_desktop_file            (const FmbFile       *file,
                                                          gboolean               *is_secure);
const gchar      *fmb_file_get_display_name           (const FmbFile       *file) G_GNUC_CONST;

gchar            *fmb_file_get_deletion_date          (const FmbFile       *file,
                                                          FmbDateStyle         date_style) G_GNUC_MALLOC G_GNUC_WARN_UNUSED_RESULT;
const gchar      *fmb_file_get_original_path          (const FmbFile       *file);
guint32           fmb_file_get_item_count             (const FmbFile       *file);

gboolean          fmb_file_is_chmodable               (const FmbFile       *file);
gboolean          fmb_file_is_renameable              (const FmbFile       *file);
gboolean          fmb_file_can_be_trashed             (const FmbFile       *file);

GList            *fmb_file_get_emblem_names           (FmbFile              *file);
void              fmb_file_set_emblem_names           (FmbFile              *file,
                                                          GList                   *emblem_names);

const gchar      *fmb_file_get_custom_icon            (const FmbFile        *file);
gboolean          fmb_file_set_custom_icon            (FmbFile              *file,
                                                          const gchar             *custom_icon,
                                                          GError                 **error);

const gchar     *fmb_file_get_thumbnail_path          (FmbFile              *file);
FmbFileThumbState fmb_file_get_thumb_state         (const FmbFile        *file);
void             fmb_file_set_thumb_state             (FmbFile              *file, 
                                                          FmbFileThumbState     state);
GIcon            *fmb_file_get_preview_icon           (const FmbFile        *file);
GFilesystemPreviewType fmb_file_get_preview_type      (const FmbFile *file);
const gchar      *fmb_file_get_icon_name              (FmbFile              *file,
                                                          FmbFileIconState      icon_state,
                                                          GtkIconTheme            *icon_theme);

void              fmb_file_watch                      (FmbFile              *file);
void              fmb_file_unwatch                    (FmbFile              *file);

gboolean          fmb_file_reload                     (FmbFile              *file);
void              fmb_file_reload_idle                (FmbFile              *file);
void              fmb_file_reload_idle_unref          (FmbFile              *file);
void              fmb_file_reload_parent              (FmbFile              *file);

void              fmb_file_destroy                    (FmbFile              *file);


gint              fmb_file_compare_by_name            (const FmbFile        *file_a,
                                                          const FmbFile        *file_b,
                                                          gboolean                 case_sensitive) G_GNUC_PURE;

FmbFile       *fmb_file_cache_lookup               (const GFile             *file);
gchar            *fmb_file_cached_display_name        (const GFile             *file);


GList            *fmb_file_list_get_applications      (GList                  *file_list);
GList            *fmb_file_list_to_fmb_g_file_list (GList                  *file_list);

gboolean          fmb_file_is_desktop                 (const FmbFile *file);

/**
 * fmb_file_is_root:
 * @file : a #FmbFile.
 *
 * Checks whether @file refers to the root directory.
 *
 * Return value: %TRUE if @file is the root directory.
 **/
#define fmb_file_is_root(file) (fmb_g_file_is_root (fmb_file_get_file (file)))

/**
 * fmb_file_has_parent:
 * @file : a #FmbFile instance.
 *
 * Checks whether it is possible to determine the parent #FmbFile
 * for @file.
 *
 * Return value: whether @file has a parent.
 **/
#define fmb_file_has_parent(file) (!fmb_file_is_root (FMB_FILE ((file))))

/**
 * fmb_file_dup_uri:
 * @file : a #FmbFile instance.
 *
 * Returns the URI for the @file. The caller is responsible
 * to free the returned string when no longer needed.
 *
 * Return value: the URI for @file.
 **/
#define fmb_file_dup_uri(file) (g_file_get_uri (fmb_file_get_file (file)))

/**
 * fmb_file_has_uri_scheme:
 * @file       : a #FmbFile instance.
 * @uri_scheme : a URI scheme string.
 *
 * Checks whether the URI scheme of the file matches @uri_scheme.
 *
 * Return value: TRUE, if the schemes match, FALSE otherwise.
 **/
#define fmb_file_has_uri_scheme(file, uri_scheme) (g_file_has_uri_scheme (fmb_file_get_file (file), (uri_scheme)))

/**
 * fmb_file_changed:
 * @file : a #FmbFile instance.
 *
 * Emits the ::changed signal on @file. This function is meant to be called
 * by derived classes whenever they notice changes to the @file.
 **/
#define fmb_file_changed(file)                         \
G_STMT_START{                                             \
  fmbx_file_info_changed (FMBX_FILE_INFO ((file))); \
}G_STMT_END


G_END_DECLS;

#endif /* !__FMB_FILE_H__ */

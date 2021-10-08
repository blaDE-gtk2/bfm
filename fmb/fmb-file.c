/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2005-2007 Benedikt Meurer <benny@xfce.org>
 * Copyright (c) 2009-2011 Jannis Pohlmann <jannis@xfce.org>
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of 
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public 
 * License along with this program; if not, write to the Free 
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_MEMORY_H
#include <memory.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_TIME_H
#include <time.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <gio/gio.h>
#include <libbladeui/libbladeui.h>

#include <fmbx/fmbx.h>

#include <fmb/fmb-application.h>
#include <fmb/fmb-chooser-dialog.h>
#include <fmb/fmb-exec.h>
#include <fmb/fmb-file.h>
#include <fmb/fmb-file-monitor.h>
#include <fmb/fmb-gio-extensions.h>
#include <fmb/fmb-gobject-extensions.h>
#include <fmb/fmb-private.h>
#include <fmb/fmb-preferences.h>
#include <fmb/fmb-user.h>
#include <fmb/fmb-util.h>
#include <fmb/fmb-dialogs.h>
#include <fmb/fmb-icon-factory.h>



/* Dump the file cache every X second, set to 0 to disable */
#define DUMP_FILE_CACHE 0



/* Signal identifiers */
enum
{
  DESTROY,
  LAST_SIGNAL,
};



static void               fmb_file_info_init                (FmbxFileInfoIface   *iface);
static void               fmb_file_dispose                  (GObject                *object);
static void               fmb_file_finalize                 (GObject                *object);
static gchar             *fmb_file_info_get_name            (FmbxFileInfo        *file_info);
static gchar             *fmb_file_info_get_uri             (FmbxFileInfo        *file_info);
static gchar             *fmb_file_info_get_parent_uri      (FmbxFileInfo        *file_info);
static gchar             *fmb_file_info_get_uri_scheme      (FmbxFileInfo        *file_info);
static gchar             *fmb_file_info_get_mime_type       (FmbxFileInfo        *file_info);
static gboolean           fmb_file_info_has_mime_type       (FmbxFileInfo        *file_info,
                                                                const gchar            *mime_type);
static gboolean           fmb_file_info_is_directory        (FmbxFileInfo        *file_info);
static GFileInfo         *fmb_file_info_get_file_info       (FmbxFileInfo        *file_info);
static GFileInfo         *fmb_file_info_get_filesystem_info (FmbxFileInfo        *file_info);
static GFile             *fmb_file_info_get_location        (FmbxFileInfo        *file_info);
static void               fmb_file_info_changed             (FmbxFileInfo        *file_info);
static gboolean           fmb_file_denies_access_permission (const FmbFile       *file,
                                                                FmbFileMode          usr_permissions,
                                                                FmbFileMode          grp_permissions,
                                                                FmbFileMode          oth_permissions);
static void               fmb_file_monitor                  (GFileMonitor           *monitor,
                                                                GFile                  *path,
                                                                GFile                  *other_path,
                                                                GFileMonitorEvent       event_type,
                                                                gpointer                user_data);
static void               fmb_file_watch_reconnect          (FmbFile             *file);
static gboolean           fmb_file_load                     (FmbFile             *file,
                                                                GCancellable           *cancellable,
                                                                GError                **error);
static gboolean           fmb_file_is_readable              (const FmbFile       *file);
static gboolean           fmb_file_same_filesystem          (const FmbFile       *file_a,
                                                                const FmbFile       *file_b);



G_LOCK_DEFINE_STATIC (file_cache_mutex);
G_LOCK_DEFINE_STATIC (file_content_type_mutex);
G_LOCK_DEFINE_STATIC (file_rename_mutex);



static FmbUserManager *user_manager;
static GHashTable        *file_cache;
static guint32            effective_user_id;
static GQuark             fmb_file_watch_quark;
static guint              file_signals[LAST_SIGNAL];



#define FLAG_SET_THUMB_STATE(file,new_state) G_STMT_START{ (file)->flags = ((file)->flags & ~FMB_FILE_FLAG_THUMB_MASK) | (new_state); }G_STMT_END
#define FLAG_GET_THUMB_STATE(file)           ((file)->flags & FMB_FILE_FLAG_THUMB_MASK)
#define FLAG_SET(file,flag)                  G_STMT_START{ ((file)->flags |= (flag)); }G_STMT_END
#define FLAG_UNSET(file,flag)                G_STMT_START{ ((file)->flags &= ~(flag)); }G_STMT_END
#define FLAG_IS_SET(file,flag)               (((file)->flags & (flag)) != 0)

#define DEFAULT_CONTENT_TYPE "application/octet-stream"



typedef enum
{
  FMB_FILE_FLAG_THUMB_MASK     = 0x03,   /* storage for FmbFileThumbState */
  FMB_FILE_FLAG_IN_DESTRUCTION = 1 << 2, /* for avoiding recursion during destroy */
  FMB_FILE_FLAG_IS_MOUNTED     = 1 << 3, /* whether this file is mounted */
}
FmbFileFlags;

struct _FmbFileClass
{
  GObjectClass __parent__;

  /* signals */
  void (*destroy) (FmbFile *file);
};

struct _FmbFile
{
  GObject __parent__;

  /* storage for the file information */
  GFileInfo            *info;
  GFileType             kind;
  GFile                *gfile;
  gchar                *content_type;
  gchar                *icon_name;

  gchar                *custom_icon_name;
  gchar                *display_name;
  gchar                *basename;
  gchar                *thumbnail_path;

  /* sorting */
  gchar                *collate_key;
  gchar                *collate_key_nocase;

  /* flags for thumbnail state etc */
  FmbFileFlags       flags;
  
  /* tells whether the file watch is not set */
  gboolean              no_file_watch;
};

typedef struct
{
  GFileMonitor  *monitor;
  guint          watch_count;
}
FmbFileWatch;

typedef struct
{
  FmbFileGetFunc  func;
  gpointer           user_data;
  GCancellable      *cancellable;
}
FmbFileGetData;

static struct
{
  GUserDirectory  type;
  const gchar    *icon_name;
}
fmb_file_dirs[] =
{
  { G_USER_DIRECTORY_DESKTOP,      "user-desktop" },
  { G_USER_DIRECTORY_DOCUMENTS,    "folder-documents" },
  { G_USER_DIRECTORY_DOWNLOAD,     "folder-download" },
  { G_USER_DIRECTORY_MUSIC,        "folder-music" },
  { G_USER_DIRECTORY_PICTURES,     "folder-pictures" },
  { G_USER_DIRECTORY_PUBLIC_SHARE, "folder-publicshare" },
  { G_USER_DIRECTORY_TEMPLATES,    "folder-templates" },
  { G_USER_DIRECTORY_VIDEOS,       "folder-videos" }
};



G_DEFINE_TYPE_WITH_CODE (FmbFile, fmb_file, G_TYPE_OBJECT,
    G_IMPLEMENT_INTERFACE (FMBX_TYPE_FILE_INFO, fmb_file_info_init))


static GWeakRef*
weak_ref_new (GObject *obj)
{
  GWeakRef *ref;
  ref = g_slice_new (GWeakRef);
  g_weak_ref_init (ref, obj);
  return ref;
}

static void
weak_ref_free (GWeakRef *ref)
{
  g_weak_ref_clear (ref);
  g_slice_free (GWeakRef, ref);
}


#ifdef G_ENABLE_DEBUG
#ifdef HAVE_ATEXIT
static gboolean fmb_file_atexit_registered = FALSE;



static void
fmb_file_atexit_foreach (gpointer key,
                            gpointer value,
                            gpointer user_data)
{
  gchar *uri;

  uri = g_file_get_uri (key);
  g_print ("--> %s\n", uri);
  if (G_OBJECT (key)->ref_count > 2)
    g_print ("    GFile (%u)\n", G_OBJECT (key)->ref_count - 2);
  g_free (uri);
}



static void
fmb_file_atexit (void)
{
  G_LOCK (file_cache_mutex);

  if (file_cache == NULL || g_hash_table_size (file_cache) == 0)
    {
      G_UNLOCK (file_cache_mutex);
      return;
    }

  g_print ("--- Leaked a total of %u FmbFile objects:\n",
           g_hash_table_size (file_cache));

  g_hash_table_foreach (file_cache, fmb_file_atexit_foreach, NULL);

  g_print ("\n");

  G_UNLOCK (file_cache_mutex);
}
#endif
#endif



#if DUMP_FILE_CACHE
static void
fmb_file_cache_dump_foreach (gpointer gfile,
                                gpointer value,
                                gpointer user_data)
{
  gchar *name;

  name = g_file_get_parse_name (G_FILE (gfile));
  g_print ("    %s\n", name);
  g_free (name);
}



static gboolean
fmb_file_cache_dump (gpointer user_data)
{
  G_LOCK (file_cache_mutex);

  if (file_cache != NULL)
    {
      g_print ("--- %d FmbFile objects in cache:\n",
               g_hash_table_size (file_cache));

      g_hash_table_foreach (file_cache, fmb_file_cache_dump_foreach, NULL);

      g_print ("\n");
    }

  G_UNLOCK (file_cache_mutex);

  return TRUE;
}
#endif



static void
fmb_file_class_init (FmbFileClass *klass)
{
  GObjectClass *gobject_class;

#ifdef G_ENABLE_DEBUG
#ifdef HAVE_ATEXIT
  if (G_UNLIKELY (!fmb_file_atexit_registered))
    {
      atexit ((void (*)(void)) fmb_file_atexit);
      fmb_file_atexit_registered = TRUE;
    }
#endif
#endif

#if DUMP_FILE_CACHE
  g_timeout_add_seconds (DUMP_FILE_CACHE, fmb_file_cache_dump, NULL);
#endif

  /* pre-allocate the required quarks */
  fmb_file_watch_quark = g_quark_from_static_string ("fmb-file-watch");

  /* grab a reference on the user manager */
  user_manager = fmb_user_manager_get_default ();

  /* determine the effective user id of the process */
  effective_user_id = geteuid ();

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->dispose = fmb_file_dispose;
  gobject_class->finalize = fmb_file_finalize;

  /**
   * FmbFile::destroy:
   * @file : the #FmbFile instance.
   *
   * Emitted when the system notices that the @file
   * was destroyed.
   **/
  file_signals[DESTROY] =
    g_signal_new (I_("destroy"),
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_CLEANUP | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (FmbFileClass, destroy),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);
}



static void
fmb_file_init (FmbFile *file)
{
}



static void
fmb_file_info_init (FmbxFileInfoIface *iface)
{
  iface->get_name = fmb_file_info_get_name;
  iface->get_uri = fmb_file_info_get_uri;
  iface->get_parent_uri = fmb_file_info_get_parent_uri;
  iface->get_uri_scheme = fmb_file_info_get_uri_scheme;
  iface->get_mime_type = fmb_file_info_get_mime_type;
  iface->has_mime_type = fmb_file_info_has_mime_type;
  iface->is_directory = fmb_file_info_is_directory;
  iface->get_file_info = fmb_file_info_get_file_info;
  iface->get_filesystem_info = fmb_file_info_get_filesystem_info;
  iface->get_location = fmb_file_info_get_location;
  iface->changed = fmb_file_info_changed;
}



static void
fmb_file_dispose (GObject *object)
{
  FmbFile *file = FMB_FILE (object);

  /* check that we don't recurse here */
  if (!FLAG_IS_SET (file, FMB_FILE_FLAG_IN_DESTRUCTION))
    {
      /* emit the "destroy" signal */
      FLAG_SET (file, FMB_FILE_FLAG_IN_DESTRUCTION);
      g_signal_emit (object, file_signals[DESTROY], 0);
      FLAG_UNSET (file, FMB_FILE_FLAG_IN_DESTRUCTION);
    }

  (*G_OBJECT_CLASS (fmb_file_parent_class)->dispose) (object);
}



static void
fmb_file_finalize (GObject *object)
{
  FmbFile *file = FMB_FILE (object);

  /* verify that nobody's watching the file anymore */
#ifdef G_ENABLE_DEBUG
  FmbFileWatch *file_watch = g_object_get_qdata (G_OBJECT (file), fmb_file_watch_quark);
  if (file_watch != NULL)
    {
      g_error ("Attempt to finalize a FmbFile, which has an active "
               "watch count of %d", file_watch->watch_count);
    }
#endif

  /* drop the entry from the cache */
  G_LOCK (file_cache_mutex);
  g_hash_table_remove (file_cache, file->gfile);
  G_UNLOCK (file_cache_mutex);

  /* release file info */
  if (file->info != NULL)
    g_object_unref (file->info);

  /* free the custom icon name */
  g_free (file->custom_icon_name);

  /* content type info */
  g_free (file->content_type);
  g_free (file->icon_name);

  /* free display name and basename */
  g_free (file->display_name);
  g_free (file->basename);

  /* free collate keys */
  if (file->collate_key_nocase != file->collate_key)
    g_free (file->collate_key_nocase);
  g_free (file->collate_key);

  /* free the thumbnail path */
  g_free (file->thumbnail_path);

  /* release file */
  g_object_unref (file->gfile);

  (*G_OBJECT_CLASS (fmb_file_parent_class)->finalize) (object);
}



static gchar *
fmb_file_info_get_name (FmbxFileInfo *file_info)
{
  return g_strdup (fmb_file_get_basename (FMB_FILE (file_info)));
}



static gchar*
fmb_file_info_get_uri (FmbxFileInfo *file_info)
{
  return fmb_file_dup_uri (FMB_FILE (file_info));
}



static gchar*
fmb_file_info_get_parent_uri (FmbxFileInfo *file_info)
{
  GFile *parent;
  gchar *uri = NULL;

  parent = g_file_get_parent (FMB_FILE (file_info)->gfile);
  if (G_LIKELY (parent != NULL))
    {
      uri = g_file_get_uri (parent);
      g_object_unref (parent);
    }

  return uri;
}



static gchar*
fmb_file_info_get_uri_scheme (FmbxFileInfo *file_info)
{
  return g_file_get_uri_scheme (FMB_FILE (file_info)->gfile);
}



static gchar*
fmb_file_info_get_mime_type (FmbxFileInfo *file_info)
{
  return g_strdup (fmb_file_get_content_type (FMB_FILE (file_info)));
}



static gboolean
fmb_file_info_has_mime_type (FmbxFileInfo *file_info,
                                const gchar     *mime_type)
{
  if (FMB_FILE (file_info)->info == NULL)
    return FALSE;

  return g_content_type_is_a (fmb_file_get_content_type (FMB_FILE (file_info)), mime_type);
}



static gboolean
fmb_file_info_is_directory (FmbxFileInfo *file_info)
{
  return fmb_file_is_directory (FMB_FILE (file_info));
}



static GFileInfo *
fmb_file_info_get_file_info (FmbxFileInfo *file_info)
{
  _fmb_return_val_if_fail (FMB_IS_FILE (file_info), NULL);
  
  if (FMB_FILE (file_info)->info != NULL)
    return g_object_ref (FMB_FILE (file_info)->info);
  else
    return NULL;
}



static GFileInfo *
fmb_file_info_get_filesystem_info (FmbxFileInfo *file_info)
{
  _fmb_return_val_if_fail (FMB_IS_FILE (file_info), NULL);

  return g_file_query_filesystem_info (FMB_FILE (file_info)->gfile, 
                                       FMBX_FILESYSTEM_INFO_NAMESPACE,
                                       NULL, NULL);
}



static GFile *
fmb_file_info_get_location (FmbxFileInfo *file_info)
{
  _fmb_return_val_if_fail (FMB_IS_FILE (file_info), NULL);
  return g_object_ref (FMB_FILE (file_info)->gfile);
}



static void
fmb_file_info_changed (FmbxFileInfo *file_info)
{
  FmbFile *file = FMB_FILE (file_info);

  _fmb_return_if_fail (FMB_IS_FILE (file_info));

  /* set the new thumbnail state manually, so we only emit file
   * changed once */
  FLAG_SET_THUMB_STATE (file, FMB_FILE_THUMB_STATE_UNKNOWN);

  /* tell the file monitor that this file changed */
  fmb_file_monitor_file_changed (file);
}



static gboolean
fmb_file_denies_access_permission (const FmbFile *file,
                                      FmbFileMode    usr_permissions,
                                      FmbFileMode    grp_permissions,
                                      FmbFileMode    oth_permissions)
{
  FmbFileMode mode;
  FmbGroup   *group;
  FmbUser    *user;
  gboolean       result;
  GList         *groups;
  GList         *lp;

  /* query the file mode */
  mode = fmb_file_get_mode (file);

  /* query the owner of the file, if we cannot determine
   * the owner, we can't tell if we're denied to access
   * the file, so we simply return FALSE then.
   */
  user = fmb_file_get_user (file);
  if (G_UNLIKELY (user == NULL))
    return FALSE;

  /* root is allowed to do everything */
  if (G_UNLIKELY (effective_user_id == 0))
    return FALSE;

  if (fmb_user_is_me (user))
    {
      /* we're the owner, so the usr permissions must be granted */
      result = ((mode & usr_permissions) == 0);
  
      /* release the user */
      g_object_unref (G_OBJECT (user));
    }
  else
    {
      group = fmb_file_get_group (file);
      if (G_LIKELY (group != NULL))
        {
          /* release the file owner */
          g_object_unref (G_OBJECT (user));

          /* determine the effective user */
          user = fmb_user_manager_get_user_by_id (user_manager, effective_user_id);
          if (G_LIKELY (user != NULL))
            {
              /* check the group permissions */
              groups = fmb_user_get_groups (user);
              for (lp = groups; lp != NULL; lp = lp->next)
                if (FMB_GROUP (lp->data) == group)
                  {
                    g_object_unref (G_OBJECT (user));
                    g_object_unref (G_OBJECT (group));
                    return ((mode & grp_permissions) == 0);
                  }
          
              /* release the effective user */
              g_object_unref (G_OBJECT (user));
            }

          /* release the file group */
          g_object_unref (G_OBJECT (group));
        }

      /* check other permissions */
      result = ((mode & oth_permissions) == 0);
    }

  return result;
}



static void
fmb_file_monitor_update (GFile             *path,
                            GFileMonitorEvent  event_type)
{
  FmbFile *file;

  _fmb_return_if_fail (G_IS_FILE (path));
  file = fmb_file_cache_lookup (path);
  if (G_LIKELY (file != NULL))
    {
      switch (event_type)
        {
        case G_FILE_MONITOR_EVENT_CREATED:
        case G_FILE_MONITOR_EVENT_ATTRIBUTE_CHANGED:
        case G_FILE_MONITOR_EVENT_CHANGES_DONE_HINT:
        case G_FILE_MONITOR_EVENT_PRE_UNMOUNT:
        case G_FILE_MONITOR_EVENT_DELETED:
          fmb_file_reload (file);
          break;

        default:
          break;
        }

      g_object_unref (file);
    }
}



static void
fmb_file_move_thumbnail_cache_file (GFile *old_file,
                                       GFile *new_file)
{
  FmbApplication    *application;
  FmbThumbnailCache *thumbnail_cache;

  _fmb_return_if_fail (G_IS_FILE (old_file));
  _fmb_return_if_fail (G_IS_FILE (new_file));

  application = fmb_application_get ();
  thumbnail_cache = fmb_application_get_thumbnail_cache (application);
  fmb_thumbnail_cache_move_file (thumbnail_cache, old_file, new_file);

  g_object_unref (thumbnail_cache);
  g_object_unref (application);
}



static void
fmb_file_monitor_moved (FmbFile *file,
                           GFile      *renamed_file)
{
  GFile *previous_file;

  /* ref the old location */
  previous_file = g_object_ref (G_OBJECT (file->gfile));

  /* notify the thumbnail cache that we can now also move the thumbnail */
  fmb_file_move_thumbnail_cache_file (previous_file, renamed_file);

  /* set the new file */
  file->gfile = g_object_ref (G_OBJECT (renamed_file));

  /* reload file information */
  fmb_file_load (file, NULL, NULL);

  /* need to re-register the monitor handle for the new uri */
  fmb_file_watch_reconnect (file);

  G_LOCK (file_cache_mutex);

  /* drop the previous entry from the cache */
  g_hash_table_remove (file_cache, previous_file);

  /* drop the reference on the previous file */
  g_object_unref (previous_file);

  /* insert the new entry */
  g_hash_table_insert (file_cache,
                       g_object_ref (file->gfile),
                       weak_ref_new (G_OBJECT (file)));

  G_UNLOCK (file_cache_mutex);
}



void
fmb_file_reload_parent (FmbFile *file)
{
  FmbFile *parent = NULL;

  _fmb_return_if_fail (FMB_IS_FILE (file));

  if (fmb_file_has_parent (file))
    {
      GFile *parent_file;

      /* only reload file if it is in cache */
      parent_file = g_file_get_parent (file->gfile);
      parent = fmb_file_cache_lookup (parent_file);
      g_object_unref (parent_file);
    }

  if (parent)
    {
      fmb_file_reload (parent);
      g_object_unref (parent);
    }
}



static void
fmb_file_monitor (GFileMonitor     *monitor,
                     GFile            *event_path,
                     GFile            *other_path,
                     GFileMonitorEvent event_type,
                     gpointer          user_data)
{
  FmbFile *file = FMB_FILE (user_data);
  FmbFile *other_file;

  _fmb_return_if_fail (G_IS_FILE_MONITOR (monitor));
  _fmb_return_if_fail (G_IS_FILE (event_path));
  _fmb_return_if_fail (FMB_IS_FILE (file));

  if (g_file_equal (event_path, file->gfile))
    {
      /* the event occurred for the monitored FmbFile */
      if (event_type == G_FILE_MONITOR_EVENT_MOVED)
        {
          G_LOCK (file_rename_mutex);
          fmb_file_monitor_moved (file, other_path);
          G_UNLOCK (file_rename_mutex);
          return;
        }

      if (G_LIKELY (event_path))
          fmb_file_monitor_update (event_path, event_type);
    }
  else
    {
      /* The event did not occur for the monitored FmbFile, but for
         a file that is contained in FmbFile which is actually a
         directory. */
      if (event_type == G_FILE_MONITOR_EVENT_MOVED)
        {
          /* reload the target file if cached */
          if (other_path == NULL)
            return;

          G_LOCK (file_rename_mutex);

          other_file = fmb_file_cache_lookup (other_path);
          if (other_file)
              fmb_file_reload (other_file);
          else
              other_file = fmb_file_get (other_path, NULL);

          if (other_file != NULL)
            {
              /* notify the thumbnail cache that we can now also move the thumbnail */
              fmb_file_move_thumbnail_cache_file (event_path, other_path);

              /* reload the containing target folder */
              fmb_file_reload_parent (other_file);

              g_object_unref (other_file);
            }

          G_UNLOCK (file_rename_mutex);
        }
      return;
    }
}


static void
fmb_file_watch_destroyed (gpointer data)
{
  FmbFileWatch *file_watch = data;

  if (G_LIKELY (file_watch->monitor != NULL))
    {
      g_file_monitor_cancel (file_watch->monitor);
      g_object_unref (file_watch->monitor);
    }

  g_slice_free (FmbFileWatch, file_watch);
}



static void
fmb_file_watch_reconnect (FmbFile *file)
{
  FmbFileWatch *file_watch;

  /* recreate the monitor without changing the watch_count for file renames */
  file_watch = g_object_get_qdata (G_OBJECT (file), fmb_file_watch_quark);
  if (file_watch != NULL)
    {
      /* reset the old monitor */
      if (G_LIKELY (file_watch->monitor != NULL))
        {
          g_file_monitor_cancel (file_watch->monitor);
          g_object_unref (file_watch->monitor);
        }

      /* create a file or directory monitor */
      file_watch->monitor = g_file_monitor (file->gfile, G_FILE_MONITOR_WATCH_MOUNTS | G_FILE_MONITOR_SEND_MOVED, NULL, NULL);
      if (G_LIKELY (file_watch->monitor != NULL))
        {
          /* watch monitor for file changes */
          g_signal_connect (file_watch->monitor, "changed", G_CALLBACK (fmb_file_monitor), file);
        }
    }
}



static void
fmb_file_set_emblem_names_ready (GObject      *source_object,
                                    GAsyncResult *result,
                                    gpointer      user_data)
{
  FmbFile *file = FMB_FILE (user_data);
  GError     *error = NULL;

  if (!g_file_set_attributes_finish (G_FILE (source_object), result, NULL, &error))
    {
      g_warning ("Failed to set metadata: %s", error->message);
      g_error_free (error);

      g_file_info_remove_attribute (file->info, "metadata::emblems");
    }

  fmb_file_changed (file);
}



static void
fmb_file_info_clear (FmbFile *file)
{
  _fmb_return_if_fail (FMB_IS_FILE (file));
  
  /* release the current file info */
  if (file->info != NULL)
    {
      g_object_unref (file->info);
      file->info = NULL;
    }

  /* unset */
  file->kind = G_FILE_TYPE_UNKNOWN;

  /* free the custom icon name */
  g_free (file->custom_icon_name);
  file->custom_icon_name = NULL;

  /* free display name and basename */
  g_free (file->display_name);
  file->display_name = NULL;

  g_free (file->basename);
  file->basename = NULL;

  /* content type */
  g_free (file->content_type);
  file->content_type = NULL;
  g_free (file->icon_name);
  file->icon_name = NULL;

  /* free collate keys */
  if (file->collate_key_nocase != file->collate_key)
    g_free (file->collate_key_nocase);
  file->collate_key_nocase = NULL;

  g_free (file->collate_key);
  file->collate_key = NULL;

  /* free thumbnail path */
  g_free (file->thumbnail_path);
  file->thumbnail_path = NULL;

  /* assume the file is mounted by default */
  FLAG_SET (file, FMB_FILE_FLAG_IS_MOUNTED);

  /* set thumb state to unknown */
  FLAG_SET_THUMB_STATE (file, FMB_FILE_THUMB_STATE_UNKNOWN);
}



static void
fmb_file_info_reload (FmbFile   *file,
                         GCancellable *cancellable)
{
  const gchar *target_uri;
  GKeyFile    *key_file;
  gchar       *p;
  const gchar *display_name;
  gboolean     is_secure = FALSE;
  gchar       *casefold;
  gchar       *path;

  _fmb_return_if_fail (FMB_IS_FILE (file));
  _fmb_return_if_fail (file->info == NULL || G_IS_FILE_INFO (file->info));

  if (G_LIKELY (file->info != NULL))
    {
      /* this is requested so often, cache it */
      file->kind = g_file_info_get_file_type (file->info);

      if (file->kind == G_FILE_TYPE_MOUNTABLE)
        {
          target_uri = g_file_info_get_attribute_string (file->info, G_FILE_ATTRIBUTE_STANDARD_TARGET_URI);
          if (target_uri != NULL
              && !g_file_info_get_attribute_boolean (file->info, G_FILE_ATTRIBUTE_MOUNTABLE_CAN_MOUNT))
            FLAG_SET (file, FMB_FILE_FLAG_IS_MOUNTED);
          else
             FLAG_UNSET (file, FMB_FILE_FLAG_IS_MOUNTED);
        }
    }

  /* determine the basename */
  file->basename = g_file_get_basename (file->gfile);
  _fmb_assert (file->basename != NULL);

  /* problematic files with content type reading */
  if (strcmp (file->basename, "kmsg") == 0
      && g_file_is_native (file->gfile))
    {
      path = g_file_get_path (file->gfile);
      if (g_strcmp0 (path, "/proc/kmsg") == 0)
        file->content_type = g_strdup (DEFAULT_CONTENT_TYPE);
      g_free (path);
    }

  /* check if this file is a desktop entry */
  if (fmb_file_is_desktop_file (file, &is_secure) && is_secure)
    {
      /* determine the custom icon and display name for .desktop files */

      /* query a key file for the .desktop file */
      key_file = fmb_g_file_query_key_file (file->gfile, cancellable, NULL);
      if (key_file != NULL)
        {
          /* read the icon name from the .desktop file */
          file->custom_icon_name = g_key_file_get_string (key_file, 
                                                          G_KEY_FILE_DESKTOP_GROUP,
                                                          G_KEY_FILE_DESKTOP_KEY_ICON,
                                                          NULL);

          if (G_UNLIKELY (blxo_str_is_empty (file->custom_icon_name)))
            {
              /* make sure we set null if the string is empty else the assertion in 
               * fmb_icon_factory_lookup_icon() will fail */
              g_free (file->custom_icon_name);
              file->custom_icon_name = NULL;
            }
          else
            {
              /* drop any suffix (e.g. '.png') from themed icons */
              if (!g_path_is_absolute (file->custom_icon_name))
                {
                  p = strrchr (file->custom_icon_name, '.');
                  if (p != NULL)
                    *p = '\0';
                }
            }

          /* read the display name from the .desktop file (will be overwritten later
           * if it's undefined here) */
          file->display_name = g_key_file_get_locale_string (key_file,
                                                             G_KEY_FILE_DESKTOP_GROUP,
                                                             G_KEY_FILE_DESKTOP_KEY_NAME,
                                                             NULL, NULL);
          
          /* drop the name if it's empty or has invalid encoding */
          if (blxo_str_is_empty (file->display_name)
              || !g_utf8_validate (file->display_name, -1, NULL))
            {
              g_free (file->display_name);
              file->display_name = NULL;
            }

          /* free the key file */
          g_key_file_free (key_file);
        }
    }

  /* determine the display name */
  if (file->display_name == NULL)
    {
      if (G_UNLIKELY (fmb_g_file_is_trash (file->gfile)))
        file->display_name = g_strdup (_("Trash"));
      else if (G_LIKELY (file->info != NULL))
        {
          display_name = g_file_info_get_display_name (file->info);
          if (G_LIKELY (display_name != NULL))
            {
              if (strcmp (display_name, "/") == 0)
                file->display_name = g_strdup (_("File System"));
              else
                file->display_name = g_strdup (display_name);
            }
        }

      /* fall back to a name for the gfile */
      if (file->display_name == NULL)
        file->display_name = fmb_g_file_get_display_name (file->gfile);
    }

  /* create case sensitive collation key */
  file->collate_key = g_utf8_collate_key_for_filename (file->display_name, -1);

  /* lowercase the display name */
  casefold = g_utf8_casefold (file->display_name, -1);

  /* if the lowercase name is equal, only peek the already hash key */
  if (casefold != NULL && strcmp (casefold, file->display_name) != 0)
    file->collate_key_nocase = g_utf8_collate_key_for_filename (casefold, -1);
  else
    file->collate_key_nocase = file->collate_key;

  /* cleanup */
  g_free (casefold);
}



static void
fmb_file_get_async_finish (GObject      *object,
                              GAsyncResult *result,
                              gpointer      user_data)
{
  FmbFileGetData *data = user_data;
  FmbFile        *file;
  GFileInfo         *file_info;
  GError            *error = NULL;
  GFile             *location = G_FILE (object);

  _fmb_return_if_fail (G_IS_FILE (location));
  _fmb_return_if_fail (G_IS_ASYNC_RESULT (result));

  /* finish querying the file information */
  file_info = g_file_query_info_finish (location, result, &error);

  /* allocate a new file object */
  file = g_object_new (FMB_TYPE_FILE, NULL);
  file->gfile = g_object_ref (location);

  /* reset the file */
  fmb_file_info_clear (file);

  /* set the file information */
  file->info = file_info;

  /* update the file from the information */
  fmb_file_info_reload (file, data->cancellable);

  /* update the mounted info */
  if (error != NULL
      && error->domain == G_IO_ERROR
      && error->code == G_IO_ERROR_NOT_MOUNTED)
   {
      FLAG_UNSET (file, FMB_FILE_FLAG_IS_MOUNTED);
      g_clear_error (&error);
   }

  /* insert the file into the cache */
  G_LOCK (file_cache_mutex);
  g_hash_table_insert (file_cache,
                       g_object_ref (file->gfile),
                       weak_ref_new (G_OBJECT (file)));
  G_UNLOCK (file_cache_mutex);

  /* pass the loaded file and possible errors to the return function */
  (data->func) (location, file, error, data->user_data);

  /* release the file, see description in FmbFileGetFunc */
  g_object_unref (file);

  /* free the error, if there is any */
  if (error != NULL)
    g_error_free (error);

  /* release the get data */
  if (data->cancellable != NULL)
    g_object_unref (data->cancellable);
  g_slice_free (FmbFileGetData, data);
}



/**
 * fmb_file_load:
 * @file        : a #FmbFile.
 * @cancellable : a #GCancellable.
 * @error       : return location for errors or %NULL.
 *
 * Loads all information about the file. As this is a possibly
 * blocking call, it can be cancelled using @cancellable. 
 *
 * If loading the file fails or the operation is cancelled,
 * @error will be set.
 *
 * Return value: %TRUE on success, %FALSE on error or interruption.
 **/
static gboolean
fmb_file_load (FmbFile   *file,
                  GCancellable *cancellable,
                  GError      **error)
{
  GError *err = NULL;

  _fmb_return_val_if_fail (FMB_IS_FILE (file), FALSE);
  _fmb_return_val_if_fail (error == NULL || *error == NULL, FALSE);
  _fmb_return_val_if_fail (cancellable == NULL || G_IS_CANCELLABLE (cancellable), FALSE);
  _fmb_return_val_if_fail (G_IS_FILE (file->gfile), FALSE);

  /* reset the file */
  fmb_file_info_clear (file);

  /* query a new file info */
  file->info = g_file_query_info (file->gfile,
                                  FMBX_FILE_INFO_NAMESPACE,
                                  G_FILE_QUERY_INFO_NONE,
                                  cancellable, &err);

  /* update the file from the information */
  fmb_file_info_reload (file, cancellable);

  /* update the mounted info */
  if (err != NULL
      && err->domain == G_IO_ERROR
      && err->code == G_IO_ERROR_NOT_MOUNTED)
   {
      FLAG_UNSET (file, FMB_FILE_FLAG_IS_MOUNTED);
      g_clear_error (&err);
   }

  if (err != NULL)
    {
      g_propagate_error (error, err);
      return FALSE;
    }
  else
    {
      return TRUE;
    }
}



/**
 * fmb_file_get:
 * @file  : a #GFile.
 * @error : return location for errors.
 *
 * Looks up the #FmbFile referred to by @file. This function may return a
 * FmbFile even though the file doesn't actually exist. This is the case
 * with remote URIs (like SFTP) for instance, if they are not mounted.
 *
 * The caller is responsible to call g_object_unref()
 * when done with the returned object.
 *
 * Return value: the #FmbFile for @file or %NULL on errors.
 **/
FmbFile*
fmb_file_get (GFile   *gfile,
                 GError **error)
{
  FmbFile *file;

  _fmb_return_val_if_fail (G_IS_FILE (gfile), NULL);

  /* check if we already have a cached version of that file */
  file = fmb_file_cache_lookup (gfile);
  if (G_UNLIKELY (file != NULL))
    {
      /* return the file, it already has an additional ref set
       * in fmb_file_cache_lookup */
    }
  else
    {
      /* allocate a new object */
      file = g_object_new (FMB_TYPE_FILE, NULL);
      file->gfile = g_object_ref (gfile);

      if (fmb_file_load (file, NULL, error))
        {
          /* setup lock until the file is inserted */
          G_LOCK (file_cache_mutex);

          /* insert the file into the cache */
          g_hash_table_insert (file_cache,
                               g_object_ref (file->gfile),
                               weak_ref_new (G_OBJECT (file)));

          /* done inserting in the cache */
          G_UNLOCK (file_cache_mutex);
        }
      else
        {
          /* failed loading, destroy the file */
          g_object_unref (file);

          /* make sure we return NULL */
          file = NULL;
        }
    }

  return file;
}


/**
 * fmb_file_get_with_info:
 * @uri         : an URI or an absolute filename.
 * @info        : #GFileInfo to use when loading the info.
 * @not_mounted : if the file is mounted.
 *
 * Looks up the #FmbFile referred to by @file. This function may return a
 * FmbFile even though the file doesn't actually exist. This is the case
 * with remote URIs (like SFTP) for instance, if they are not mounted.
 *
 * This function does not use g_file_query_info() to get the info,
 * but takes a reference on the @info,
 *
 * The caller is responsible to call g_object_unref()
 * when done with the returned object.
 *
 * Return value: the #FmbFile for @file or %NULL on errors.
 **/
FmbFile *
fmb_file_get_with_info (GFile     *gfile,
                           GFileInfo *info,
                           gboolean   not_mounted)
{
  FmbFile *file;

  _fmb_return_val_if_fail (G_IS_FILE (gfile), NULL);
  _fmb_return_val_if_fail (G_IS_FILE_INFO (info), NULL);

  /* check if we already have a cached version of that file */
  file = fmb_file_cache_lookup (gfile);
  if (G_UNLIKELY (file != NULL))
    {
      /* return the file, it already has an additional ref set
       * in fmb_file_cache_lookup */
    }
  else
    {
      /* allocate a new object */
      file = g_object_new (FMB_TYPE_FILE, NULL);
      file->gfile = g_object_ref (gfile);

      /* reset the file */
      fmb_file_info_clear (file);

      /* set the passed info */
      file->info = g_object_ref (info);

      /* update the file from the information */
      fmb_file_info_reload (file, NULL);

      /* update the mounted info */
      if (not_mounted)
        FLAG_UNSET (file, FMB_FILE_FLAG_IS_MOUNTED);

      /* setup lock until the file is inserted */
      G_LOCK (file_cache_mutex);

      /* insert the file into the cache */
      g_hash_table_insert (file_cache,
                           g_object_ref (file->gfile),
                           weak_ref_new (G_OBJECT (file)));

      /* done inserting in the cache */
      G_UNLOCK (file_cache_mutex);
    }

  return file;
}





/**
 * fmb_file_get_for_uri:
 * @uri   : an URI or an absolute filename.
 * @error : return location for errors or %NULL.
 *
 * Convenience wrapper function for fmb_file_get_for_path(), as its
 * often required to determine a #FmbFile for a given @uri.
 *
 * The caller is responsible to free the returned object using
 * g_object_unref() when no longer needed.
 *
 * Return value: the #FmbFile for the given @uri or %NULL if
 *               unable to determine.
 **/
FmbFile*
fmb_file_get_for_uri (const gchar *uri,
                         GError     **error)
{
  FmbFile *file;
  GFile      *path;

  _fmb_return_val_if_fail (uri != NULL, NULL);
  _fmb_return_val_if_fail (error == NULL || *error == NULL, NULL);

  path = g_file_new_for_commandline_arg (uri);
  file = fmb_file_get (path, error);
  g_object_unref (path);

  return file;
}



/**
 * fmb_file_get_async:
 **/
void
fmb_file_get_async (GFile            *location,
                       GCancellable     *cancellable,
                       FmbFileGetFunc func,
                       gpointer          user_data)
{
  FmbFile        *file;
  FmbFileGetData *data;

  _fmb_return_if_fail (G_IS_FILE (location));
  _fmb_return_if_fail (func != NULL);

  /* check if we already have a cached version of that file */
  file = fmb_file_cache_lookup (location);
  if (G_UNLIKELY (file != NULL))
    {
      /* call the return function with the file from the cache */
      (func) (location, file, NULL, user_data);
      g_object_unref (file);
    }
  else
    {
      /* allocate get data */
      data = g_slice_new0 (FmbFileGetData);
      data->user_data = user_data;
      data->func = func;
      if (cancellable != NULL)
        data->cancellable = g_object_ref (cancellable);

      /* load the file information asynchronously */
      g_file_query_info_async (location,
                               FMBX_FILE_INFO_NAMESPACE,
                               G_FILE_QUERY_INFO_NONE,
                               G_PRIORITY_DEFAULT,
                               cancellable,
                               fmb_file_get_async_finish,
                               data);
    }
}



/**
 * fmb_file_get_file:
 * @file : a #FmbFile instance.
 *
 * Returns the #GFile that refers to the location of @file.
 *
 * The returned #GFile is owned by @file and must not be released
 * with g_object_unref().
 * 
 * Return value: the #GFile corresponding to @file.
 **/
GFile *
fmb_file_get_file (const FmbFile *file)
{
  _fmb_return_val_if_fail (FMB_IS_FILE (file), NULL);
  _fmb_return_val_if_fail (G_IS_FILE (file->gfile), NULL);
  return file->gfile;
}



/**
 * fmb_file_get_info:
 * @file : a #FmbFile instance.
 *
 * Returns the #GFileInfo for @file.
 *
 * Note, that there's no reference taken for the caller on the
 * returned #GFileInfo, so if you need the object for a longer
 * perioud, you'll need to take a reference yourself using the
 * g_object_ref() method.
 *
 * Return value: the #GFileInfo for @file or %NULL.
 **/
GFileInfo *
fmb_file_get_info (const FmbFile *file)
{
  _fmb_return_val_if_fail (FMB_IS_FILE (file), NULL);
  _fmb_return_val_if_fail (file->info == NULL || G_IS_FILE_INFO (file->info), NULL);
  return file->info;
}



/**
 * fmb_file_get_parent:
 * @file  : a #FmbFile instance.
 * @error : return location for errors.
 *
 * Determines the parent #FmbFile for @file. If @file has no parent or
 * the user is not allowed to open the parent folder of @file, %NULL will
 * be returned and @error will be set to point to a #GError that
 * describes the cause. Else, the #FmbFile will be returned, and
 * the caller must call g_object_unref() on it.
 *
 * You may want to call fmb_file_has_parent() first to
 * determine whether @file has a parent.
 *
 * Return value: the parent #FmbFile or %NULL.
 **/
FmbFile*
fmb_file_get_parent (const FmbFile *file,
                        GError          **error)
{
  FmbFile *parent = NULL;
  GFile      *parent_file;

  _fmb_return_val_if_fail (FMB_IS_FILE (file), NULL);
  _fmb_return_val_if_fail (error == NULL || *error == NULL, NULL);

  parent_file = g_file_get_parent (file->gfile);

  if (parent_file == NULL)
    {
      g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_NOENT, _("The root folder has no parent"));
      return NULL;
    }

  parent = fmb_file_get (parent_file, error);
  g_object_unref (parent_file);

  return parent;
}



/**
 * fmb_file_check_loaded:
 * @file              : a #FmbFile instance.
 *
 * Check if @file has its information loaded, if not, try this once else
 * return %FALSE.
 *
 * Return value: %TRUE on success, else %FALSE.
 **/
gboolean
fmb_file_check_loaded (FmbFile *file)
{
  _fmb_return_val_if_fail (FMB_IS_FILE (file), FALSE);

  if (G_UNLIKELY (file->info == NULL))
    fmb_file_load (file, NULL, NULL);

  return (file->info != NULL);
}



/**
 * fmb_file_execute:
 * @file              : a #FmbFile instance.
 * @working_directory : the working directory used to resolve relative filenames 
 *                      in @file_list.
 * @parent            : %NULL, a #GdkScreen or #GtkWidget.
 * @file_list         : the list of #GFile<!---->s to supply to @file on execution.
 * @startup_id        : startup id for the new window (send over for dbus) or %NULL.
 * @error             : return location for errors or %NULL.
 *
 * Tries to execute @file on the specified @screen. If @file is executable
 * and could have been spawned successfully, %TRUE is returned, else %FALSE
 * will be returned and @error will be set to point to the error location.
 *
 * Return value: %TRUE on success, else %FALSE.
 **/
gboolean
fmb_file_execute (FmbFile  *file,
                     GFile       *working_directory,
                     gpointer     parent,
                     GList       *file_list,
                     const gchar *startup_id,
                     GError     **error)
{
  gboolean    snotify = FALSE;
  gboolean    terminal;
  gboolean    result = FALSE;
  GKeyFile   *key_file;
  GError     *err = NULL;
  GFile      *file_parent;
  gchar      *icon_name = NULL;
  gchar      *name;
  gchar      *type;
  gchar      *url;
  gchar      *location;
  gchar      *escaped_location;
  gchar     **argv = NULL;
  gchar      *exec;
  gchar      *directory = NULL;
  gboolean    is_secure = FALSE;
  guint32     stimestamp = 0;

  _fmb_return_val_if_fail (FMB_IS_FILE (file), FALSE);
  _fmb_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  location = fmb_g_file_get_location (file->gfile);

  if (fmb_file_is_desktop_file (file, &is_secure))
    {
      /* parse file first, even if it is insecure */
      key_file = fmb_g_file_query_key_file (file->gfile, NULL, &err);
      if (key_file == NULL)
        {
          g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_INVAL,
                       _("Failed to parse the desktop file: %s"), err->message);
          g_error_free (err);
          return FALSE;
        }

      type = g_key_file_get_string (key_file, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_TYPE, NULL);
      if (G_LIKELY (blxo_str_is_equal (type, "Application")))
        {
          exec = g_key_file_get_string (key_file, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_EXEC, NULL);
          if (G_LIKELY (exec != NULL))
            {
              /* if the .desktop file is not secure, ask user what to do */
              if (is_secure || fmb_dialogs_show_insecure_program (parent, _("Untrusted application launcher"), file, exec))
                {
                  /* parse other fields */
                  name = g_key_file_get_locale_string (key_file, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_NAME, NULL, NULL);
                  icon_name = g_key_file_get_string (key_file, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_ICON, NULL);
                  directory = g_key_file_get_string (key_file, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_PATH, NULL);
                  terminal = g_key_file_get_boolean (key_file, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_TERMINAL, NULL);
                  snotify = g_key_file_get_boolean (key_file, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_STARTUP_NOTIFY, NULL);

                  result = fmb_exec_parse (exec, file_list, icon_name, name, location, terminal, NULL, &argv, error);

                  g_free (name);
                }
              else
                {
                  /* fall-through to free value and leave without execution */
                  result = TRUE;
                }

              g_free (exec);
            }
          else
            {
              /* TRANSLATORS: `Exec' is a field name in a .desktop file. Don't translate it. */
              g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_INVAL,
                           _("No Exec field specified"));
            }
        }
      else if (blxo_str_is_equal (type, "Link"))
        {
          url = g_key_file_get_string (key_file, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_URL, NULL);
          if (G_LIKELY (url != NULL))
            {
              /* if the .desktop file is not secure, ask user what to do */
              if (is_secure || fmb_dialogs_show_insecure_program (parent, _("Untrusted link launcher"), file, url))
                {
                  /* pass the URL to the webbrowser, this could be a bit strange,
                   * but then at least we are on the secure side */
                  argv = g_new (gchar *, 3);
                  argv[0] = g_strdup ("blxo-open");
                  argv[1] = url;
                  argv[2] = NULL;
                }

              result = TRUE;
            }
          else
            {
              /* TRANSLATORS: `URL' is a field name in a .desktop file. Don't translate it. */
              g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_INVAL, 
                           _("No URL field specified"));
            }
        }
      else
        {
          g_set_error_literal (error, G_FILE_ERROR, G_FILE_ERROR_INVAL, _("Invalid desktop file"));
        }

      g_free (type);
      g_key_file_free (key_file);
    }
  else
    {
      /* fake the Exec line */
      escaped_location = g_shell_quote (location);
      exec = g_strconcat (escaped_location, " %F", NULL);
      result = fmb_exec_parse (exec, file_list, NULL, NULL, NULL, FALSE, NULL, &argv, error);
      g_free (escaped_location);
      g_free (exec);
    }

  if (G_LIKELY (result && argv != NULL))
    {
      /* use other directory if the Path from the desktop file was not set */
      if (G_LIKELY (directory == NULL))
        {
          /* determine the working directory */
          if (G_LIKELY (working_directory != NULL))
            {
              /* copy the working directory provided to this method */
              directory = g_file_get_path (working_directory);
            }
          else if (file_list != NULL)
            {
              /* use the directory of the first list item */
              file_parent = g_file_get_parent (file_list->data);
              directory = (file_parent != NULL) ? fmb_g_file_get_location (file_parent) : NULL;
              g_object_unref (file_parent);
            }
          else
            {
              /* use the directory of the executable file */
              parent = g_file_get_parent (file->gfile);
              directory = (parent != NULL) ? fmb_g_file_get_location (parent) : NULL;
              g_object_unref (parent);
            }
        }

      /* check if a startup id was passed (launch request over dbus) */
      if (startup_id != NULL && *startup_id != '\0')
        {
          /* parse startup_id string and extract timestamp
           * format: <unique>_TIME<timestamp>) */
          gchar *time_str = g_strrstr (startup_id, "_TIME");
          if (time_str != NULL)
            {
              gchar *end;

              /* ignore the "_TIME" part */
              time_str += 5;

              stimestamp = strtoul (time_str, &end, 0);
              if (end == time_str)
                stimestamp = 0;
            }
        }
      else
        {
          /* use current event time */
          stimestamp = gtk_get_current_event_time ();
        }

      /* execute the command */
      result = xfce_spawn_on_screen (fmb_util_parse_parent (parent, NULL), 
                                     directory, argv, NULL, G_SPAWN_SEARCH_PATH,
                                     snotify, stimestamp, icon_name, error);
    }

  /* clean up */
  g_strfreev (argv);
  g_free (location);
  g_free (directory);
  g_free (icon_name);

  return result;
}



/**
 * fmb_file_launch:
 * @file       : a #FmbFile instance.
 * @parent     : a #GtkWidget or a #GdkScreen on which to launch the @file.
 *               May also be %NULL in which case the default #GdkScreen will
 *               be used.
 * @startup_id : startup id for the new window (send over for dbus) or %NULL.
 * @error      : return location for errors or %NULL.
 *
 * If @file is an executable file, tries to execute it. Else if @file is
 * a directory, opens a new #FmbWindow to display the directory. Else,
 * the default handler for @file is determined and run.
 *
 * The @parent can be either a #GtkWidget or a #GdkScreen, on which to
 * launch the @file. If @parent is a #GtkWidget, the chooser dialog (if
 * no default application is available for @file) will be transient for
 * @parent. Else if @parent is a #GdkScreen it specifies the screen on
 * which to launch @file.
 *
 * Return value: %TRUE on success, else %FALSE.
 **/
gboolean
fmb_file_launch (FmbFile  *file,
                    gpointer     parent,
                    const gchar *startup_id,
                    GError     **error)
{
  GdkAppLaunchContext *context;
  FmbApplication   *application;
  GAppInfo            *app_info;
  gboolean             succeed;
  GList                path_list;
  GdkScreen           *screen;

  _fmb_return_val_if_fail (FMB_IS_FILE (file), FALSE);
  _fmb_return_val_if_fail (error == NULL || *error == NULL, FALSE);
  _fmb_return_val_if_fail (parent == NULL || GDK_IS_SCREEN (parent) || GTK_IS_WIDGET (parent), FALSE);

  screen = fmb_util_parse_parent (parent, NULL);

  /* check if we have a folder here */
  if (fmb_file_is_directory (file))
    {
      application = fmb_application_get ();
      fmb_application_open_window (application, file, screen, startup_id);
      g_object_unref (G_OBJECT (application));
      return TRUE;
    }

  /* check if we should execute the file */
  if (fmb_file_is_executable (file))
    return fmb_file_execute (file, NULL, parent, NULL, NULL, error);

  /* determine the default application to open the file */
  /* TODO We should probably add a cancellable argument to fmb_file_launch() */
  app_info = fmb_file_get_default_handler (FMB_FILE (file));

  /* display the application chooser if no application is defined for this file
   * type yet */
  if (G_UNLIKELY (app_info == NULL))
    {
      fmb_show_chooser_dialog (parent, file, TRUE);
      return TRUE;
    }

  /* HACK: check if we're not trying to launch another file manager again, possibly
   * ourselfs which will end in a loop */
  if (g_strcmp0 (g_app_info_get_id (app_info), "blxo-file-manager.desktop") == 0
      || g_strcmp0 (g_app_info_get_id (app_info), "Fmb.desktop") == 0
      || g_strcmp0 (g_app_info_get_name (app_info), "blxo-file-manager") == 0)
    {
      g_object_unref (G_OBJECT (app_info));
      fmb_show_chooser_dialog (parent, file, TRUE);
      return TRUE;
    }

  /* fake a path list */
  path_list.data = file->gfile;
  path_list.next = path_list.prev = NULL;

  /* create a launch context */
  context = gdk_app_launch_context_new ();
  gdk_app_launch_context_set_screen (context, screen);
  gdk_app_launch_context_set_timestamp (context, gtk_get_current_event_time ());

  /* otherwise try to execute the application */
  succeed = g_app_info_launch (app_info, &path_list, G_APP_LAUNCH_CONTEXT (context), error);

  /* destroy the launch context */
  g_object_unref (context);

  /* release the handler reference */
  g_object_unref (G_OBJECT (app_info));

  return succeed;
}



/**
 * fmb_file_rename:
 * @file  : a #FmbFile instance.
 * @name  : the new file name in UTF-8 encoding.
 * @error : return location for errors or %NULL.
 *
 * Tries to rename @file to the new @name. If @file cannot be renamed,
 * %FALSE will be returned and @error will be set accordingly. Else, if
 * the operation succeeds, %TRUE will be returned, and @file will have
 * a new URI and a new display name.
 *
 * When offering a rename action in the user interface, the implementation
 * should first check whether the file is available, using the
 * fmb_file_is_renameable() method.
 *
 * Return value: %TRUE on success, else %FALSE.
 **/
gboolean
fmb_file_rename (FmbFile   *file,
                    const gchar  *name,
                    GCancellable *cancellable,
                    gboolean      called_from_job,
                    GError      **error)
{
  GKeyFile             *key_file;
  GError               *err = NULL;
  GFile                *renamed_file;
  gboolean              is_secure;
  const gchar * const  *languages;
  guint                 i;
  gboolean              name_set = FALSE;

  _fmb_return_val_if_fail (FMB_IS_FILE (file), FALSE);
  _fmb_return_val_if_fail (g_utf8_validate (name, -1, NULL), FALSE);
  _fmb_return_val_if_fail (cancellable == NULL || G_IS_CANCELLABLE (cancellable), FALSE);
  _fmb_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  /* check if this file is a desktop entry */
  if (fmb_file_is_desktop_file (file, &is_secure)
      && is_secure)
    {
      /* try to load the desktop entry into a key file */
      key_file = fmb_g_file_query_key_file (file->gfile, cancellable, &err);
      if (key_file == NULL)
        {
          g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_INVAL,
                       _("Failed to parse the desktop file: %s"), err->message);
          g_error_free (err);
          return FALSE;
        }

      /* check if we can set the language name */
      languages = g_get_language_names ();
      if (languages != NULL)
        {
          for (i = 0; !name_set && languages[i] != NULL; i++)
            {
              /* skip C language */
              if (g_ascii_strcasecmp (languages[i], "C") == 0)
                continue;

              /* change the translated Name field of the desktop entry */
              g_key_file_set_locale_string (key_file, G_KEY_FILE_DESKTOP_GROUP,
                                            G_KEY_FILE_DESKTOP_KEY_NAME,
                                            languages[i], name);

              /* done */
              name_set = TRUE;
            }
        }

      if (!name_set)
        {
          /* change the Name field of the desktop entry */
          g_key_file_set_string (key_file, G_KEY_FILE_DESKTOP_GROUP,
                                 G_KEY_FILE_DESKTOP_KEY_NAME, name);
        }

      /* write the changes back to the file */
      if (fmb_g_file_write_key_file (file->gfile, key_file, cancellable, &err))
        {
          /* reload file information */
          fmb_file_load (file, NULL, NULL);

          if (!called_from_job)
            {
              /* tell the associated folder that the file was renamed */
              fmbx_file_info_renamed (FMBX_FILE_INFO (file));

              /* notify everybody that the file has changed */
              fmb_file_changed (file);
            }

          /* release the key file and return with success */
          g_key_file_free (key_file);
          return TRUE;
        }
      else
        {
          /* propagate the error message and return with failure */
          g_propagate_error (error, err);
          g_key_file_free (key_file);
          return FALSE;
        }
    }
  else
    {
      G_LOCK (file_rename_mutex);
      /* try to rename the file */
      renamed_file = g_file_set_display_name (file->gfile, name, cancellable, error);

      /* check if we succeeded */
      if (renamed_file != NULL)
        {
          /* notify the file is renamed */
          fmb_file_monitor_moved (file, renamed_file);

          g_object_unref (G_OBJECT (renamed_file));

          if (!called_from_job)
            {
              /* emit the file changed signal */
              fmb_file_changed (file);
            }
          G_UNLOCK (file_rename_mutex);
          return TRUE;
        }
      else
        {
          G_UNLOCK (file_rename_mutex);
          return FALSE;
        }
    }
}



/**
 * fmb_file_accepts_drop:
 * @file                    : a #FmbFile instance.
 * @file_list               : the list of #GFile<!---->s that will be droppped.
 * @context                 : the current #GdkDragContext, which is used for the drop.
 * @suggested_action_return : return location for the suggested #GdkDragAction or %NULL.
 *
 * Checks whether @file can accept @path_list for the given @context and
 * returns the #GdkDragAction<!---->s that can be used or 0 if no actions
 * apply.
 *
 * If any #GdkDragAction<!---->s apply and @suggested_action_return is not
 * %NULL, the suggested #GdkDragAction for this drop will be stored to the
 * location pointed to by @suggested_action_return.
 *
 * Return value: the #GdkDragAction<!---->s supported for the drop or
 *               0 if no drop is possible.
 **/
GdkDragAction
fmb_file_accepts_drop (FmbFile     *file,
                          GList          *file_list,
                          GdkDragContext *context,
                          GdkDragAction  *suggested_action_return)
{
  GdkDragAction suggested_action;
  GdkDragAction actions;
  FmbFile   *ofile;
  GFile        *parent_file;
  GList        *lp;
  guint         n;

  _fmb_return_val_if_fail (FMB_IS_FILE (file), 0);
  _fmb_return_val_if_fail (GDK_IS_DRAG_CONTEXT (context), 0);

  /* we can never drop an empty list */
  if (G_UNLIKELY (file_list == NULL))
    return 0;

  /* default to whatever GTK+ thinks for the suggested action */
  suggested_action = context->suggested_action;

  /* check if we have a writable directory here or an executable file */
  if (fmb_file_is_directory (file) && fmb_file_is_writable (file))
    {
      /* determine the possible actions */
      actions = context->actions & (GDK_ACTION_COPY | GDK_ACTION_MOVE | GDK_ACTION_LINK | GDK_ACTION_ASK);

      /* cannot create symbolic links in the trash or copy to the trash */
      if (fmb_file_is_trashed (file))
        actions &= ~(GDK_ACTION_COPY | GDK_ACTION_LINK);

      /* check up to 100 of the paths (just in case somebody tries to
       * drag around his music collection with 5000 files).
       */
      for (lp = file_list, n = 0; lp != NULL && n < 100; lp = lp->next, ++n)
        {
          /* we cannot drop a file on itself */
          if (G_UNLIKELY (g_file_equal (file->gfile, lp->data)))
            return 0;

          /* check whether source and destination are the same */
          parent_file = g_file_get_parent (lp->data);
          if (G_LIKELY (parent_file != NULL))
            {
              if (g_file_equal (file->gfile, parent_file))
                {
                  g_object_unref (parent_file);
                  return 0;
                }
              else
                g_object_unref (parent_file);
            }

          /* copy/move/link within the trash not possible */
          if (G_UNLIKELY (fmb_g_file_is_trashed (lp->data) && fmb_file_is_trashed (file)))
            return 0;
        }

      /* if the source offers both copy and move and the GTK+ suggested action is copy, try to be smart telling whether
       * we should copy or move by default by checking whether the source and target are on the same disk.
       */
      if ((actions & (GDK_ACTION_COPY | GDK_ACTION_MOVE)) != 0 
          && (suggested_action == GDK_ACTION_COPY))
        {
          /* default to move as suggested action */
          suggested_action = GDK_ACTION_MOVE;

          /* check for up to 100 files, for the reason state above */
          for (lp = file_list, n = 0; lp != NULL && n < 100; lp = lp->next, ++n)
            {
              /* dropping from the trash always suggests move */
              if (G_UNLIKELY (fmb_g_file_is_trashed (lp->data)))
                break;

              /* determine the cached version of the source file */
              ofile = fmb_file_cache_lookup (lp->data);

              /* fallback to non-cached version */
              if (ofile == NULL)
                ofile = fmb_file_get (lp->data, NULL);

              /* we have only move if we know the source and both the source and the target
               * are on the same disk, and the source file is owned by the current user.
               */
              if (ofile == NULL 
                  || !fmb_file_same_filesystem (file, ofile)
                  || (ofile->info != NULL 
                      && g_file_info_get_attribute_uint32 (ofile->info, 
                                                           G_FILE_ATTRIBUTE_UNIX_UID) != effective_user_id))
                {
                  /* default to copy and get outa here */
                  suggested_action = GDK_ACTION_COPY;
                  break;
                }

              if (ofile != NULL)
                g_object_unref (ofile);
            }
        }
    }
  else if (fmb_file_is_executable (file))
    {
      /* determine the possible actions */
      actions = context->actions & (GDK_ACTION_COPY | GDK_ACTION_MOVE | GDK_ACTION_LINK | GDK_ACTION_PRIVATE);
    }
  else
    return 0;

  /* determine the preferred action based on the context */
  if (G_LIKELY (suggested_action_return != NULL))
    {
      /* determine a working action */
      if (G_LIKELY ((suggested_action & actions) != 0))
        *suggested_action_return = suggested_action;
      else if ((actions & GDK_ACTION_ASK) != 0)
        *suggested_action_return = GDK_ACTION_ASK;
      else if ((actions & GDK_ACTION_COPY) != 0)
        *suggested_action_return = GDK_ACTION_COPY;
      else if ((actions & GDK_ACTION_LINK) != 0)
        *suggested_action_return = GDK_ACTION_LINK;
      else if ((actions & GDK_ACTION_MOVE) != 0)
        *suggested_action_return = GDK_ACTION_MOVE;
      else
        *suggested_action_return = GDK_ACTION_PRIVATE;
    }

  /* yeppa, we can drop here */
  return actions;
}



/**
 * fmb_file_get_date:
 * @file        : a #FmbFile instance.
 * @date_type   : the kind of date you are interested in.
 *
 * Queries the given @date_type from @file and returns the result.
 *
 * Return value: the time for @file of the given @date_type.
 **/
guint64
fmb_file_get_date (const FmbFile  *file,
                      FmbFileDateType date_type)
{
  const gchar *attribute;

  _fmb_return_val_if_fail (FMB_IS_FILE (file), 0);

  if (file->info == NULL)
    return 0;
  
  switch (date_type)
    {
    case FMB_FILE_DATE_ACCESSED: 
      attribute = G_FILE_ATTRIBUTE_TIME_ACCESS;
      break;
    case FMB_FILE_DATE_CHANGED:
      attribute = G_FILE_ATTRIBUTE_TIME_CHANGED;
      break;
    case FMB_FILE_DATE_MODIFIED: 
      attribute = G_FILE_ATTRIBUTE_TIME_MODIFIED;
      break;
    default:
      _fmb_assert_not_reached ();
    }

  return g_file_info_get_attribute_uint64 (file->info, attribute);
}



/**
 * fmb_file_get_date_string:
 * @file       : a #FmbFile instance.
 * @date_type  : the kind of date you are interested to know about @file.
 * @date_style : the style used to format the date.
 *
 * Tries to determine the @date_type of @file, and if @file supports the
 * given @date_type, it'll be formatted as string and returned. The
 * caller is responsible for freeing the string using the g_free()
 * function.
 *
 * Return value: the @date_type of @file formatted as string.
 **/
gchar*
fmb_file_get_date_string (const FmbFile  *file,
                             FmbFileDateType date_type,
                             FmbDateStyle    date_style)
{
  return fmb_util_humanize_file_time (fmb_file_get_date (file, date_type), date_style);
}



/**
 * fmb_file_get_mode_string:
 * @file : a #FmbFile instance.
 *
 * Returns the mode of @file as text. You'll need to free
 * the result using g_free() when you're done with it.
 *
 * Return value: the mode of @file as string.
 **/
gchar*
fmb_file_get_mode_string (const FmbFile *file)
{
  FmbFileMode mode;
  GFileType      kind;
  gchar         *text;

  _fmb_return_val_if_fail (FMB_IS_FILE (file), NULL);

  kind = fmb_file_get_kind (file);
  mode = fmb_file_get_mode (file);
  text = g_new (gchar, 11);

  /* file type */
  /* TODO earlier versions of Fmb had 'P' for ports and
   * 'D' for doors. Do we still need those? */
  switch (kind)
    {
    case G_FILE_TYPE_SYMBOLIC_LINK: text[0] = 'l'; break;
    case G_FILE_TYPE_REGULAR:       text[0] = '-'; break;
    case G_FILE_TYPE_DIRECTORY:     text[0] = 'd'; break;
    case G_FILE_TYPE_SPECIAL:
    case G_FILE_TYPE_UNKNOWN:
    default:
      if (S_ISCHR (mode))
        text[0] = 'c';
      else if (S_ISSOCK (mode))
        text[0] = 's';
      else if (S_ISFIFO (mode))
        text[0] = 'f';
      else if (S_ISBLK (mode))
        text[0] = 'b';
      else
        text[0] = ' ';
    }

  /* permission flags */
  text[1] = (mode & FMB_FILE_MODE_USR_READ)  ? 'r' : '-';
  text[2] = (mode & FMB_FILE_MODE_USR_WRITE) ? 'w' : '-';
  text[3] = (mode & FMB_FILE_MODE_USR_EXEC)  ? 'x' : '-';
  text[4] = (mode & FMB_FILE_MODE_GRP_READ)  ? 'r' : '-';
  text[5] = (mode & FMB_FILE_MODE_GRP_WRITE) ? 'w' : '-';
  text[6] = (mode & FMB_FILE_MODE_GRP_EXEC)  ? 'x' : '-';
  text[7] = (mode & FMB_FILE_MODE_OTH_READ)  ? 'r' : '-';
  text[8] = (mode & FMB_FILE_MODE_OTH_WRITE) ? 'w' : '-';
  text[9] = (mode & FMB_FILE_MODE_OTH_EXEC)  ? 'x' : '-';

  /* special flags */
  if (G_UNLIKELY (mode & FMB_FILE_MODE_SUID))
    text[3] = 's';
  if (G_UNLIKELY (mode & FMB_FILE_MODE_SGID))
    text[6] = 's';
  if (G_UNLIKELY (mode & FMB_FILE_MODE_STICKY))
    text[9] = 't';

  text[10] = '\0';

  return text;
}



/**
 * fmb_file_get_size_string:
 * @file : a #FmbFile instance.
 *
 * Returns the size of the file as text in a human readable
 * format. You'll need to free the result using g_free()
 * if you're done with it.
 *
 * Return value: the size of @file in a human readable
 *               format.
 **/
gchar *
fmb_file_get_size_string (const FmbFile *file)
{
  _fmb_return_val_if_fail (FMB_IS_FILE (file), NULL);
  return g_format_size (fmb_file_get_size (file));
}



/**
 * fmb_file_get_size_string_formatted:
 * @file             : a #FmbFile instance.
 * @file_size_binary : indicates if file size format
 *                     should be binary or not.
 *
 * Returns the size of the file as text in a human readable
 * format in decimal or binary format. You'll need to free
 * the result using g_free() if you're done with it.
 *
 * Return value: the size of @file in a human readable
 *               format.
 **/
gchar *
fmb_file_get_size_string_formatted (const FmbFile *file, const gboolean file_size_binary)
{
  _fmb_return_val_if_fail (FMB_IS_FILE (file), NULL);
  return g_format_size_full (fmb_file_get_size (file),
                             file_size_binary ? G_FORMAT_SIZE_IEC_UNITS : G_FORMAT_SIZE_DEFAULT);
}



/**
 * fmb_file_get_volume:
 * @file           : a #FmbFile instance.
 *
 * Attempts to determine the #GVolume on which @file is located. If @file cannot 
 * determine it's volume, then %NULL will be returned. Else a #GVolume instance 
 * is returned which has to be released by the caller using g_object_unref().
 *
 * Return value: the #GVolume for @file or %NULL.
 **/
GVolume*
fmb_file_get_volume (const FmbFile *file)
{
  GVolume *volume = NULL;
  GMount  *mount;

  _fmb_return_val_if_fail (FMB_IS_FILE (file), NULL);

  /* TODO make this function call asynchronous */
  mount = g_file_find_enclosing_mount (file->gfile, NULL, NULL);
  if (mount != NULL)
    {
      volume = g_mount_get_volume (mount);
      g_object_unref (mount);
    }

  return volume;
}



/**
 * fmb_file_get_group:
 * @file : a #FmbFile instance.
 *
 * Determines the #FmbGroup for @file. If there's no
 * group associated with @file or if the system is unable to
 * determine the group, %NULL will be returned.
 *
 * The caller is responsible for freeing the returned object
 * using g_object_unref().
 *
 * Return value: the #FmbGroup for @file or %NULL.
 **/
FmbGroup *
fmb_file_get_group (const FmbFile *file)
{
  guint32 gid;

  _fmb_return_val_if_fail (FMB_IS_FILE (file), NULL);

  /* TODO what are we going to do on non-UNIX systems? */
  gid = g_file_info_get_attribute_uint32 (file->info,
                                          G_FILE_ATTRIBUTE_UNIX_GID);

  return fmb_user_manager_get_group_by_id (user_manager, gid);
}



/**
 * fmb_file_get_user:
 * @file : a #FmbFile instance.
 *
 * Determines the #FmbUser for @file. If there's no
 * user associated with @file or if the system is unable
 * to determine the user, %NULL will be returned.
 *
 * The caller is responsible for freeing the returned object
 * using g_object_unref().
 *
 * Return value: the #FmbUser for @file or %NULL.
 **/
FmbUser*
fmb_file_get_user (const FmbFile *file)
{
  guint32 uid;

  _fmb_return_val_if_fail (FMB_IS_FILE (file), NULL);

  /* TODO what are we going to do on non-UNIX systems? */
  uid = g_file_info_get_attribute_uint32 (file->info,
                                          G_FILE_ATTRIBUTE_UNIX_UID);

  return fmb_user_manager_get_user_by_id (user_manager, uid);
}



/**
 * fmb_file_get_content_type:
 * @file : a #FmbFile.
 *
 * Returns the content type of @file.
 *
 * Return value: content type of @file.
 **/
const gchar *
fmb_file_get_content_type (FmbFile *file)
{
  GFileInfo   *info;
  GError      *err = NULL;
  const gchar *content_type = NULL;

  _fmb_return_val_if_fail (FMB_IS_FILE (file), NULL);

  if (G_UNLIKELY (file->content_type == NULL))
    {
      G_LOCK (file_content_type_mutex);

      /* make sure we weren't waiting for a lock */
      if (G_UNLIKELY (file->content_type != NULL))
        goto bailout;

      /* make sure this is not loaded in the general info */
      _fmb_assert (file->info == NULL
          || !g_file_info_has_attribute (file->info, G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE));

      if (G_UNLIKELY (file->kind == G_FILE_TYPE_DIRECTORY))
        {
          /* this we known for sure */
          file->content_type = g_strdup ("inode/directory");
        }
      else
        {
          /* async load the content-type */
          info = g_file_query_info (file->gfile,
                                    G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE,
                                    G_FILE_QUERY_INFO_NONE,
                                    NULL, &err);

          if (G_LIKELY (info != NULL))
            {
              /* store the new content type */
              content_type = g_file_info_get_content_type (info);
              if (G_UNLIKELY (content_type != NULL))
                file->content_type = g_strdup (content_type);
              g_object_unref (G_OBJECT (info));
            }
          else
            {
              g_warning ("Content type loading failed for %s: %s",
                         fmb_file_get_display_name (file),
                         err->message);
              g_error_free (err);
            }

          /* always provide a fallback */
          if (file->content_type == NULL)
            file->content_type = g_strdup (DEFAULT_CONTENT_TYPE);
        }

      bailout:

      G_UNLOCK (file_content_type_mutex);
    }

  return file->content_type;
}



gboolean
fmb_file_load_content_type (FmbFile *file)
{
  _fmb_return_val_if_fail (FMB_IS_FILE (file), TRUE);

  if (file->content_type != NULL)
    return FALSE;

  fmb_file_get_content_type (file);

  return TRUE;
}



/**
 * fmb_file_get_symlink_target:
 * @file : a #FmbFile.
 *
 * Returns the path of the symlink target or %NULL if the @file
 * is not a symlink.
 *
 * Return value: path of the symlink target or %NULL.
 **/
const gchar *
fmb_file_get_symlink_target (const FmbFile *file)
{
  _fmb_return_val_if_fail (FMB_IS_FILE (file), NULL);

  if (file->info == NULL)
    return NULL;

  return g_file_info_get_symlink_target (file->info);
}



/**
 * fmb_file_get_basename:
 * @file : a #FmbFile.
 *
 * Returns the basename of the @file in UTF-8 encoding.
 *
 * Return value: UTF-8 encoded basename of the @file.
 **/
const gchar *
fmb_file_get_basename (const FmbFile *file)
{
  _fmb_return_val_if_fail (FMB_IS_FILE (file), NULL);
  return file->basename;
}



/**
 * fmb_file_is_symlink:
 * @file : a #FmbFile.
 *
 * Returns %TRUE if @file is a symbolic link.
 *
 * Return value: %TRUE if @file is a symbolic link.
 **/
gboolean 
fmb_file_is_symlink (const FmbFile *file) 
{
  _fmb_return_val_if_fail (FMB_IS_FILE (file), FALSE);

  if (file->info == NULL)
    return FALSE;

  return g_file_info_get_is_symlink (file->info);
}



/**
 * fmb_file_get_size:
 * @file : a #FmbFile instance.
 *
 * Tries to determine the size of @file in bytes and
 * returns the size.
 *
 * Return value: the size of @file in bytes.
 **/
guint64
fmb_file_get_size (const FmbFile *file)
{
  _fmb_return_val_if_fail (FMB_IS_FILE (file), 0);

  if (file->info == NULL)
    return 0;

  return g_file_info_get_size (file->info);
}



/**
 * fmb_file_get_default_handler:
 * @file : a #FmbFile instance.
 *
 * Returns the default #GAppInfo for @file or %NULL if there is none.
 * 
 * The caller is responsible to free the returned #GAppInfo using
 * g_object_unref().
 *
 * Return value: Default #GAppInfo for @file or %NULL if there is none.
 **/
GAppInfo *
fmb_file_get_default_handler (const FmbFile *file) 
{
  const gchar *content_type;
  GAppInfo    *app_info = NULL;
  gboolean     must_support_uris = FALSE;
  gchar       *path;

  _fmb_return_val_if_fail (FMB_IS_FILE (file), NULL);

  content_type = fmb_file_get_content_type (FMB_FILE (file));
  if (content_type != NULL)
    {
      path = g_file_get_path (file->gfile);
      must_support_uris = (path == NULL);
      g_free (path);

      app_info = g_app_info_get_default_for_type (content_type, must_support_uris);
    }

  if (app_info == NULL)
    app_info = g_file_query_default_handler (file->gfile, NULL, NULL);

  return app_info;
}



/**
 * fmb_file_get_kind:
 * @file : a #FmbFile instance.
 *
 * Returns the kind of @file.
 *
 * Return value: the kind of @file.
 **/
GFileType
fmb_file_get_kind (const FmbFile *file) 
{
  _fmb_return_val_if_fail (FMB_IS_FILE (file), G_FILE_TYPE_UNKNOWN);
  return file->kind;
}



GFile *
fmb_file_get_target_location (const FmbFile *file)
{
  const gchar *uri;

  _fmb_return_val_if_fail (FMB_IS_FILE (file), NULL);

  if (file->info == NULL)
    return g_object_ref (file->gfile);
  
  uri = g_file_info_get_attribute_string (file->info,
                                          G_FILE_ATTRIBUTE_STANDARD_TARGET_URI);

  return (uri != NULL) ? g_file_new_for_uri (uri) : NULL;
}



/**
 * fmb_file_get_mode:
 * @file : a #FmbFile instance.
 *
 * Returns the permission bits of @file.
 *
 * Return value: the permission bits of @file.
 **/
FmbFileMode
fmb_file_get_mode (const FmbFile *file) 
{
  _fmb_return_val_if_fail (FMB_IS_FILE (file), 0);

  if (file->info == NULL)
    return 0;

  if (g_file_info_has_attribute (file->info, G_FILE_ATTRIBUTE_UNIX_MODE))
    return g_file_info_get_attribute_uint32 (file->info, G_FILE_ATTRIBUTE_UNIX_MODE);
  else
    return fmb_file_is_directory (file) ? 0777 : 0666;
}



gboolean
fmb_file_is_mounted (const FmbFile *file)
{
  _fmb_return_val_if_fail (FMB_IS_FILE (file), FALSE);
  return FLAG_IS_SET (file, FMB_FILE_FLAG_IS_MOUNTED);
}



gboolean
fmb_file_exists (const FmbFile *file)
{
  _fmb_return_val_if_fail (FMB_IS_FILE (file), FALSE);
  return g_file_query_exists (file->gfile, NULL);
}



/**
 * fmb_file_is_directory:
 * @file : a #FmbFile instance.
 *
 * Checks whether @file refers to a directory.
 *
 * Return value: %TRUE if @file is a directory.
 **/
gboolean
fmb_file_is_directory (const FmbFile *file) 
{
  _fmb_return_val_if_fail (FMB_IS_FILE (file), FALSE);
  return file->kind == G_FILE_TYPE_DIRECTORY;
}



/**
 * fmb_file_is_shortcut:
 * @file : a #FmbFile instance.
 *
 * Checks whether @file refers to a shortcut to something else.
 *
 * Return value: %TRUE if @file is a shortcut.
 **/
gboolean
fmb_file_is_shortcut (const FmbFile *file) 
{
  _fmb_return_val_if_fail (FMB_IS_FILE (file), FALSE);
  return file->kind == G_FILE_TYPE_SHORTCUT;
}



/**
 * fmb_file_is_mountable:
 * @file : a #FmbFile instance.
 *
 * Checks whether @file refers to a mountable file/directory.
 *
 * Return value: %TRUE if @file is a mountable file/directory.
 **/
gboolean
fmb_file_is_mountable (const FmbFile *file) 
{
  _fmb_return_val_if_fail (FMB_IS_FILE (file), FALSE);
  return file->kind == G_FILE_TYPE_MOUNTABLE;
}



/**
 * fmb_file_is_local:
 * @file : a #FmbFile instance.
 *
 * Returns %TRUE if @file is a local file with the
 * file:// URI scheme.
 *
 * Return value: %TRUE if @file is local.
 **/
gboolean
fmb_file_is_local (const FmbFile *file)
{
  _fmb_return_val_if_fail (FMB_IS_FILE (file), FALSE);
  return g_file_has_uri_scheme (file->gfile, "file");
}



/**
 * fmb_file_is_parent:
 * @file  : a #FmbFile instance.
 * @child : another #FmbFile instance.
 *
 * Determines whether @file is the parent directory of @child.
 *
 * Return value: %TRUE if @file is the parent of @child.
 **/
gboolean
fmb_file_is_parent (const FmbFile *file,
                       const FmbFile *child)
{
  gboolean is_parent = FALSE;
  GFile   *parent;

  _fmb_return_val_if_fail (FMB_IS_FILE (file), FALSE);
  _fmb_return_val_if_fail (FMB_IS_FILE (child), FALSE);

  parent = g_file_get_parent (child->gfile);
  if (parent != NULL)
    {
      is_parent = g_file_equal (file->gfile, parent);
      g_object_unref (parent);
    }

  return is_parent;
}



/**
 * fmb_file_is_ancestor:
 * @file     : a #FmbFile instance.
 * @ancestor : another #GFile instance.
 *
 * Determines whether @file is somewhere inside @ancestor,
 * possibly with intermediate folders.
 *
 * Return value: %TRUE if @ancestor contains @file as a
 *               child, grandchild, great grandchild, etc.
 **/
gboolean
fmb_file_is_gfile_ancestor (const FmbFile *file,
                               GFile            *ancestor)
{
  GFile   *current = NULL;
  GFile   *tmp;

  _fmb_return_val_if_fail (FMB_IS_FILE (file), FALSE);
  _fmb_return_val_if_fail (G_IS_FILE (file->gfile), FALSE);
  _fmb_return_val_if_fail (G_IS_FILE (ancestor), FALSE);

  current = g_object_ref (file->gfile);
  while(TRUE)
    {
      tmp = g_file_get_parent (current);
      g_object_unref (current);
      current = tmp;

      if (current == NULL) /* parent of root is NULL */
        return FALSE;

      if (G_UNLIKELY (g_file_equal (current, ancestor)))
        {
          g_object_unref (current);
          return TRUE;
        }
    }

  return FALSE;
}



/**
 * fmb_file_is_ancestor:
 * @file     : a #FmbFile instance.
 * @ancestor : another #FmbFile instance.
 *
 * Determines whether @file is somewhere inside @ancestor,
 * possibly with intermediate folders.
 *
 * Return value: %TRUE if @ancestor contains @file as a
 *               child, grandchild, great grandchild, etc.
 **/
gboolean
fmb_file_is_ancestor (const FmbFile *file,
                         const FmbFile *ancestor)
{
  _fmb_return_val_if_fail (FMB_IS_FILE (file), FALSE);
  _fmb_return_val_if_fail (FMB_IS_FILE (ancestor), FALSE);

  return fmb_file_is_gfile_ancestor (file, ancestor->gfile);
}



/**
 * fmb_file_is_executable:
 * @file : a #FmbFile instance.
 *
 * Determines whether the owner of the current process is allowed
 * to execute the @file (or enter the directory refered to by
 * @file). On UNIX it also returns %TRUE if @file refers to a 
 * desktop entry.
 *
 * Return value: %TRUE if @file can be executed.
 **/
gboolean
fmb_file_is_executable (const FmbFile *file)
{
  FmbPreferences *preferences;
  gboolean           can_execute = FALSE;
  gboolean           exec_shell_scripts = FALSE;
  const gchar       *content_type;

  _fmb_return_val_if_fail (FMB_IS_FILE (file), FALSE);

  if (file->info == NULL)
    return FALSE;

  if (g_file_info_get_attribute_boolean (file->info, G_FILE_ATTRIBUTE_ACCESS_CAN_EXECUTE))
    {
      /* get the content type of the file */
      content_type = fmb_file_get_content_type (FMB_FILE (file));
      if (G_LIKELY (content_type != NULL))
        {
          can_execute = g_content_type_can_be_executable (content_type);

          if (can_execute)
            {
              /* check if the shell scripts should be executed or opened by default */
              preferences = fmb_preferences_get ();
              g_object_get (preferences, "misc-exec-shell-scripts-by-default", &exec_shell_scripts, NULL);
              g_object_unref (preferences);

              /* do never execute plain text files which are not shell scripts but marked executable */
              if (g_strcmp0 (content_type, "text/plain") == 0)
                  can_execute = FALSE;
              else if (g_content_type_is_a (content_type, "text/plain") && ! exec_shell_scripts)
                  can_execute = FALSE;
            }
        }
    }

  return can_execute || fmb_file_is_desktop_file (file, NULL);
}



/**
 * fmb_file_is_readable:
 * @file : a #FmbFile instance.
 *
 * Determines whether the owner of the current process is allowed
 * to read the @file.
 *
 * Return value: %TRUE if @file can be read.
 **/
static gboolean
fmb_file_is_readable (const FmbFile *file)
{
  _fmb_return_val_if_fail (FMB_IS_FILE (file), FALSE);

  if (file->info == NULL)
    return FALSE;

  if (!g_file_info_has_attribute (file->info, G_FILE_ATTRIBUTE_ACCESS_CAN_READ))
    return TRUE;
      
  return g_file_info_get_attribute_boolean (file->info, 
                                            G_FILE_ATTRIBUTE_ACCESS_CAN_READ);
}



/**
 * fmb_file_is_writable:
 * @file : a #FmbFile instance.
 *
 * Determines whether the owner of the current process is allowed
 * to write the @file.
 *
 * Return value: %TRUE if @file can be read.
 **/
gboolean
fmb_file_is_writable (const FmbFile *file)
{
  _fmb_return_val_if_fail (FMB_IS_FILE (file), FALSE);
  
  if (file->info == NULL)
    return FALSE;

  if (!g_file_info_has_attribute (file->info, G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE))
    return TRUE;

  return g_file_info_get_attribute_boolean (file->info, 
                                            G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE);
}



/**
 * fmb_file_is_hidden:
 * @file : a #FmbFile instance.
 *
 * Checks whether @file can be considered a hidden file.
 *
 * Return value: %TRUE if @file is a hidden file, else %FALSE.
 **/
gboolean
fmb_file_is_hidden (const FmbFile *file)
{
  _fmb_return_val_if_fail (FMB_IS_FILE (file), FALSE);
  
  if (file->info == NULL)
    return FALSE;

  return g_file_info_get_is_hidden (file->info)
         || g_file_info_get_is_backup (file->info);
}



/**
 * fmb_file_is_home:
 * @file : a #FmbFile.
 *
 * Checks whether @file refers to the users home directory.
 *
 * Return value: %TRUE if @file is the users home directory.
 **/
gboolean
fmb_file_is_home (const FmbFile *file)
{
  _fmb_return_val_if_fail (FMB_IS_FILE (file), FALSE);
  return fmb_g_file_is_home (file->gfile);
}



/**
 * fmb_file_is_regular:
 * @file : a #FmbFile.
 *
 * Checks whether @file refers to a regular file.
 *
 * Return value: %TRUE if @file is a regular file.
 **/
gboolean
fmb_file_is_regular (const FmbFile *file)
{
  _fmb_return_val_if_fail (FMB_IS_FILE (file), FALSE);
  return file->kind == G_FILE_TYPE_REGULAR;
}



/**
 * fmb_file_is_trashed:
 * @file : a #FmbFile instance.
 *
 * Returns %TRUE if @file is a local file that resides in 
 * the trash bin.
 *
 * Return value: %TRUE if @file is in the trash, or
 *               the trash folder itself.
 **/
gboolean
fmb_file_is_trashed (const FmbFile *file)
{
  _fmb_return_val_if_fail (FMB_IS_FILE (file), FALSE);
  return fmb_g_file_is_trashed (file->gfile);
}



/**
 * fmb_file_is_desktop_file:
 * @file      : a #FmbFile.
 * @is_secure : if %NULL do a simple check, else it will set this boolean
 *              to indicate if the desktop file is safe see bug #5012
 *              for more info.
 *
 * Returns %TRUE if @file is a .desktop file. The @is_secure return value
 * will tell if the .desktop file is also secure.
 *
 * Return value: %TRUE if @file is a .desktop file.
 **/
gboolean
fmb_file_is_desktop_file (const FmbFile *file,
                             gboolean         *is_secure)
{
  const gchar * const *data_dirs;
  guint                n;
  gchar               *path;

  _fmb_return_val_if_fail (FMB_IS_FILE (file), FALSE);

  if (file->info == NULL)
    return FALSE;

  /* only allow regular files with a .desktop extension */
  if (!g_str_has_suffix (file->basename, ".desktop")
      || file->kind != G_FILE_TYPE_REGULAR)
    return FALSE;

  /* don't check more if not needed */
  if (is_secure == NULL)
    return TRUE;

  /* desktop files outside xdg directories need to be executable for security reasons */
  if (g_file_info_get_attribute_boolean (file->info, G_FILE_ATTRIBUTE_ACCESS_CAN_EXECUTE))
    {
      /* has +x */
      *is_secure = TRUE;
    }
  else
    {
      /* assume the file is not safe */
      *is_secure = FALSE;

      /* deskopt files in xdg directories are also fine... */
      if (g_file_is_native (fmb_file_get_file (file)))
        {
          data_dirs = g_get_system_data_dirs ();
          if (G_LIKELY (data_dirs != NULL))
            {
              path = g_file_get_path (fmb_file_get_file (file));
              for (n = 0; data_dirs[n] != NULL; n++)
                {
                  if (g_str_has_prefix (path, data_dirs[n]))
                    {
                      /* has known prefix, can launch without problems */
                      *is_secure = TRUE;
                      break;
                    }
                }
              g_free (path);
            }
        }
    }

  return TRUE;
}



/**
 * fmb_file_get_display_name:
 * @file : a #FmbFile instance.
 *
 * Returns the @file name in the UTF-8 encoding, which is
 * suitable for displaying the file name in the GUI.
 *
 * Return value: the @file name suitable for display.
 **/
const gchar *
fmb_file_get_display_name (const FmbFile *file)
{
  _fmb_return_val_if_fail (FMB_IS_FILE (file), FALSE);
  return file->display_name;
}



/**
 * fmb_file_get_deletion_date:
 * @file       : a #FmbFile instance.
 * @date_style : the style used to format the date.
 *
 * Returns the deletion date of the @file if the @file
 * is located in the trash. Otherwise %NULL will be
 * returned.
 *
 * The caller is responsible to free the returned string
 * using g_free() when no longer needed.
 *
 * Return value: the deletion date of @file if @file is
 *               in the trash, %NULL otherwise.
 **/
gchar*
fmb_file_get_deletion_date (const FmbFile *file,
                               FmbDateStyle   date_style)
{
  const gchar *date;
  time_t       deletion_time;

  _fmb_return_val_if_fail (FMB_IS_FILE (file), NULL);
  _fmb_return_val_if_fail (G_IS_FILE_INFO (file->info), NULL);

  date = g_file_info_get_attribute_string (file->info, G_FILE_ATTRIBUTE_TRASH_DELETION_DATE);
  if (G_UNLIKELY (date == NULL))
    return NULL;

  /* try to parse the DeletionDate (RFC 3339 string) */
  deletion_time = fmb_util_time_from_rfc3339 (date);

  /* humanize the time value */
  return fmb_util_humanize_file_time (deletion_time, date_style);
}



/**
 * fmb_file_get_original_path:
 * @file : a #FmbFile instance.
 *
 * Returns the original path of the @file if the @file
 * is located in the trash. Otherwise %NULL will be
 * returned.
 *
 * Return value: the original path of @file if @file is
 *               in the trash, %NULL otherwise.
 **/
const gchar *
fmb_file_get_original_path (const FmbFile *file)
{
  _fmb_return_val_if_fail (FMB_IS_FILE (file), NULL);

  if (file->info == NULL)
    return NULL;

  return g_file_info_get_attribute_byte_string (file->info, G_FILE_ATTRIBUTE_TRASH_ORIG_PATH);
}



/**
 * fmb_file_get_item_count:
 * @file : a #FmbFile instance.
 *
 * Returns the number of items in the trash, if @file refers to the
 * trash root directory. Otherwise returns 0.
 *
 * Return value: number of files in the trash if @file is the trash 
 *               root dir, 0 otherwise.
 **/
guint32
fmb_file_get_item_count (const FmbFile *file)
{
  _fmb_return_val_if_fail (FMB_IS_FILE (file), 0);

  if (file->info == NULL)
    return 0;

  return g_file_info_get_attribute_uint32 (file->info, 
                                           G_FILE_ATTRIBUTE_TRASH_ITEM_COUNT);
}



/**
 * fmb_file_is_chmodable:
 * @file : a #FmbFile instance.
 *
 * Determines whether the owner of the current process is allowed
 * to changed the file mode of @file.
 *
 * Return value: %TRUE if the mode of @file can be changed.
 **/
gboolean
fmb_file_is_chmodable (const FmbFile *file)
{
  _fmb_return_val_if_fail (FMB_IS_FILE (file), FALSE);

  /* we can only change the mode if we the euid is
   *   a) equal to the file owner id
   * or
   *   b) the super-user id
   * and the file is not in the trash.
   */
  if (file->info == NULL)
    {
      return (effective_user_id == 0 && !fmb_file_is_trashed (file));
    }
  else
    {
      return ((effective_user_id == 0 
               || effective_user_id == g_file_info_get_attribute_uint32 (file->info,
                                                                         G_FILE_ATTRIBUTE_UNIX_UID))
              && !fmb_file_is_trashed (file));
    }
}



/**
 * fmb_file_is_renameable:
 * @file : a #FmbFile instance.
 *
 * Determines whether @file can be renamed using
 * #fmb_file_rename(). Note that the return
 * value is just a guess and #fmb_file_rename()
 * may fail even if this method returns %TRUE.
 *
 * Return value: %TRUE if @file can be renamed.
 **/
gboolean
fmb_file_is_renameable (const FmbFile *file)
{
  _fmb_return_val_if_fail (FMB_IS_FILE (file), FALSE);
  
  if (file->info == NULL)
    return FALSE;

  return g_file_info_get_attribute_boolean (file->info,
                                            G_FILE_ATTRIBUTE_ACCESS_CAN_RENAME);
}



gboolean
fmb_file_can_be_trashed (const FmbFile *file)
{
  _fmb_return_val_if_fail (FMB_IS_FILE (file), FALSE);

  if (file->info == NULL)
    return FALSE;

  return g_file_info_get_attribute_boolean (file->info, 
                                            G_FILE_ATTRIBUTE_ACCESS_CAN_TRASH);
}



/**
 * fmb_file_get_emblem_names:
 * @file : a #FmbFile instance.
 *
 * Determines the names of the emblems that should be displayed for
 * @file. The returned list is owned by the caller, but the list
 * items - the name strings - are owned by @file. So the caller
 * must call g_list_free(), but don't g_free() the list items.
 *
 * Note that the strings contained in the returned list are
 * not garantied to exist over the next iteration of the main
 * loop. So in case you need the list of emblem names for
 * a longer time, you'll need to take a copy of the strings.
 *
 * Return value: the names of the emblems for @file.
 **/
GList*
fmb_file_get_emblem_names (FmbFile *file)
{
  guint32   uid;
  gchar   **emblem_names;
  GList    *emblems = NULL;

  _fmb_return_val_if_fail (FMB_IS_FILE (file), NULL);

  /* leave if there is no info */
  if (file->info == NULL)
    return NULL;

  /* determine the custom emblems */
  emblem_names = g_file_info_get_attribute_stringv (file->info, "metadata::emblems");
  if (G_UNLIKELY (emblem_names != NULL))
    {
      for (; *emblem_names != NULL; ++emblem_names)
        emblems = g_list_append (emblems, *emblem_names);
    }

  if (fmb_file_is_symlink (file))
    emblems = g_list_prepend (emblems, FMB_FILE_EMBLEM_NAME_SYMBOLIC_LINK);

  /* determine the user ID of the file owner */
  /* TODO what are we going to do here on non-UNIX systems? */
  uid = file->info != NULL 
        ? g_file_info_get_attribute_uint32 (file->info, G_FILE_ATTRIBUTE_UNIX_UID) 
        : 0;

  /* we add "cant-read" if either (a) the file is not readable or (b) a directory, that lacks the
   * x-bit, see http://bugzilla.xfce.org/show_bug.cgi?id=1408 for the details about this change.
   */
  if (!fmb_file_is_readable (file)
      || (fmb_file_is_directory (file)
          && fmb_file_denies_access_permission (file, FMB_FILE_MODE_USR_EXEC,
                                                         FMB_FILE_MODE_GRP_EXEC,
                                                         FMB_FILE_MODE_OTH_EXEC)))
    {
      emblems = g_list_prepend (emblems, FMB_FILE_EMBLEM_NAME_CANT_READ);
    }
  else if (G_UNLIKELY (uid == effective_user_id && !fmb_file_is_writable (file)))
    {
      /* we own the file, but we cannot write to it, that's why we mark it as "cant-write", so
       * users won't be surprised when opening the file in a text editor, but are unable to save.
       */
      emblems = g_list_prepend (emblems, FMB_FILE_EMBLEM_NAME_CANT_WRITE);
    }

  return emblems;
}



/**
 * fmb_file_set_emblem_names:
 * @file         : a #FmbFile instance.
 * @emblem_names : a #GList of emblem names.
 *
 * Sets the custom emblem name list of @file to @emblem_names
 * and stores them in the @file<!---->s metadata.
 **/
void
fmb_file_set_emblem_names (FmbFile *file,
                              GList      *emblem_names)
{
  GList      *lp;
  gchar     **emblems = NULL;
  gint        n;
  GFileInfo  *info;

  _fmb_return_if_fail (FMB_IS_FILE (file));
  _fmb_return_if_fail (G_IS_FILE_INFO (file->info));

  /* allocate a zero-terminated array for the emblem names */
  emblems = g_new0 (gchar *, g_list_length (emblem_names) + 1);

  /* turn the emblem_names list into a zero terminated array */
  for (lp = emblem_names, n = 0; lp != NULL; lp = lp->next)
    {
      /* skip special emblems */
      if (strcmp (lp->data, FMB_FILE_EMBLEM_NAME_SYMBOLIC_LINK) == 0
          || strcmp (lp->data, FMB_FILE_EMBLEM_NAME_CANT_READ) == 0
          || strcmp (lp->data, FMB_FILE_EMBLEM_NAME_CANT_WRITE) == 0
          || strcmp (lp->data, FMB_FILE_EMBLEM_NAME_DESKTOP) == 0)
        continue;

      /* add the emblem to our list */
      emblems[n++] = g_strdup (lp->data);
    }

  /* set the value in the current info */
  if (n == 0)
    g_file_info_remove_attribute (file->info, "metadata::emblems");
  else
    g_file_info_set_attribute_stringv (file->info, "metadata::emblems", emblems);

  /* set meta data to the daemon */
  info = g_file_info_new ();
  g_file_info_set_attribute_stringv (info, "metadata::emblems", emblems);
  g_file_set_attributes_async (file->gfile, info,
                               G_FILE_QUERY_INFO_NONE,
                               G_PRIORITY_DEFAULT,
                               NULL,
                               fmb_file_set_emblem_names_ready,
                               file);
  g_object_unref (G_OBJECT (info));

  g_strfreev (emblems);
}



/**
 * fmb_file_set_custom_icon:
 * @file        : a #FmbFile instance.
 * @custom_icon : the new custom icon for the @file.
 * @error       : return location for errors or %NULL.
 *
 * Tries to change the custom icon of the .desktop file referred
 * to by @file. If that fails, %FALSE is returned and the
 * @error is set accordingly.
 *
 * Return value: %TRUE if the icon of @file was changed, %FALSE otherwise.
 **/
gboolean
fmb_file_set_custom_icon (FmbFile  *file,
                             const gchar *custom_icon,
                             GError     **error)
{
  GKeyFile *key_file;

  _fmb_return_val_if_fail (error == NULL || *error == NULL, FALSE);
  _fmb_return_val_if_fail (FMB_IS_FILE (file), FALSE);
  _fmb_return_val_if_fail (custom_icon != NULL, FALSE);

  key_file = fmb_g_file_query_key_file (file->gfile, NULL, error);

  if (key_file == NULL)
    return FALSE;

  g_key_file_set_string (key_file, G_KEY_FILE_DESKTOP_GROUP,
                         G_KEY_FILE_DESKTOP_KEY_ICON, custom_icon);

  if (fmb_g_file_write_key_file (file->gfile, key_file, NULL, error))
    {
      /* tell everybody that we have changed */
      fmb_file_changed (file);

      g_key_file_free (key_file);
      return TRUE;
    }
  else
    {
      g_key_file_free (key_file);
      return FALSE;
    }
}


/**
 * fmb_file_is_desktop:
 * @file : a #FmbFile.
 *
 * Checks whether @file refers to the users desktop directory.
 *
 * Return value: %TRUE if @file is the users desktop directory.
 **/
gboolean
fmb_file_is_desktop (const FmbFile *file)
{
  GFile   *desktop;
  gboolean is_desktop = FALSE;

  desktop = g_file_new_for_path (g_get_user_special_dir (G_USER_DIRECTORY_DESKTOP));
  is_desktop = g_file_equal (file->gfile, desktop);
  g_object_unref (desktop);

  return is_desktop;
}



const gchar *
fmb_file_get_thumbnail_path (FmbFile *file)
{
  GChecksum *checksum;
  gchar     *filename;
  gchar     *uri;

  _fmb_return_val_if_fail (FMB_IS_FILE (file), NULL);

  /* if the thumbstate is known to be not there, return null */
  if (fmb_file_get_thumb_state (file) == FMB_FILE_THUMB_STATE_NONE)
    return NULL;

  if (G_UNLIKELY (file->thumbnail_path == NULL))
    {
      checksum = g_checksum_new (G_CHECKSUM_MD5);
      if (G_LIKELY (checksum != NULL))
        {
          uri = fmb_file_dup_uri (file);
          g_checksum_update (checksum, (const guchar *) uri, strlen (uri));
          g_free (uri);

          filename = g_strconcat (g_checksum_get_string (checksum), ".png", NULL);
          g_checksum_free (checksum);

          /* The thumbnail is in the format/location
           * $XDG_CACHE_HOME/thumbnails/(nromal|large)/MD5_Hash_Of_URI.png
           * for version 0.8.0 if XDG_CACHE_HOME is defined, otherwise
           * /homedir/.thumbnails/(normal|large)/MD5_Hash_Of_URI.png
           * will be used, which is also always used for versions prior
           * to 0.7.0.
           */

          /* build and check if the thumbnail is in the new location */
          file->thumbnail_path = g_build_path ("/", g_get_user_cache_dir(),
                                               "thumbnails", "normal",
                                               filename, NULL);

          if (!g_file_test(file->thumbnail_path, G_FILE_TEST_EXISTS))
            {
              /* Fallback to old version */
              g_free(file->thumbnail_path);

              file->thumbnail_path = g_build_filename (xfce_get_homedir (), ".thumbnails",
                                                       "normal", filename, NULL);

              if(!g_file_test(file->thumbnail_path, G_FILE_TEST_EXISTS))
              {
                /* Thumbnail doesn't exist in either spot */
                g_free(file->thumbnail_path);
                file->thumbnail_path = NULL;
              }
            }

          g_free (filename);
        }
    }

  return file->thumbnail_path;
}



/**
 * fmb_file_get_thumb_state:
 * @file : a #FmbFile.
 *
 * Returns the current #FmbFileThumbState for @file. This
 * method is intended to be used by #FmbIconFactory only.
 *
 * Return value: the #FmbFileThumbState for @file.
 **/
FmbFileThumbState
fmb_file_get_thumb_state (const FmbFile *file)
{
  _fmb_return_val_if_fail (FMB_IS_FILE (file), FMB_FILE_THUMB_STATE_UNKNOWN);
  return FLAG_GET_THUMB_STATE (file);
}



/**
 * fmb_file_set_thumb_state:
 * @file        : a #FmbFile.
 * @thumb_state : the new #FmbFileThumbState.
 *
 * Sets the #FmbFileThumbState for @file to @thumb_state. 
 * This will cause a "file-changed" signal to be emitted from
 * #FmbFileMonitor. 
 **/ 
void
fmb_file_set_thumb_state (FmbFile          *file, 
                             FmbFileThumbState state)
{
  _fmb_return_if_fail (FMB_IS_FILE (file));

  /* check if the state changes */
  if (fmb_file_get_thumb_state (file) == state)
    return;

  /* set the new thumbnail state */
  FLAG_SET_THUMB_STATE (file, state);

  /* remove path if the type is not supported */
  if (state == FMB_FILE_THUMB_STATE_NONE
      && file->thumbnail_path != NULL)
    {
      g_free (file->thumbnail_path);
      file->thumbnail_path = NULL;
    }

  /* if the file has a thumbnail, reload it */
  if (state == FMB_FILE_THUMB_STATE_READY)
    fmb_file_monitor_file_changed (file);
}



/**
 * fmb_file_get_custom_icon:
 * @file : a #FmbFile instance.
 *
 * Queries the custom icon from @file if any, else %NULL is returned. 
 * The custom icon can be either a themed icon name or an absolute path
 * to an icon file in the local file system.
 *
 * Return value: the custom icon for @file or %NULL.
 **/
const gchar *
fmb_file_get_custom_icon (const FmbFile *file)
{
  _fmb_return_val_if_fail (FMB_IS_FILE (file), NULL);
  return file->custom_icon_name;
}



/**
 * fmb_file_get_preview_icon:
 * @file : a #FmbFile instance.
 *
 * Returns the preview icon for @file if any, else %NULL is returned.
 *
 * Return value: the custom icon for @file or %NULL, the GIcon is owner
 * by the file, so do not unref it.
 **/
GIcon *
fmb_file_get_preview_icon (const FmbFile *file)
{
  GObject *icon;

  _fmb_return_val_if_fail (FMB_IS_FILE (file), NULL);
  _fmb_return_val_if_fail (G_IS_FILE_INFO (file->info), NULL);

  icon = g_file_info_get_attribute_object (file->info, G_FILE_ATTRIBUTE_PREVIEW_ICON);
  if (G_LIKELY (icon != NULL))
    return G_ICON (icon);

  return NULL;
}



GFilesystemPreviewType
fmb_file_get_preview_type (const FmbFile *file)
{
  GFilesystemPreviewType  preview;
  GFileInfo              *info;

  _fmb_return_val_if_fail (FMB_IS_FILE (file), G_FILESYSTEM_PREVIEW_TYPE_NEVER);
  _fmb_return_val_if_fail (G_IS_FILE (file->gfile), G_FILESYSTEM_PREVIEW_TYPE_NEVER);

  info = g_file_query_filesystem_info (file->gfile, G_FILE_ATTRIBUTE_FILESYSTEM_USE_PREVIEW, NULL, NULL);
  if (G_LIKELY (info != NULL))
    {
      preview = g_file_info_get_attribute_uint32 (info, G_FILE_ATTRIBUTE_FILESYSTEM_USE_PREVIEW);
      g_object_unref (G_OBJECT (info));
    }
  else
    {
      /* assume we don't know */
      preview = G_FILESYSTEM_PREVIEW_TYPE_NEVER;
    }

  return preview;
}



static const gchar *
fmb_file_get_icon_name_for_state (const gchar         *icon_name,
                                     FmbFileIconState  icon_state)
{
  if (blxo_str_is_empty (icon_name))
    return NULL;

  /* check if we have an accept icon for the icon we found */
  if (icon_state != FMB_FILE_ICON_STATE_DEFAULT
      && (strcmp (icon_name, "inode-directory") == 0
          || strcmp (icon_name, "folder") == 0))
    {
      if (icon_state == FMB_FILE_ICON_STATE_DROP)
        return "folder-drag-accept";
      else if (icon_state == FMB_FILE_ICON_STATE_OPEN)
        return "folder-open";
    }

  return icon_name;
}



/**
 * fmb_file_get_icon_name:
 * @file       : a #FmbFile instance.
 * @icon_state : the state of the @file<!---->s icon we are interested in.
 * @icon_theme : the #GtkIconTheme on which to lookup up the icon name.
 *
 * Returns the name of the icon that can be used to present @file, based
 * on the given @icon_state and @icon_theme.
 *
 * Return value: the icon name for @file in @icon_theme.
 **/
const gchar *
fmb_file_get_icon_name (FmbFile          *file,
                           FmbFileIconState  icon_state,
                           GtkIconTheme        *icon_theme)
{
  GFile               *icon_file;
  GIcon               *icon = NULL;
  const gchar * const *names;
  gchar               *icon_name = NULL;
  gchar               *path;
  const gchar         *special_names[] = { NULL, "folder", NULL };
  guint                i;
  const gchar         *special_dir;
  GFileInfo           *fileinfo;

  _fmb_return_val_if_fail (FMB_IS_FILE (file), NULL);
  _fmb_return_val_if_fail (GTK_IS_ICON_THEME (icon_theme), NULL);

  /* return cached name */
  if (G_LIKELY (file->icon_name != NULL))
    return fmb_file_get_icon_name_for_state (file->icon_name, icon_state);

  /* the system root folder has a special icon */
  if (fmb_file_is_directory (file))
    {
      if (G_LIKELY (fmb_file_is_local (file)))
        {
          path = g_file_get_path (file->gfile);
          if (G_LIKELY (path != NULL))
            {
              if (strcmp (path, G_DIR_SEPARATOR_S) == 0)
                *special_names = "drive-harddisk";
              else if (strcmp (path, xfce_get_homedir ()) == 0)
                *special_names = "user-home";
              else
                {
                  for (i = 0; i < G_N_ELEMENTS (fmb_file_dirs); i++)
                    {
                      special_dir = g_get_user_special_dir (fmb_file_dirs[i].type);
                      if (special_dir != NULL
                          && strcmp (path, special_dir) == 0)
                        {
                          *special_names = fmb_file_dirs[i].icon_name;
                          break;
                        }
                    }
                }

              g_free (path);
            }
        }
      else if (!fmb_file_has_parent (file))
        {
          if (g_file_has_uri_scheme (file->gfile, "trash"))
            {
              special_names[0] = fmb_file_get_item_count (file) > 0 ? "user-trash-full" : "user-trash";
              special_names[1] = "user-trash";
            }
          else if (g_file_has_uri_scheme (file->gfile, "network"))
            {
              special_names[0] = "network-workgroup";
            }
          else if (g_file_has_uri_scheme (file->gfile, "recent"))
            {
              special_names[0] = "document-open-recent";
            }
          else if (g_file_has_uri_scheme (file->gfile, "computer"))
            {
              special_names[0] = "computer";
            }
        }

      if (*special_names != NULL)
        {
          names = special_names;
          goto check_names;
        }
    }
  else if (fmb_file_is_mountable (file)
           || g_file_has_uri_scheme (file->gfile, "network"))
    {
      /* query the icon (computer:// and network:// backend) */
      fileinfo = g_file_query_info (file->gfile,
                                    G_FILE_ATTRIBUTE_STANDARD_ICON,
                                    G_FILE_QUERY_INFO_NONE, NULL, NULL);
      if (G_LIKELY (fileinfo != NULL))
        {
          /* take the icon from the info */
          icon = g_file_info_get_icon (fileinfo);
          if (G_LIKELY (icon != NULL))
            g_object_ref (icon);

          /* release */
          g_object_unref (G_OBJECT (fileinfo));

          if (G_LIKELY (icon != NULL))
            goto check_icon;
        }
    }

  /* try again later */
  if (file->info == NULL)
    return NULL;

  /* lookup for content type, just like gio does for local files */
  icon = g_content_type_get_icon (fmb_file_get_content_type (file));
  if (G_LIKELY (icon != NULL))
    {
      check_icon:
      if (G_IS_THEMED_ICON (icon))
        {
          names = g_themed_icon_get_names (G_THEMED_ICON (icon));

          check_names:

          if (G_LIKELY (names != NULL))
            {
              for (i = 0; names[i] != NULL; ++i)
                if (*names[i] != '(' /* see gnome bug 688042 */
                    && gtk_icon_theme_has_icon (icon_theme, names[i]))
                  {
                    icon_name = g_strdup (names[i]);
                    break;
                  }
            }
        }
      else if (G_IS_FILE_ICON (icon))
        {
          icon_file = g_file_icon_get_file (G_FILE_ICON (icon));
          if (icon_file != NULL)
            icon_name = g_file_get_path (icon_file);
        }

      if (G_LIKELY (icon != NULL))
        g_object_unref (icon);
    }

  /* store new name, fallback to legacy names, or empty string to avoid recursion */
  g_free (file->icon_name);
  if (G_LIKELY (icon_name != NULL))
    file->icon_name = icon_name;
  else if (file->kind == G_FILE_TYPE_DIRECTORY
           && gtk_icon_theme_has_icon (icon_theme, "folder"))
    file->icon_name = g_strdup ("folder");
  else
    file->icon_name = g_strdup ("");

  return fmb_file_get_icon_name_for_state (file->icon_name, icon_state);
}



/**
 * fmb_file_watch:
 * @file : a #FmbFile instance.
 *
 * Tells @file to watch itself for changes. Not all #FmbFile
 * implementations must support this, but if a #FmbFile
 * implementation implements the fmb_file_watch() method,
 * it must also implement the fmb_file_unwatch() method.
 *
 * The #FmbFile base class implements automatic "ref
 * counting" for watches, that says, you can call fmb_file_watch()
 * multiple times, but the virtual method will be invoked only
 * once. This also means that you MUST call fmb_file_unwatch()
 * for every fmb_file_watch() invokation, else the application
 * will abort.
 **/
void
fmb_file_watch (FmbFile *file)
{
  FmbFileWatch *file_watch;
  GError          *error = NULL;

  _fmb_return_if_fail (FMB_IS_FILE (file));

  file_watch = g_object_get_qdata (G_OBJECT (file), fmb_file_watch_quark);
  if (file_watch == NULL)
    {
      file_watch = g_slice_new (FmbFileWatch);
      file_watch->watch_count = 1;

      /* create a file or directory monitor */
      file_watch->monitor = g_file_monitor (file->gfile, G_FILE_MONITOR_WATCH_MOUNTS | G_FILE_MONITOR_SEND_MOVED, NULL, &error);
      if (G_UNLIKELY (file_watch->monitor == NULL))
        {
          g_debug ("Failed to create file monitor: %s", error->message);
          g_error_free (error);
          file->no_file_watch = TRUE;
        }
      else
        {
          /* watch monitor for file changes */
          g_signal_connect (file_watch->monitor, "changed", G_CALLBACK (fmb_file_monitor), file);
        }

      /* attach to file */
      g_object_set_qdata_full (G_OBJECT (file), fmb_file_watch_quark, file_watch, fmb_file_watch_destroyed);
    }
  else if (G_LIKELY (!file->no_file_watch))
    {
      /* increase watch count */
      _fmb_return_if_fail (G_IS_FILE_MONITOR (file_watch->monitor));
      file_watch->watch_count++;
    }
}



/**
 * fmb_file_unwatch:
 * @file : a #FmbFile instance.
 *
 * See fmb_file_watch() for a description of how watching
 * #FmbFile<!---->s works.
 **/
void
fmb_file_unwatch (FmbFile *file)
{
  FmbFileWatch *file_watch;

  _fmb_return_if_fail (FMB_IS_FILE (file));

  if (G_UNLIKELY (file->no_file_watch))
    {
      return;
    }

  file_watch = g_object_get_qdata (G_OBJECT (file), fmb_file_watch_quark);
  if (file_watch != NULL)
    {
      /* remove if this was the last ref */
      if (--file_watch->watch_count == 0)
        g_object_set_qdata (G_OBJECT (file), fmb_file_watch_quark, NULL);
    }
  else
    {
      _fmb_assert_not_reached ();
    }
}



/**
 * fmb_file_reload:
 * @file : a #FmbFile instance.
 *
 * Tells @file to reload its internal state, e.g. by reacquiring
 * the file info from the underlying media.
 *
 * You must be able to handle the case that @file is
 * destroyed during the reload call.
 *
 * Return value: As this function can be used as a callback function
 * for fmb_file_reload_idle, it will always return FALSE to prevent
 * being called repeatedly.
 **/
gboolean
fmb_file_reload (FmbFile *file)
{
  _fmb_return_val_if_fail (FMB_IS_FILE (file), FALSE);

  /* clear file pxmap cache */
  fmb_icon_factory_clear_pixmap_cache (file);

  if (!fmb_file_load (file, NULL, NULL))
    {
      /* destroy the file if we cannot query any file information */
      fmb_file_destroy (file);
      return FALSE;
    }

  /* ... and tell others */
  fmb_file_changed (file);

  return FALSE;
}


 
/**
 * fmb_file_reload_idle:
 * @file : a #FmbFile instance.
 *
 * Schedules a reload of the @file by calling fmb_file_reload
 * when idle.
 *
 **/
void
fmb_file_reload_idle (FmbFile *file)
{
  _fmb_return_if_fail (FMB_IS_FILE (file));

  g_idle_add ((GSourceFunc) fmb_file_reload, file);
}



/**
 * fmb_file_reload_idle_unref:
 * @file : a #FmbFile instance.
 *
 * Schedules a reload of the @file by calling fmb_file_reload
 * when idle. When scheduled function returns @file object will be
 * unreferenced.
 *
 **/
void
fmb_file_reload_idle_unref (FmbFile *file)
{
  _fmb_return_if_fail (FMB_IS_FILE (file));

  g_idle_add_full (G_PRIORITY_DEFAULT_IDLE,
                   (GSourceFunc) fmb_file_reload,
                   file,
                   (GDestroyNotify) g_object_unref);
}



/**
 * fmb_file_destroy:
 * @file : a #FmbFile instance.
 *
 * Emits the ::destroy signal notifying all reference holders
 * that they should release their references to the @file.
 *
 * This method is very similar to what gtk_object_destroy()
 * does for #GtkObject<!---->s.
 **/
void
fmb_file_destroy (FmbFile *file)
{
  _fmb_return_if_fail (FMB_IS_FILE (file));

  if (!FLAG_IS_SET (file, FMB_FILE_FLAG_IN_DESTRUCTION))
    {
      /* take an additional reference on the file, as the file-destroyed
       * invocation may already release the last reference.
       */
      g_object_ref (G_OBJECT (file));

      /* tell the file monitor that this file was destroyed */
      fmb_file_monitor_file_destroyed (file);

      /* run the dispose handler */
      g_object_run_dispose (G_OBJECT (file));

      /* release our reference */
      g_object_unref (G_OBJECT (file));
    }
}



/**
 * fmb_file_compare_by_name:
 * @file_a         : the first #FmbFile.
 * @file_b         : the second #FmbFile.
 * @case_sensitive : whether the comparison should be case-sensitive.
 *
 * Compares @file_a and @file_b by their display names. If @case_sensitive
 * is %TRUE the comparison will be case-sensitive.
 *
 * Return value: -1 if @file_a should be sorted before @file_b, 1 if
 *               @file_b should be sorted before @file_a, 0 if equal.
 **/
gint
fmb_file_compare_by_name (const FmbFile *file_a,
                             const FmbFile *file_b,
                             gboolean          case_sensitive)
{
  gint result = 0;

#ifdef G_ENABLE_DEBUG
  /* probably too expensive to do the instance check every time
   * this function is called, so only for debugging builds.
   */
  _fmb_return_val_if_fail (FMB_IS_FILE (file_a), 0);
  _fmb_return_val_if_fail (FMB_IS_FILE (file_b), 0);
#endif

  /* case insensitive checking */
  if (G_LIKELY (!case_sensitive))
    result = g_strcmp0 (file_a->collate_key_nocase, file_b->collate_key_nocase);

  /* fall-back to case sensitive */
  if (result == 0)
    result = g_strcmp0 (file_a->collate_key, file_b->collate_key);

  /* this happens in the trash */
  if (result == 0)
    {
      result = g_strcmp0 (fmb_file_get_original_path (file_a),
                          fmb_file_get_original_path (file_b));
    }

  return result;
}



static gboolean
fmb_file_same_filesystem (const FmbFile *file_a,
                             const FmbFile *file_b)
{
  const gchar *filesystem_id_a;
  const gchar *filesystem_id_b;

  _fmb_return_val_if_fail (FMB_IS_FILE (file_a), FALSE);
  _fmb_return_val_if_fail (FMB_IS_FILE (file_b), FALSE);

  /* return false if we have no information about one of the files */
  if (file_a->info == NULL || file_b->info == NULL)
    return FALSE;

  /* determine the filesystem IDs */
  filesystem_id_a = g_file_info_get_attribute_string (file_a->info, 
                                                      G_FILE_ATTRIBUTE_ID_FILESYSTEM);

  filesystem_id_b = g_file_info_get_attribute_string (file_b->info, 
                                                      G_FILE_ATTRIBUTE_ID_FILESYSTEM);

  /* compare the filesystem IDs */
  return blxo_str_is_equal (filesystem_id_a, filesystem_id_b);
}



/**
 * fmb_file_cache_lookup:
 * @file : a #GFile.
 *
 * Looks up the #FmbFile for @file in the internal file
 * cache and returns the file present for @file in the
 * cache or %NULL if no #FmbFile is cached for @file.
 *
 * Note that no reference is taken for the caller.
 *
 * This method should not be used but in very rare cases.
 * Consider using fmb_file_get() instead.
 *
 * Return value: the #FmbFile for @file in the internal
 *               cache, or %NULL. If you are done with the
 *               file, use g_object_unref to release.
 **/
FmbFile *
fmb_file_cache_lookup (const GFile *file)
{
  GWeakRef   *ref;
  FmbFile *cached_file;

  _fmb_return_val_if_fail (G_IS_FILE (file), NULL);

  G_LOCK (file_cache_mutex);

  /* allocate the FmbFile cache on-demand */
  if (G_UNLIKELY (file_cache == NULL))
    {
      file_cache = g_hash_table_new_full (g_file_hash, 
                                          (GEqualFunc) g_file_equal, 
                                          (GDestroyNotify) g_object_unref, 
                                          (GDestroyNotify) weak_ref_free);
    }

  ref = g_hash_table_lookup (file_cache, file);

  if (ref == NULL)
    cached_file = NULL;
  else
    cached_file = g_weak_ref_get (ref);

  G_UNLOCK (file_cache_mutex);

  return cached_file;
}



gchar *
fmb_file_cached_display_name (const GFile *file)
{
  FmbFile *cached_file;
  gchar      *display_name;

  /* check if we have a FmbFile for it in the cache (usually is the case) */
  cached_file = fmb_file_cache_lookup (file);
  if (cached_file != NULL)
    {
      /* determine the display name of the file */
      display_name = g_strdup (fmb_file_get_display_name (cached_file));
      g_object_unref (cached_file);
    }
  else
    {
      /* determine something a hopefully good approximation of the display name */
      display_name = fmb_g_file_get_display_name (G_FILE (file));
    }

  return display_name;
}



static gint
compare_app_infos (gconstpointer a,
                   gconstpointer b)
{
  return g_app_info_equal (G_APP_INFO (a), G_APP_INFO (b)) ? 0 : 1;
}



/**
 * fmb_file_list_get_applications:
 * @file_list : a #GList of #FmbFile<!---->s.
 *
 * Returns the #GList of #GAppInfo<!---->s that can be used to open 
 * all #FmbFile<!---->s in the given @file_list.
 *
 * The caller is responsible to free the returned list using something like:
 * <informalexample><programlisting>
 * g_list_free_full (list, g_object_unref);
 * </programlisting></informalexample>
 *
 * Return value: the list of #GAppInfo<!---->s that can be used to open all 
 *               items in the @file_list.
 **/
GList*
fmb_file_list_get_applications (GList *file_list)
{
  GList       *applications = NULL;
  GList       *list;
  GList       *next;
  GList       *ap;
  GList       *lp;
  GAppInfo    *default_application;
  const gchar *previous_type = NULL;
  const gchar *current_type;

  /* determine the set of applications that can open all files */
  for (lp = file_list; lp != NULL; lp = lp->next)
    {
      current_type = fmb_file_get_content_type (lp->data);

      /* no need to check anything if this file has the same mimetype as the previous file */
      if (current_type != NULL && previous_type != NULL)
        if (G_LIKELY (g_content_type_equals (previous_type, current_type)))
          continue;

      /* store the previous type */
      previous_type = current_type;

      /* determine the list of applications that can open this file */
      if (G_UNLIKELY (current_type != NULL))
        {
          list = g_app_info_get_all_for_type (current_type);

          /* move any default application in front of the list */
          default_application = g_app_info_get_default_for_type (current_type, FALSE);
          if (G_LIKELY (default_application != NULL))
            {
              for (ap = list; ap != NULL; ap = ap->next)
                {
                  if (g_app_info_equal (ap->data, default_application))
                    {
                      g_object_unref (ap->data);
                      list = g_list_delete_link (list, ap);
                      break;
                    }
                }
              list = g_list_prepend (list, default_application);
            }
        }
      else
        list = NULL;

      if (G_UNLIKELY (applications == NULL))
        {
          /* first file, so just use the applications list */
          applications = list;
        }
      else
        {
          /* keep only the applications that are also present in list */
          for (ap = applications; ap != NULL; ap = next)
            {
              /* grab a pointer on the next application */
              next = ap->next;

              /* check if the application is present in list */
              if (g_list_find_custom (list, ap->data, compare_app_infos) == NULL)
                {
                  /* drop our reference on the application */
                  g_object_unref (G_OBJECT (ap->data));

                  /* drop this application from the list */
                  applications = g_list_delete_link (applications, ap);
                }
            }

          /* release the list of applications for this file */
          g_list_free_full (list, g_object_unref);
        }

      /* check if the set is still not empty */
      if (G_LIKELY (applications == NULL))
        break;
    }

  /* remove hidden applications */
  for (ap = applications; ap != NULL; ap = next)
    {
      /* grab a pointer on the next application */
      next = ap->next;

      if (!fmb_g_app_info_should_show (ap->data))
        {
          /* drop our reference on the application */
          g_object_unref (G_OBJECT (ap->data));

          /* drop this application from the list */
          applications = g_list_delete_link (applications, ap);
        }
    }

  return applications;
}



/**
 * fmb_file_list_to_fmb_g_file_list:
 * @file_list : a #GList of #FmbFile<!---->s.
 *
 * Transforms the @file_list to a #GList of #GFile<!---->s for
 * the #FmbFile<!---->s contained within @file_list.
 *
 * The caller is responsible to free the returned list using
 * fmb_g_file_list_free() when no longer needed.
 *
 * Return value: the list of #GFile<!---->s for @file_list.
 **/
GList*
fmb_file_list_to_fmb_g_file_list (GList *file_list)
{
  GList *list = NULL;
  GList *lp;

  for (lp = g_list_last (file_list); lp != NULL; lp = lp->prev)
    list = g_list_prepend (list, g_object_ref (FMB_FILE (lp->data)->gfile));

  return list;
}

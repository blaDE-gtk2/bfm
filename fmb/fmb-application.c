/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2005-2007 Benedikt Meurer <benny@xfce.org>
 * Copyright (c) 2005      Jeff Franks <jcfranks@xfce.org>
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

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_MEMORY_H
#include <memory.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_TIME_H
#include <time.h>
#endif

#ifdef HAVE_GUDEV
#include <gudev/gudev.h>
#endif

#include <libbladeui/libbladeui.h>

#include <fmb/fmb-application.h>
#include <fmb/fmb-browser.h>
#include <fmb/fmb-create-dialog.h>
#include <fmb/fmb-dialogs.h>
#include <fmb/fmb-gdk-extensions.h>
#include <fmb/fmb-gobject-extensions.h>
#include <fmb/fmb-io-jobs.h>
#include <fmb/fmb-preferences.h>
#include <fmb/fmb-private.h>
#include <fmb/fmb-progress-dialog.h>
#include <fmb/fmb-renamer-dialog.h>
#include <fmb/fmb-thumbnail-cache.h>
#include <fmb/fmb-thumbnailer.h>
#include <fmb/fmb-util.h>
#include <fmb/fmb-view.h>

#define ACCEL_MAP_PATH "Fmb/accels.scm"



/* Prototype for the Fmb job launchers */
typedef FmbJob *(*Launcher) (GList *source_path_list,
                                GList *target_path_list);



/* Property identifiers */
enum
{
  PROP_0,
  PROP_DAEMON,
};



static void           fmb_application_finalize               (GObject                *object);
static void           fmb_application_get_property           (GObject                *object,
                                                                 guint                   prop_id,
                                                                 GValue                 *value,
                                                                 GParamSpec             *pspec);
static void           fmb_application_set_property           (GObject                *object,
                                                                 guint                   prop_id,
                                                                 const GValue           *value,
                                                                 GParamSpec             *pspec);
static void           fmb_application_accel_map_changed      (FmbApplication      *application);
static gboolean       fmb_application_accel_map_save         (gpointer                user_data);
static void           fmb_application_collect_and_launch     (FmbApplication      *application,
                                                                 gpointer                parent,
                                                                 const gchar            *icon_name,
                                                                 const gchar            *title,
                                                                 Launcher                launcher,
                                                                 GList                  *source_file_list,
                                                                 GFile                  *target_file,
                                                                 GClosure               *new_files_closure);
static void           fmb_application_launch_finished        (FmbJob              *job,
                                                                 FmbView             *view);
static void           fmb_application_launch                 (FmbApplication      *application,
                                                                 gpointer                parent,
                                                                 const gchar            *icon_name,
                                                                 const gchar            *title,
                                                                 Launcher                launcher,
                                                                 GList                  *source_path_list,
                                                                 GList                  *target_path_list,
                                                                 GClosure               *new_files_closure);
static void           fmb_application_window_destroyed       (GtkWidget              *window,
                                                                 FmbApplication      *application);
#ifdef HAVE_GUDEV
static void           fmb_application_uevent                 (GUdevClient            *client,
                                                                 const gchar            *action,
                                                                 GUdevDevice            *device,
                                                                 FmbApplication      *application);
static gboolean       fmb_application_volman_idle            (gpointer                user_data);
static void           fmb_application_volman_idle_destroy    (gpointer                user_data);
static void           fmb_application_volman_watch           (GPid                    pid,
                                                                 gint                    status,
                                                                 gpointer                user_data);
static void           fmb_application_volman_watch_destroy   (gpointer                user_data);
#endif
static gboolean       fmb_application_show_dialogs           (gpointer                user_data);
static void           fmb_application_show_dialogs_destroy   (gpointer                user_data);
static GtkWidget     *fmb_application_get_progress_dialog    (FmbApplication      *application);
static void           fmb_application_process_files          (FmbApplication      *application);



struct _FmbApplicationClass
{
  GObjectClass __parent__;
};

struct _FmbApplication
{
  GObject                __parent__;

  FmbPreferences     *preferences;
  GtkWidget             *progress_dialog;
  GList                 *windows;

  FmbThumbnailCache  *thumbnail_cache;
  FmbThumbnailer     *thumbnailer;

  gboolean               daemon;

  guint                  accel_map_save_id;
  GtkAccelMap           *accel_map;

  guint                  show_dialogs_timer_id;

#ifdef HAVE_GUDEV
  GUdevClient           *udev_client;

  GSList                *volman_udis;
  guint                  volman_idle_id;
  guint                  volman_watch_id;
#endif

  GList                 *files_to_launch;
};



static GQuark fmb_application_screen_quark;
static GQuark fmb_application_startup_id_quark;
static GQuark fmb_application_file_quark;



G_DEFINE_TYPE_EXTENDED (FmbApplication, fmb_application, G_TYPE_OBJECT, 0,
                        G_IMPLEMENT_INTERFACE (FMB_TYPE_BROWSER, NULL))



static void
fmb_application_class_init (FmbApplicationClass *klass)
{
  GObjectClass *gobject_class;
 
  /* pre-allocate the required quarks */
  fmb_application_screen_quark = 
    g_quark_from_static_string ("fmb-application-screen");
  fmb_application_startup_id_quark =
    g_quark_from_static_string ("fmb-application-startup-id");
  fmb_application_file_quark =
    g_quark_from_static_string ("fmb-application-file");

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = fmb_application_finalize;
  gobject_class->get_property = fmb_application_get_property;
  gobject_class->set_property = fmb_application_set_property;

  /**
   * FmbApplication:daemon:
   *
   * %TRUE if the application should be run in daemon mode,
   * in which case it will never terminate. %FALSE if the
   * application should terminate once the last window is
   * closed.
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_DAEMON,
                                   g_param_spec_boolean ("daemon",
                                                         "daemon",
                                                         "daemon",
                                                         FALSE,
                                                         BLXO_PARAM_READWRITE));
}



static void
fmb_application_init (FmbApplication *application)
{
#ifdef HAVE_GUDEV
  static const gchar *subsystems[] = { "block", "input", "usb", NULL };
#endif
  gchar              *path;

  /* initialize the application */
  application->preferences = fmb_preferences_get ();

  application->files_to_launch = NULL;
  application->progress_dialog = NULL;

  /* check if we have a saved accel map */
  path = xfce_resource_lookup (XFCE_RESOURCE_CONFIG, ACCEL_MAP_PATH);
  if (G_LIKELY (path != NULL))
    {
      /* load the accel map */
      gtk_accel_map_load (path);
      g_free (path);
    }

  /* watch for changes */
  application->accel_map = gtk_accel_map_get ();
  g_signal_connect_swapped (G_OBJECT (application->accel_map), "changed",
      G_CALLBACK (fmb_application_accel_map_changed), application);

#ifdef HAVE_GUDEV
  /* establish connection with udev */
  application->udev_client = g_udev_client_new (subsystems);

  /* connect to the client in order to be notified when devices are plugged in
   * or disconnected from the computer */
  g_signal_connect (application->udev_client, "uevent", 
                    G_CALLBACK (fmb_application_uevent), application);
#endif
}



static void
fmb_application_finalize (GObject *object)
{
  FmbApplication *application = FMB_APPLICATION (object);
  GList             *lp;

  /* unqueue all files waiting to be processed */
  fmb_g_file_list_free (application->files_to_launch);

  /* save the current accel map */
  if (G_UNLIKELY (application->accel_map_save_id != 0))
    {
      g_source_remove (application->accel_map_save_id);
      fmb_application_accel_map_save (application);
    }

  if (application->accel_map != NULL)
    g_object_unref (G_OBJECT (application->accel_map));

#ifdef HAVE_GUDEV
  /* cancel any pending volman watch source */
  if (G_UNLIKELY (application->volman_watch_id != 0))
    g_source_remove (application->volman_watch_id);
  
  /* cancel any pending volman idle source */
  if (G_UNLIKELY (application->volman_idle_id != 0))
    g_source_remove (application->volman_idle_id);

  /* drop all pending volume manager UDIs */
  g_slist_free_full (application->volman_udis, g_free);

  /* disconnect from the udev client */
  g_object_unref (application->udev_client);
#endif

  /* drop any running "show dialogs" timer */
  if (G_UNLIKELY (application->show_dialogs_timer_id != 0))
    g_source_remove (application->show_dialogs_timer_id);

  /* drop ref on the thumbnailer */
  if (application->thumbnailer != NULL)
    g_object_unref (application->thumbnailer);

  /* drop the open windows (this includes the progress dialog) */
  for (lp = application->windows; lp != NULL; lp = lp->next)
    {
      g_signal_handlers_disconnect_by_func (G_OBJECT (lp->data), G_CALLBACK (fmb_application_window_destroyed), application);
      gtk_widget_destroy (GTK_WIDGET (lp->data));
    }
  g_list_free (application->windows);

  /* release the thumbnail cache */
  if (application->thumbnail_cache != NULL)
    g_object_unref (G_OBJECT (application->thumbnail_cache));

  /* disconnect from the preferences */
  g_object_unref (G_OBJECT (application->preferences));
  
  (*G_OBJECT_CLASS (fmb_application_parent_class)->finalize) (object);
}



static void
fmb_application_get_property (GObject    *object,
                                 guint       prop_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
  FmbApplication *application = FMB_APPLICATION (object);
  
  switch (prop_id)
    {
    case PROP_DAEMON:
      g_value_set_boolean (value, fmb_application_get_daemon (application));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static void
fmb_application_set_property (GObject      *object,
                                 guint         prop_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
  FmbApplication *application = FMB_APPLICATION (object);
  
  switch (prop_id)
    {
    case PROP_DAEMON:
      fmb_application_set_daemon (application, g_value_get_boolean (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static gboolean
fmb_application_accel_map_save (gpointer user_data)
{
  FmbApplication *application = FMB_APPLICATION (user_data);
  gchar             *path;

  _fmb_return_val_if_fail (FMB_IS_APPLICATION (application), FALSE);

  application->accel_map_save_id = 0;

  /* save the current accel map */
  path = xfce_resource_save_location (XFCE_RESOURCE_CONFIG, ACCEL_MAP_PATH, TRUE);
  if (G_LIKELY (path != NULL))
    {
      /* save the accel map */
      gtk_accel_map_save (path);
      g_free (path);
    }

  return FALSE;
}



static void
fmb_application_accel_map_changed (FmbApplication *application)
{
  _fmb_return_if_fail (FMB_IS_APPLICATION (application));

  /* stop pending save */
  if (application->accel_map_save_id != 0)
    {
      g_source_remove (application->accel_map_save_id);
      application->accel_map_save_id = 0;
    }

  /* schedule new save */
  application->accel_map_save_id =
      g_timeout_add_seconds (10, fmb_application_accel_map_save, application);
}



static void
fmb_application_collect_and_launch (FmbApplication *application,
                                       gpointer           parent,
                                       const gchar       *icon_name,
                                       const gchar       *title,
                                       Launcher           launcher,
                                       GList             *source_file_list,
                                       GFile             *target_file,
                                       GClosure          *new_files_closure)
{
  GFile  *file;
  GError *err = NULL;
  GList  *target_file_list = NULL;
  GList  *lp;
  gchar  *base_name;

  /* check if we have anything to operate on */
  if (G_UNLIKELY (source_file_list == NULL))
    return;

  /* generate the target path list */
  for (lp = g_list_last (source_file_list); err == NULL && lp != NULL; lp = lp->prev)
    {
      /* verify that we're not trying to collect a root node */
      if (G_UNLIKELY (fmb_g_file_is_root (lp->data)))
        {
          /* tell the user that we cannot perform the requested operation */
          g_set_error (&err, G_FILE_ERROR, G_FILE_ERROR_INVAL, "%s", g_strerror (EINVAL));
        }
      else
        {
          base_name = g_file_get_basename (lp->data);
          file = g_file_resolve_relative_path (target_file, base_name);
          g_free (base_name);

          /* add to the target file list */
          target_file_list = fmb_g_file_list_prepend (target_file_list, file);
          g_object_unref (file);
        }
    }

  /* check if we failed */
  if (G_UNLIKELY (err != NULL))
    {
      /* display an error message to the user */
      fmb_dialogs_show_error (parent, err, _("Failed to launch operation"));

      /* release the error */
      g_error_free (err);
    }
  else
    {
      /* launch the operation */
      fmb_application_launch (application, parent, icon_name, title, launcher,
                                 source_file_list, target_file_list, new_files_closure);
    }

  /* release the target path list */
  fmb_g_file_list_free (target_file_list);
}



static void
fmb_application_launch_finished_too_late (gpointer  user_data,
                                             GObject  *where_the_object_was)
{
  FmbJob *job = FMB_JOB (user_data);

  _fmb_return_if_fail (FMB_IS_JOB (job));
  _fmb_return_if_fail (FMB_IS_VIEW (where_the_object_was));

  /* remove the finished signal */
  g_signal_handlers_disconnect_by_func (G_OBJECT (job),
                                        G_CALLBACK (fmb_application_launch_finished),
                                        where_the_object_was);
}



static void
fmb_application_launch_finished (FmbJob  *job,
                                    FmbView *view)
{
  _fmb_return_if_fail (FMB_IS_JOB (job));
  _fmb_return_if_fail (FMB_IS_VIEW (view));

  /* remove the view weak ref */
  g_object_weak_unref (G_OBJECT (view),
                       fmb_application_launch_finished_too_late,
                       job);

  /* the job completed, refresh the interface
   * directly to make it feel snappy */
  fmb_view_reload (view, FALSE);
}



static void
fmb_application_launch (FmbApplication *application,
                           gpointer           parent,
                           const gchar       *icon_name,
                           const gchar       *title,
                           Launcher           launcher,
                           GList             *source_file_list,
                           GList             *target_file_list,
                           GClosure          *new_files_closure)
{
  GtkWidget *dialog;
  GdkScreen *screen;
  FmbJob *job;

  _fmb_return_if_fail (parent == NULL || GDK_IS_SCREEN (parent) || GTK_IS_WIDGET (parent));

  /* parse the parent pointer */
  screen = fmb_util_parse_parent (parent, NULL);

  /* try to allocate a new job for the operation */
  job = (*launcher) (source_file_list, target_file_list);

  if (FMB_IS_VIEW (parent))
    {
      /* connect a callback to instantly refresh the fmb view */
      g_signal_connect (G_OBJECT (job), "finished",
                        G_CALLBACK (fmb_application_launch_finished),
                        parent);

      /* watch destruction of the parent, so we disconnect before the
       * job is finished */
      g_object_weak_ref (G_OBJECT (parent),
                         fmb_application_launch_finished_too_late,
                         job);
    }

  /* connect the "new-files" closure (if any) */
  if (G_LIKELY (new_files_closure != NULL))
    g_signal_connect_closure (job, "new-files", new_files_closure, FALSE);

  /* get the shared progress dialog */
  dialog = fmb_application_get_progress_dialog (application);

  /* place the dialog on the given screen */
  if (screen != NULL)
    gtk_window_set_screen (GTK_WINDOW (dialog), screen);

  if (fmb_progress_dialog_has_jobs (FMB_PROGRESS_DIALOG (dialog)))
    {
      /* add the job to the dialog */
      fmb_progress_dialog_add_job (FMB_PROGRESS_DIALOG (dialog), 
                                      job, icon_name, title);

      /* show the dialog immediately */
      fmb_application_show_dialogs (application);
    }
  else
    {
      /* add the job to the dialog */
      fmb_progress_dialog_add_job (FMB_PROGRESS_DIALOG (dialog), 
                                      job, icon_name, title);

      /* Set up a timer to show the dialog, to make sure we don't
       * just popup and destroy a dialog for a very short job.
       */
      if (G_LIKELY (application->show_dialogs_timer_id == 0))
        {
          application->show_dialogs_timer_id = 
            g_timeout_add_full (G_PRIORITY_DEFAULT, 750, fmb_application_show_dialogs,
                                application, fmb_application_show_dialogs_destroy);
        }
    }

  /* drop our reference on the job */
  g_object_unref (job);
}



static void
fmb_application_window_destroyed (GtkWidget         *window,
                                     FmbApplication *application)
{
  _fmb_return_if_fail (GTK_IS_WINDOW (window));
  _fmb_return_if_fail (FMB_IS_APPLICATION (application));
  _fmb_return_if_fail (g_list_find (application->windows, window) != NULL);

  /* take a ref on the thumbnailer in daemon mode, this way we don't
   * need to build the content-type / scheme match table
   */
  if (application->thumbnailer == NULL && application->daemon)
    application->thumbnailer = fmb_thumbnailer_get ();

  application->windows = g_list_remove (application->windows, window);

  /* terminate the application if we don't have any more
   * windows and we are not in daemon mode.
   */
  if (G_UNLIKELY (application->windows == NULL && !application->daemon))
    gtk_main_quit ();
}



#ifdef HAVE_GUDEV
static void
fmb_application_uevent (GUdevClient       *client,
                           const gchar       *action,
                           GUdevDevice       *device,
                           FmbApplication *application)
{
  const gchar *sysfs_path;
  gboolean     is_cdrom = FALSE;
  gboolean     has_media = FALSE;
  GSList      *lp;

  _fmb_return_if_fail (G_UDEV_IS_CLIENT (client));
  _fmb_return_if_fail (action != NULL && *action != '\0');
  _fmb_return_if_fail (G_UDEV_IS_DEVICE (device));
  _fmb_return_if_fail (FMB_IS_APPLICATION (application));
  _fmb_return_if_fail (client == application->udev_client);

  /* determine the sysfs path of the device */
  sysfs_path = g_udev_device_get_sysfs_path (device);

  /* check if the device is a CD drive */
  is_cdrom = g_udev_device_get_property_as_boolean (device, "ID_CDROM");
  has_media = g_udev_device_get_property_as_boolean (device, "ID_CDROM_MEDIA");

  /* distinguish between "add", "change" and "remove" actions, ignore "move" */
  if (g_strcmp0 (action, "add") == 0  
      || (is_cdrom && has_media && g_strcmp0 (action, "change") == 0))
    {
      /* only insert the path if we don't have it already */
      if (g_slist_find_custom (application->volman_udis, sysfs_path, 
                               (GCompareFunc) g_utf8_collate) == NULL)
        {
          application->volman_udis = g_slist_prepend (application->volman_udis, 
                                                      g_strdup (sysfs_path));

          /* check if there's currently no active or scheduled handler */
          if (G_LIKELY (application->volman_idle_id == 0 
                        && application->volman_watch_id == 0))
            {
              /* schedule a new handler using the idle source, which invokes the handler */
              application->volman_idle_id = 
                g_idle_add_full (G_PRIORITY_LOW, fmb_application_volman_idle, 
                                 application, fmb_application_volman_idle_destroy);
            }
        }
    }
  else if (g_strcmp0 (action, "remove") == 0)
    {
      /* look for the sysfs path in the list of pending paths */
      lp = g_slist_find_custom (application->volman_udis, sysfs_path, 
                                (GCompareFunc) g_utf8_collate);

      if (G_LIKELY (lp != NULL))
        {
          /* free the sysfs path string */
          g_free (lp->data);

          /* drop the sysfs path from the list of pending device paths */
          application->volman_udis = g_slist_delete_link (application->volman_udis, lp);
        }
    }
}



static gboolean
fmb_application_volman_idle (gpointer user_data)
{
  FmbApplication *application = FMB_APPLICATION (user_data);
  GdkScreen         *screen;
  gboolean           misc_volume_management;
  GError            *err = NULL;
  gchar            **argv;
  GPid               pid;

  GDK_THREADS_ENTER ();

  /* check if volume management is enabled (otherwise, we don't spawn anything, but clear the list here) */
  g_object_get (G_OBJECT (application->preferences), "misc-volume-management", &misc_volume_management, NULL);
  if (G_LIKELY (misc_volume_management))
    {
      /* check if we don't already have a handler, and we have a pending UDI */
      if (application->volman_watch_id == 0 && application->volman_udis != NULL)
        {
          /* generate the argument list for the volman */
          argv = g_new (gchar *, 4);
          argv[0] = g_strdup ("fmb-volman");
          argv[1] = g_strdup ("--device-added");
          argv[2] = application->volman_udis->data;
          argv[3] = NULL;

          /* remove the first list item from the pending list */
          application->volman_udis = g_slist_delete_link (application->volman_udis, application->volman_udis);

          /* locate the currently active screen (the one with the pointer) */
          screen = xfce_gdk_screen_get_active (NULL);

          /* try to spawn the volman on the active screen */
          if (gdk_spawn_on_screen (screen, NULL, argv, NULL, G_SPAWN_DO_NOT_REAP_CHILD | G_SPAWN_SEARCH_PATH, NULL, NULL, &pid, &err))
            {
              /* add a child watch for the volman handler */
              application->volman_watch_id = g_child_watch_add_full (G_PRIORITY_LOW, pid, fmb_application_volman_watch,
                                                                     application, fmb_application_volman_watch_destroy);
            }
          else
            {
              /* failed to spawn, tell the user, giving a hint to install the fmb-volman package */
              g_warning ("Failed to launch the volume manager (%s), make sure you have the \"fmb-volman\" package installed.", err->message);
              g_error_free (err);
            }

          /* cleanup */
          g_strfreev (argv);
        }

    }
  else
    {
      /* drop all pending HAL device UDIs */
      g_slist_free_full (application->volman_udis, g_free);
      application->volman_udis = NULL;
    }

  GDK_THREADS_LEAVE ();

  /* keep the idle source alive as long as no handler is
   * active and we have pending UDIs that must be handled
   */
  return (application->volman_watch_id == 0
       && application->volman_udis != NULL);
}



static void
fmb_application_volman_idle_destroy (gpointer user_data)
{
  FMB_APPLICATION (user_data)->volman_idle_id = 0;
}



static void
fmb_application_volman_watch (GPid     pid,
                                 gint     status,
                                 gpointer user_data)
{
  FmbApplication *application = FMB_APPLICATION (user_data);

  GDK_THREADS_ENTER ();

  /* check if the idle source isn't active, but we have pending UDIs */
  if (application->volman_idle_id == 0 && application->volman_udis != NULL)
    {
      /* schedule a new handler using the idle source, which invokes the handler */
      application->volman_idle_id = g_idle_add_full (G_PRIORITY_LOW, fmb_application_volman_idle,
                                                     application, fmb_application_volman_idle_destroy);
    }

  /* be sure to close the pid handle */
  g_spawn_close_pid (pid);

  GDK_THREADS_LEAVE ();
}



static void
fmb_application_volman_watch_destroy (gpointer user_data)
{
  FMB_APPLICATION (user_data)->volman_watch_id = 0;
}
#endif /* HAVE_GUDEV */



static gboolean
fmb_application_show_dialogs (gpointer user_data)
{
  FmbApplication *application = FMB_APPLICATION (user_data);

  GDK_THREADS_ENTER ();

  /* show the progress dialog */
  if (application->progress_dialog != NULL)
    gtk_window_present (GTK_WINDOW (application->progress_dialog));

  GDK_THREADS_LEAVE ();

  return FALSE;
}



static void
fmb_application_show_dialogs_destroy (gpointer user_data)
{
  FMB_APPLICATION (user_data)->show_dialogs_timer_id = 0;
}



/**
 * fmb_application_get:
 *
 * Returns the global shared #FmbApplication instance.
 * This method takes a reference on the global instance
 * for the caller, so you must call g_object_unref()
 * on it when done.
 *
 * Return value: the shared #FmbApplication instance.
 **/
FmbApplication*
fmb_application_get (void)
{
  static FmbApplication *application = NULL;

  if (G_UNLIKELY (application == NULL))
    {
      application = g_object_new (FMB_TYPE_APPLICATION, NULL);
      g_object_add_weak_pointer (G_OBJECT (application), (gpointer) &application);
    }
  else
    {
      g_object_ref (G_OBJECT (application));
    }

  return application;
}



/**
 * fmb_application_get_daemon:
 * @application : a #FmbApplication.
 *
 * Returns %TRUE if @application is in daemon mode.
 *
 * Return value: %TRUE if @application is in daemon mode.
 **/
gboolean
fmb_application_get_daemon (FmbApplication *application)
{
  _fmb_return_val_if_fail (FMB_IS_APPLICATION (application), FALSE);
  return application->daemon;
}



/**
 * fmb_application_set_daemon:
 * @application : a #FmbApplication.
 * @daemonize   : %TRUE to set @application into daemon mode.
 *
 * If @daemon is %TRUE, @application will be set into daemon mode.
 **/
void
fmb_application_set_daemon (FmbApplication *application,
                               gboolean           daemonize)
{
  _fmb_return_if_fail (FMB_IS_APPLICATION (application));

  if (application->daemon != daemonize)
    {
      application->daemon = daemonize;
      g_object_notify (G_OBJECT (application), "daemon");
    }
}



/**
 * fmb_application_get_windows:
 * @application : a #FmbApplication.
 *
 * Returns the list of regular #FmbWindows currently handled by
 * @application. The returned list is owned by the caller and
 * must be freed using g_list_free().
 *
 * Return value: the list of regular #FmbWindows in @application.
 **/
GList*
fmb_application_get_windows (FmbApplication *application)
{
  GList *windows = NULL;
  GList *lp;

  _fmb_return_val_if_fail (FMB_IS_APPLICATION (application), NULL);

  for (lp = application->windows; lp != NULL; lp = lp->next)
    if (G_LIKELY (FMB_IS_WINDOW (lp->data)))
      windows = g_list_prepend (windows, lp->data);

  return windows;
}


/**
 * fmb_application_has_windows:
 * @application : a #FmbApplication.
 *
 * Returns %TRUE if @application controls atleast one window.
 *
 * Return value: %TRUE if @application controls atleast one window.
 **/
gboolean
fmb_application_has_windows (FmbApplication *application)
{
  _fmb_return_val_if_fail (FMB_IS_APPLICATION (application), FALSE);
  return (application->windows != NULL);
}



/**
 * fmb_application_take_window:
 * @application : a #FmbApplication.
 * @window      : a #GtkWindow.
 *
 * Lets @application take over control of the specified @window.
 * @application will not exit until the last controlled #GtkWindow
 * is closed by the user.
 * 
 * If the @window has no transient window, it will also create a
 * new #GtkWindowGroup for this window. This will make different
 * windows work independant (think gtk_dialog_run).
 **/
void
fmb_application_take_window (FmbApplication *application,
                                GtkWindow         *window)
{
  GtkWindowGroup *group;

  _fmb_return_if_fail (GTK_IS_WINDOW (window));
  _fmb_return_if_fail (FMB_IS_APPLICATION (application));
  _fmb_return_if_fail (g_list_find (application->windows, window) == NULL);

  /* only windows without a parent get a new window group */
  if (gtk_window_get_transient_for (window) == NULL)
    {
      group = gtk_window_group_new ();
      gtk_window_group_add_window (group, window);
      g_object_weak_ref (G_OBJECT (window), (GWeakNotify) g_object_unref, group);
    }

  /* connect to the "destroy" signal */
  g_signal_connect (G_OBJECT (window), "destroy", G_CALLBACK (fmb_application_window_destroyed), application);

  /* add the window to our internal list */
  application->windows = g_list_prepend (application->windows, window);
}



/**
 * fmb_application_open_window:
 * @application    : a #FmbApplication.
 * @directory      : the directory to open.
 * @screen         : the #GdkScreen on which to open the window or %NULL
 *                   to open on the default screen.
 * @startup_id     : startup id from startup notification passed along
 *                   with dbus to make focus stealing work properly.
 *
 * Opens a new #FmbWindow for @application, displaying the
 * given @directory.
 *
 * Return value: the newly allocated #FmbWindow.
 **/
GtkWidget*
fmb_application_open_window (FmbApplication *application,
                                FmbFile        *directory,
                                GdkScreen         *screen,
                                const gchar       *startup_id)
{
  GtkWidget *window;
  gchar     *role;

  _fmb_return_val_if_fail (FMB_IS_APPLICATION (application), NULL);
  _fmb_return_val_if_fail (directory == NULL || FMB_IS_FILE (directory), NULL);
  _fmb_return_val_if_fail (screen == NULL || GDK_IS_SCREEN (screen), NULL);

  if (G_UNLIKELY (screen == NULL))
    screen = gdk_screen_get_default ();

  /* generate a unique role for the new window (for session management) */
  role = g_strdup_printf ("Fmb-%u-%u", (guint) time (NULL), (guint) g_random_int ());

  /* allocate the window */
  window = g_object_new (FMB_TYPE_WINDOW,
                         "role", role,
                         "screen", screen,
                         NULL);

  /* cleanup */
  g_free (role);

  /* set the startup id */
  if (startup_id != NULL)
    gtk_window_set_startup_id (GTK_WINDOW (window), startup_id);

  /* hook up the window */
  fmb_application_take_window (application, GTK_WINDOW (window));

  /* show the new window */
  gtk_widget_show (window);

  /* change the directory */
  if (directory != NULL)
    fmb_window_set_current_directory (FMB_WINDOW (window), directory);

  return window;
}



/**
 * fmb_application_bulk_rename:
 * @application       : a #FmbApplication.
 * @working_directory : the default working directory for the bulk rename dialog.
 * @filenames         : the list of file names that should be renamed or the empty
 *                      list to start with an empty rename dialog. The file names
 *                      can either be absolute paths, supported URIs or relative file
 *                      names to @working_directory.
 * @standalone        : %TRUE to display the bulk rename dialog like a standalone
 *                      application.
 * @screen            : the #GdkScreen on which to rename the @filenames or %NULL
 *                      to use the default #GdkScreen.
 * @startup_id        : startup notification id to properly finish startup notification
 *                      and focus the window when focus stealing is enabled or %NULL.
 * @error             : return location for errors or %NULL.
 *
 * Tries to popup the bulk rename dialog.
 *
 * Return value: %TRUE if the dialog was opened successfully, otherwise %FALSE.
 **/
gboolean
fmb_application_bulk_rename (FmbApplication *application,
                                const gchar       *working_directory,
                                gchar            **filenames,
                                gboolean           standalone,
                                GdkScreen         *screen,
                                const gchar       *startup_id,
                                GError           **error)
{
  FmbFile *current_directory = NULL;
  FmbFile *file;
  gboolean    result = FALSE;
  GList      *file_list = NULL;
  gchar      *filename;
  gint        n;

  _fmb_return_val_if_fail (screen == NULL || GDK_IS_SCREEN (screen), FALSE);
  _fmb_return_val_if_fail (FMB_IS_APPLICATION (application), FALSE);
  _fmb_return_val_if_fail (error == NULL || *error == NULL, FALSE);
  _fmb_return_val_if_fail (working_directory != NULL, FALSE);

  /* determine the file for the working directory */
  current_directory = fmb_file_get_for_uri (working_directory, error);
  if (G_UNLIKELY (current_directory == NULL))
    return FALSE;

  /* check if we should use the default screen */
  if (G_LIKELY (screen == NULL))
    screen = gdk_screen_get_default ();

  /* try to process all filenames and convert them to the appropriate file objects */
  for (n = 0; filenames[n] != NULL; ++n)
    {
      /* check if the filename is an absolute path or looks like an URI */
      if (g_path_is_absolute (filenames[n]) || blxo_str_looks_like_an_uri (filenames[n]))
        {
          /* determine the file for the filename directly */
          file = fmb_file_get_for_uri (filenames[n], error);
        }
      else
        {
          /* translate the filename into an absolute path first */
          filename = g_build_filename (working_directory, filenames[n], NULL);
          file = fmb_file_get_for_uri (filename, error);
          g_free (filename);
        }

      /* verify that we have a valid file */
      if (G_LIKELY (file != NULL))
        file_list = g_list_append (file_list, file);
      else
        break;
    }

  /* check if the filenames where resolved successfully */
  if (G_LIKELY (filenames[n] == NULL))
    {
      /* popup the bulk rename dialog */
      fmb_show_renamer_dialog (screen, current_directory, file_list, standalone, startup_id);

      /* we succeed */
      result = TRUE;
    }

  /* cleanup */
  g_object_unref (G_OBJECT (current_directory));
  fmb_g_file_list_free (file_list);

  return result;
}



static GtkWidget *
fmb_application_get_progress_dialog (FmbApplication *application)
{
  _fmb_return_val_if_fail (FMB_IS_APPLICATION (application), NULL);

  if (application->progress_dialog == NULL)
    {
      application->progress_dialog = fmb_progress_dialog_new ();

      g_object_add_weak_pointer (G_OBJECT (application->progress_dialog),
                                 (gpointer) &application->progress_dialog);

      fmb_application_take_window (application, 
                                      GTK_WINDOW (application->progress_dialog));
    }

  return application->progress_dialog;
}



static void
fmb_application_process_files_finish (FmbBrowser *browser,
                                         FmbFile    *file,
                                         FmbFile    *target_file,
                                         GError        *error,
                                         gpointer       unused)
{ 
  FmbApplication *application = FMB_APPLICATION (browser);
  GdkScreen         *screen;
  const gchar       *startup_id;

  _fmb_return_if_fail (FMB_IS_BROWSER (browser));
  _fmb_return_if_fail (FMB_IS_FILE (file));
  _fmb_return_if_fail (FMB_IS_APPLICATION (application));

  /* determine and reset the screen of the file */
  screen = g_object_get_qdata (G_OBJECT (file), fmb_application_screen_quark);
  g_object_set_qdata (G_OBJECT (file), fmb_application_screen_quark, NULL);

  /* determine and the startup id of the file */
  startup_id = g_object_get_qdata (G_OBJECT (file), fmb_application_startup_id_quark);

  /* check if resolving/mounting failed */
  if (error != NULL)
    {
      /* don't display cancel errors */
      if (error->domain != G_IO_ERROR || error->code != G_IO_ERROR_CANCELLED)
        {
          /* tell the user that we were unable to launch the file specified */
          fmb_dialogs_show_error (screen, error, _("Failed to open \"%s\""), 
                                     fmb_file_get_display_name (file));
        }

      /* stop processing files */
      fmb_g_file_list_free (application->files_to_launch);
      application->files_to_launch = NULL;
    }
  else
    {
      /* try to open the file or directory */
      fmb_file_launch (target_file, screen, startup_id, &error);

      /* remove the file from the list */
      application->files_to_launch = g_list_delete_link (application->files_to_launch,
                                                         application->files_to_launch);

      /* release the file */
      g_object_unref (file);

      /* check if we have more files to process */
      if (application->files_to_launch != NULL)
        {
          /* continue processing the next file */
          fmb_application_process_files (application);
        }
    }

  /* unset the startup id */
  if (startup_id != NULL)
    g_object_set_qdata (G_OBJECT (file), fmb_application_startup_id_quark, NULL);
}



static void
fmb_application_process_files (FmbApplication *application)
{
  FmbFile *file;
  GdkScreen  *screen;

  _fmb_return_if_fail (FMB_IS_APPLICATION (application));
  
  /* don't do anything if no files are to be processed */
  if (application->files_to_launch == NULL)
    return;

  /* take the next file from the queue */
  file = FMB_FILE (application->files_to_launch->data);

  /* retrieve the screen we need to launch the file on */
  screen = g_object_get_qdata (G_OBJECT (file), fmb_application_screen_quark);

  /* resolve the file and/or mount its enclosing volume 
   * before handling it in the callback */
  fmb_browser_poke_file (FMB_BROWSER (application), file, screen,
                            fmb_application_process_files_finish, NULL);
}



/**
 * fmb_application_process_filenames:
 * @application       : a #FmbApplication.
 * @working_directory : the working directory relative to which the @filenames should
 *                      be interpreted.
 * @filenames         : a list of supported URIs or filenames. If a filename is specified
 *                      it can be either an absolute path or a path relative to the
 *                      @working_directory.
 * @screen            : the #GdkScreen on which to process the @filenames, or %NULL to
 *                      use the default screen.
 * @startup_id        : startup id to finish startup notification and properly focus the
 *                      window when focus stealing is enabled or %NULL.
 * @error             : return location for errors or %NULL.
 *
 * Tells @application to process the given @filenames and launch them appropriately.
 *
 * Return value: %TRUE on success, %FALSE if @error is set.
 **/
gboolean
fmb_application_process_filenames (FmbApplication *application,
                                      const gchar       *working_directory,
                                      gchar            **filenames,
                                      GdkScreen         *screen,
                                      const gchar       *startup_id,
                                      GError           **error)
{
  FmbFile *file;
  GError     *derror = NULL;
  gchar      *filename;
  GList      *file_list = NULL;
  GList      *lp;
  gint        n;

  _fmb_return_val_if_fail (FMB_IS_APPLICATION (application), FALSE);
  _fmb_return_val_if_fail (working_directory != NULL, FALSE);
  _fmb_return_val_if_fail (filenames != NULL, FALSE);
  _fmb_return_val_if_fail (*filenames != NULL, FALSE);
  _fmb_return_val_if_fail (screen == NULL || GDK_IS_SCREEN (screen), FALSE);
  _fmb_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  /* try to process all filenames and convert them to the appropriate file objects */
  for (n = 0; filenames[n] != NULL; ++n)
    {
      /* check if the filename is an absolute path or looks like an URI */
      if (g_path_is_absolute (filenames[n]) || blxo_str_looks_like_an_uri (filenames[n]))
        {
          /* determine the file for the filename directly */
          file = fmb_file_get_for_uri (filenames[n], &derror);
        }
      else
        {
          /* translate the filename into an absolute path first */
          filename = g_build_filename (working_directory, filenames[n], NULL);
          file = fmb_file_get_for_uri (filename, &derror);
          g_free (filename);
        }

      /* verify that we have a valid file */
      if (G_LIKELY (file != NULL))
        {
          file_list = g_list_append (file_list, file);
        }
      else
        {
          /* tell the user that we were unable to launch the file specified */
          fmb_dialogs_show_error (screen, derror, _("Failed to open \"%s\""), 
                                     filenames[n]);

          g_set_error (error, derror->domain, derror->code, 
                       _("Failed to open \"%s\": %s"), filenames[n], derror->message);
          g_error_free (derror);

          fmb_g_file_list_free (file_list);

          return FALSE;
        }
    }

  /* loop over all files */
  for (lp = file_list; lp != NULL; lp = lp->next)
    {
      /* remember the screen to launch the file on */
      g_object_set_qdata (G_OBJECT (lp->data), fmb_application_screen_quark, screen);

      /* remember the startup id to set on the window */
      if (G_LIKELY (startup_id != NULL && *startup_id != '\0'))
        g_object_set_qdata_full (G_OBJECT (lp->data), fmb_application_startup_id_quark, 
                                 g_strdup (startup_id), (GDestroyNotify) g_free);

      /* append the file to the list of files we need to launch */
      application->files_to_launch = g_list_append (application->files_to_launch, 
                                                    lp->data);
    }

  /* start processing files if we have any to launch */
  if (application->files_to_launch != NULL)
    fmb_application_process_files (application);

  /* free the file list */
  g_list_free (file_list);

  return TRUE;
}



gboolean
fmb_application_is_processing (FmbApplication *application)
{
  _fmb_return_val_if_fail (FMB_IS_APPLICATION (application), FALSE);
  return application->files_to_launch != NULL;
}



static void
fmb_application_rename_file_error (BlxoJob            *job,
                                      GError            *error,
                                      FmbApplication *application)
{
  FmbFile *file;
  GdkScreen  *screen;

  _fmb_return_if_fail (BLXO_IS_JOB (job));
  _fmb_return_if_fail (error != NULL);
  _fmb_return_if_fail (FMB_IS_APPLICATION (application));

  screen = g_object_get_qdata (G_OBJECT (job), fmb_application_screen_quark);
  file = g_object_get_qdata (G_OBJECT (job), fmb_application_file_quark);

  g_assert (screen != NULL);
  g_assert (file != NULL);

  fmb_dialogs_show_error (screen, error, _("Failed to rename \"%s\""), 
                             fmb_file_get_display_name (file));
}



static void
fmb_application_rename_file_finished (BlxoJob  *job,
                                         gpointer user_data)
{
  _fmb_return_if_fail (BLXO_IS_JOB (job));

  /* destroy the job object */
  g_object_unref (job);
}



/**
 * fmb_application_rename_file:
 * @application : a #FmbApplication.
 * @file        : a #FmbFile to be renamed.
 * @screen      : the #GdkScreen on which to open the window or %NULL
 *                to open on the default screen.
 * @startup_id  : startup id from startup notification passed along
 *                with dbus to make focus stealing work properly.
 *
 * Prompts the user to rename the @file.
 **/
void
fmb_application_rename_file (FmbApplication *application,
                                FmbFile        *file,
                                GdkScreen         *screen,
                                const gchar       *startup_id)
{
  FmbJob *job;

  _fmb_return_if_fail (FMB_IS_APPLICATION (application));
  _fmb_return_if_fail (FMB_IS_FILE (file));
  _fmb_return_if_fail (GDK_IS_SCREEN (screen));
  _fmb_return_if_fail (startup_id != NULL);

  /* TODO pass the startup ID to the rename dialog */

  /* run the rename dialog */
  job = fmb_dialogs_show_rename_file (screen, file);
  if (G_LIKELY (job != NULL))
    {
      /* remember the screen and file */
      g_object_set_qdata (G_OBJECT (job), fmb_application_screen_quark, screen);
      g_object_set_qdata_full (G_OBJECT (job), fmb_application_file_quark, 
                               g_object_ref (file), g_object_unref);

      /* handle rename errors */
      g_signal_connect (job, "error", 
                        G_CALLBACK (fmb_application_rename_file_error), application);

      /* destroy the job when it has finished */
      g_signal_connect (job, "finished",
                        G_CALLBACK (fmb_application_rename_file_finished), NULL);
    }
}



/**
 * fmb_application_create_file:
 * @application      : a #FmbApplication.
 * @parent_directory : the #FmbFile of the parent directory.
 * @content_type     : the content type of the new file.
 * @screen           : the #GdkScreen on which to open the window or %NULL
 *                     to open on the default screen.
 * @startup_id       : startup id from startup notification passed along
 *                     with dbus to make focus stealing work properly.
 *
 * Prompts the user to create a new file or directory in @parent_directory.
 * The @content_type defines the icon and other elements in the filename 
 * prompt dialog.
 **/
void
fmb_application_create_file (FmbApplication *application,
                                FmbFile        *parent_directory,
                                const gchar       *content_type,
                                GdkScreen         *screen,
                                const gchar       *startup_id)
{
  const gchar *dialog_title;
  const gchar *title;
  gboolean     is_directory;
  GList        path_list;
  gchar       *name;

  _fmb_return_if_fail (FMB_IS_APPLICATION (application));
  _fmb_return_if_fail (FMB_IS_FILE (parent_directory));
  _fmb_return_if_fail (content_type != NULL && *content_type != '\0');
  _fmb_return_if_fail (GDK_IS_SCREEN (screen));
  _fmb_return_if_fail (startup_id != NULL);

  is_directory = (g_strcmp0 (content_type, "inode/directory") == 0);

  if (is_directory)
    {
      dialog_title = _("New Folder");
      title = _("Create New Folder");
    }
  else
    {
      dialog_title = _("New File");
      title = _("Create New File");
    }

  /* TODO pass the startup ID to the rename dialog */

  /* ask the user to enter a name for the new folder */
  name = fmb_show_create_dialog (screen, content_type, dialog_title, title);
  if (G_LIKELY (name != NULL))
    {
      path_list.data = g_file_get_child (fmb_file_get_file (parent_directory), name);
      path_list.next = path_list.prev = NULL;

      /* launch the operation */
      if (is_directory)
        fmb_application_mkdir (application, screen, &path_list, NULL);
      else
        fmb_application_creat (application, screen, &path_list, NULL, NULL);

      g_object_unref (path_list.data);
      g_free (name);
    }
}



/**
 * fmb_application_create_file_from_template:
 * @application      : a #FmbApplication.
 * @parent_directory : the #FmbFile of the parent directory.
 * @template_file    : the #FmbFile of the template.
 * @screen           : the #GdkScreen on which to open the window or %NULL
 *                     to open on the default screen.
 * @startup_id       : startup id from startup notification passed along
 *                     with dbus to make focus stealing work properly.
 *
 * Prompts the user to create a new file or directory in @parent_directory
 * from an existing @template_file which predefines the name and extension
 * in the create dialog.
 **/
void
fmb_application_create_file_from_template (FmbApplication *application,
                                              FmbFile        *parent_directory,
                                              FmbFile        *template_file,
                                              GdkScreen         *screen,
                                              const gchar       *startup_id)
{
  GList  target_path_list;
  gchar *name;
  gchar *title;

  _fmb_return_if_fail (FMB_IS_APPLICATION (application));
  _fmb_return_if_fail (FMB_IS_FILE (parent_directory));
  _fmb_return_if_fail (FMB_IS_FILE (template_file));
  _fmb_return_if_fail (GDK_IS_SCREEN (screen));
  _fmb_return_if_fail (startup_id != NULL);

  /* generate a title for the create dialog */
  title = g_strdup_printf (_("Create Document from template \"%s\""),
                           fmb_file_get_display_name (template_file));

  /* TODO pass the startup ID to the rename dialog */

  /* ask the user to enter a name for the new document */
  name = fmb_show_create_dialog (screen, 
                                    fmb_file_get_content_type (template_file),
                                    fmb_file_get_display_name (template_file), 
                                    title);
  if (G_LIKELY (name != NULL))
    {
      /* fake the target path list */
      target_path_list.data = g_file_get_child (fmb_file_get_file (parent_directory), name);
      target_path_list.next = target_path_list.prev = NULL;

      /* launch the operation */
      fmb_application_creat (application, screen,
                                &target_path_list,
                                fmb_file_get_file (template_file),
                                NULL);

      /* release the target path */
      g_object_unref (target_path_list.data);

      /* release the file name */
      g_free (name);
    }

  /* clean up */
  g_free (title);
}



/**
 * fmb_application_copy_to:
 * @application       : a #FmbApplication.
 * @parent            : a #GdkScreen, a #GtkWidget or %NULL.
 * @source_file_list  : the lst of #GFile<!---->s that should be copied.
 * @target_file_list  : the list of #GFile<!---->s where files should be copied to.
 * @new_files_closure : a #GClosure to connect to the job's "new-files" signal,
 *                      which will be emitted when the job finishes with the
 *                      list of #GFile<!---->s created by the job, or
 *                      %NULL if you're not interested in the signal.
 *
 * Copies all files from @source_file_list to their locations specified in
 * @target_file_list.
 *
 * @source_file_list and @target_file_list must be of the same length. 
 **/
void
fmb_application_copy_to (FmbApplication *application,
                            gpointer           parent,
                            GList             *source_file_list,
                            GList             *target_file_list,
                            GClosure          *new_files_closure)
{
  _fmb_return_if_fail (g_list_length (source_file_list) == g_list_length (target_file_list));
  _fmb_return_if_fail (parent == NULL || GDK_IS_SCREEN (parent) || GTK_IS_WIDGET (parent));
  _fmb_return_if_fail (FMB_IS_APPLICATION (application));

  /* launch the operation */
  fmb_application_launch (application, parent, "stock_folder-copy",
                             _("Copying files..."), fmb_io_jobs_copy_files,
                             source_file_list, target_file_list, new_files_closure);
}



/**
 * fmb_application_copy_into:
 * @application       : a #FmbApplication.
 * @parent            : a #GdkScreen, a #GtkWidget or %NULL.
 * @source_file_list  : the list of #GFile<!---->s that should be copied.
 * @target_file       : the #GFile to the target directory.
 * @new_files_closure : a #GClosure to connect to the job's "new-files" signal,
 *                      which will be emitted when the job finishes with the
 *                      list of #GFile<!---->s created by the job, or
 *                      %NULL if you're not interested in the signal.
 *
 * Copies all files referenced by the @source_file_list to the directory
 * referenced by @target_file. This method takes care of all user interaction.
 **/
void
fmb_application_copy_into (FmbApplication *application,
                              gpointer           parent,
                              GList             *source_file_list,
                              GFile             *target_file,
                              GClosure          *new_files_closure)
{
  gchar *display_name;
  gchar *title;

  _fmb_return_if_fail (parent == NULL || GDK_IS_SCREEN (parent) || GTK_IS_WIDGET (parent));
  _fmb_return_if_fail (FMB_IS_APPLICATION (application));
  _fmb_return_if_fail (G_IS_FILE (target_file));

   /* generate a title for the progress dialog */
   display_name = fmb_file_cached_display_name (target_file);
   title = g_strdup_printf (_("Copying files to \"%s\"..."), display_name);
   g_free (display_name);

  /* collect the target files and launch the job */
  fmb_application_collect_and_launch (application, parent, "stock_folder-copy",
                                         title, fmb_io_jobs_copy_files,
                                         source_file_list, target_file, 
                                         new_files_closure);

  /* free the title */
  g_free (title);
}



/**
 * fmb_application_link_into:
 * @application       : a #FmbApplication.
 * @parent            : a #GdkScreen, a #GtkWidget or %NULL.
 * @source_file_list  : the list of #GFile<!---->s that should be symlinked.
 * @target_file       : the target directory.
 * @new_files_closure : a #GClosure to connect to the job's "new-files" signal,
 *                      which will be emitted when the job finishes with the
 *                      list of #GFile<!---->s created by the job, or
 *                      %NULL if you're not interested in the signal.
 *
 * Symlinks all files referenced by the @source_file_list to the directory
 * referenced by @target_file. This method takes care of all user
 * interaction.
 **/
void
fmb_application_link_into (FmbApplication *application,
                              gpointer           parent,
                              GList             *source_file_list,
                              GFile             *target_file,
                              GClosure          *new_files_closure)
{
  gchar *display_name;
  gchar *title;

  _fmb_return_if_fail (parent == NULL || GDK_IS_SCREEN (parent) || GTK_IS_WIDGET (parent));
  _fmb_return_if_fail (FMB_IS_APPLICATION (application));
  _fmb_return_if_fail (G_IS_FILE (target_file));

  /* generate a title for the progress dialog */
  display_name = fmb_file_cached_display_name (target_file);
  title = g_strdup_printf (_("Creating symbolic links in \"%s\"..."), display_name);
  g_free (display_name);

  /* collect the target files and launch the job */
  fmb_application_collect_and_launch (application, parent, "insert-link",
                                         title, fmb_io_jobs_link_files, 
                                         source_file_list, target_file, 
                                         new_files_closure);

  /* free the title */
  g_free (title);
}



/**
 * fmb_application_move_into:
 * @application       : a #FmbApplication.
 * @parent            : a #GdkScreen, a #GtkWidget or %NULL.
 * @source_file_list  : the list of #GFile<!---->s that should be moved.
 * @target_file       : the target directory.
 * @new_files_closure : a #GClosure to connect to the job's "new-files" signal,
 *                      which will be emitted when the job finishes with the
 *                      list of #GFile<!---->s created by the job, or
 *                      %NULL if you're not interested in the signal.
 *
 * Moves all files referenced by the @source_file_list to the directory
 * referenced by @target_file. This method takes care of all user
 * interaction.
 **/
void
fmb_application_move_into (FmbApplication *application,
                              gpointer           parent,
                              GList             *source_file_list,
                              GFile             *target_file,
                              GClosure          *new_files_closure)
{
  gchar *display_name;
  gchar *title;

  _fmb_return_if_fail (parent == NULL || GDK_IS_SCREEN (parent) || GTK_IS_WIDGET (parent));
  _fmb_return_if_fail (FMB_IS_APPLICATION (application));
  _fmb_return_if_fail (target_file != NULL);
  
  /* launch the appropriate operation depending on the target file */
  if (fmb_g_file_is_trashed (target_file))
    {
      fmb_application_trash (application, parent, source_file_list);
    }
  else
    {
      /* generate a title for the progress dialog */
      display_name = fmb_file_cached_display_name (target_file);
      title = g_strdup_printf (_("Moving files into \"%s\"..."), display_name);
      g_free (display_name);

      /* collect the target files and launch the job */
      fmb_application_collect_and_launch (application, parent, 
                                             "stock_folder-move", title,
                                             fmb_io_jobs_move_files, 
                                             source_file_list, target_file, 
                                             new_files_closure);

      /* free the title */
      g_free (title);
    }
}



static FmbJob *
unlink_stub (GList *source_path_list,
             GList *target_path_list)
{
  return fmb_io_jobs_unlink_files (source_path_list);
}



/**
 * fmb_application_unlink_files:
 * @application : a #FmbApplication.
 * @parent      : a #GdkScreen, a #GtkWidget or %NULL.
 * @file_list   : the list of #FmbFile<!---->s that should be deleted.
 * @permanently : whether to unlink the files permanently.
 *
 * Deletes all files in the @file_list and takes care of all user interaction.
 *
 * If the user pressed the shift key while triggering the delete action,
 * the files will be deleted permanently (after confirming the action),
 * otherwise the files will be moved to the trash.
 **/
void
fmb_application_unlink_files (FmbApplication *application,
                                 gpointer           parent,
                                 GList             *file_list,
                                 gboolean           permanently)
{
  GtkWidget *dialog;
  GtkWindow *window;
  GdkScreen *screen;
  GList     *path_list = NULL;
  GList     *lp;
  gchar     *message;
  guint      n_path_list = 0;
  gint       response;

  _fmb_return_if_fail (parent == NULL || GDK_IS_SCREEN (parent) || GTK_IS_WIDGET (parent));
  _fmb_return_if_fail (FMB_IS_APPLICATION (application));

  /* determine the paths for the files */
  for (lp = g_list_last (file_list); lp != NULL; lp = lp->prev, ++n_path_list)
    {
      /* prepend the path to the path list */
      path_list = fmb_g_file_list_prepend (path_list, fmb_file_get_file (lp->data));

      /* permanently delete if at least one of the file is not a local 
       * file (e.g. resides in the trash) or cannot be trashed */
      if (!fmb_file_is_local (lp->data) || !fmb_file_can_be_trashed (lp->data))
        permanently = TRUE;
    }

  /* nothing to do if we don't have any paths */
  if (G_UNLIKELY (n_path_list == 0))
    return;

  /* ask the user to confirm if deleting permanently */
  if (G_UNLIKELY (permanently))
    {
      /* parse the parent pointer */
      screen = fmb_util_parse_parent (parent, &window);

      /* generate the question to confirm the delete operation */
      if (G_LIKELY (n_path_list == 1))
        {
          message = g_strdup_printf (_("Are you sure that you want to\npermanently delete \"%s\"?"),
                                     fmb_file_get_display_name (FMB_FILE (file_list->data)));
        }
      else
        {
          message = g_strdup_printf (ngettext ("Are you sure that you want to permanently\ndelete the selected file?",
                                               "Are you sure that you want to permanently\ndelete the %u selected files?",
                                               n_path_list),
                                     n_path_list);
        }

      /* ask the user to confirm the delete operation */
      dialog = gtk_message_dialog_new (window,
                                       GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                       GTK_MESSAGE_QUESTION,
                                       GTK_BUTTONS_NONE,
                                       "%s", message);
      if (G_UNLIKELY (window == NULL && screen != NULL))
        gtk_window_set_screen (GTK_WINDOW (dialog), screen);
      gtk_dialog_add_buttons (GTK_DIALOG (dialog),
                              GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                              GTK_STOCK_DELETE, GTK_RESPONSE_YES,
                              NULL);
      gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_YES);
      gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog), 
                                                _("If you delete a file, it is permanently lost."));
      response = gtk_dialog_run (GTK_DIALOG (dialog));
      gtk_widget_destroy (dialog);
      g_free (message);

      /* perform the delete operation */
      if (G_LIKELY (response == GTK_RESPONSE_YES))
        {
          /* launch the "Delete" operation */
          fmb_application_launch (application, parent, "edit-delete",
                                     _("Deleting files..."), unlink_stub,
                                     path_list, path_list, NULL);
        }
    }
  else
    {
      /* launch the "Move to Trash" operation */
      fmb_application_trash (application, parent, path_list);
    }

  /* release the path list */
  fmb_g_file_list_free (path_list);
}



static FmbJob *
trash_stub (GList *source_file_list,
            GList *target_file_list)
{
  return fmb_io_jobs_trash_files (source_file_list);
}



void
fmb_application_trash (FmbApplication *application,
                          gpointer           parent,
                          GList             *file_list)
{
  _fmb_return_if_fail (parent == NULL || GDK_IS_SCREEN (parent) || GTK_IS_WIDGET (parent));
  _fmb_return_if_fail (FMB_IS_APPLICATION (application));
  _fmb_return_if_fail (file_list != NULL);

  fmb_application_launch (application, parent, "user-trash-full", 
                             _("Moving files into the trash..."), trash_stub,
                             file_list, NULL, NULL);
}



static FmbJob *
creat_stub (GList *template_file,
            GList *target_path_list)
{
   _fmb_return_val_if_fail (template_file->data == NULL || G_IS_FILE (template_file->data), NULL);
  return fmb_io_jobs_create_files (target_path_list, template_file->data);
}



/**
 * fmb_application_creat:
 * @application       : a #FmbApplication.
 * @parent            : a #GdkScreen, a #GtkWidget or %NULL.
 * @file_list         : the list of files to create.
 * @new_files_closure : a #GClosure to connect to the job's "new-files" signal,
 *                      which will be emitted when the job finishes with the
 *                      list of #GFile<!---->s created by the job, or
 *                      %NULL if you're not interested in the signal.
 *
 * Creates empty files for all #GFile<!---->s listed in @file_list. This
 * method takes care of all user interaction.
 **/
void
fmb_application_creat (FmbApplication *application,
                          gpointer           parent,
                          GList             *file_list,
                          GFile             *template_file,
                          GClosure          *new_files_closure)
{
  GList template_list;

  _fmb_return_if_fail (parent == NULL || GDK_IS_SCREEN (parent) || GTK_IS_WIDGET (parent));
  _fmb_return_if_fail (FMB_IS_APPLICATION (application));
  
  template_list.next = template_list.prev = NULL;
  template_list.data = template_file;

  /* launch the operation */
  fmb_application_launch (application, parent, "document-new",
                             _("Creating files..."), creat_stub,
                             &template_list, file_list, new_files_closure);
}



static FmbJob *
mkdir_stub (GList *source_path_list,
            GList *target_path_list)
{
  return fmb_io_jobs_make_directories (source_path_list);
}



/**
 * fmb_application_mkdir:
 * @application       : a #FmbApplication.
 * @parent            : a #GdkScreen, a #GtkWidget or %NULL.
 * @file_list         : the list of directories to create.
 * @new_files_closure : a #GClosure to connect to the job's "new-files" signal,
 *                      which will be emitted when the job finishes with the
 *                      list of #GFile<!---->s created by the job, or
 *                      %NULL if you're not interested in the signal.
 *
 * Creates all directories referenced by the @file_list. This method takes care of all user
 * interaction.
 **/
void
fmb_application_mkdir (FmbApplication *application,
                          gpointer           parent,
                          GList             *file_list,
                          GClosure          *new_files_closure)
{
  _fmb_return_if_fail (parent == NULL || GDK_IS_SCREEN (parent) || GTK_IS_WIDGET (parent));
  _fmb_return_if_fail (FMB_IS_APPLICATION (application));

  /* launch the operation */
  fmb_application_launch (application, parent, "folder-new",
                             _("Creating directories..."), mkdir_stub,
                             file_list, file_list, new_files_closure);
}



/**
 * fmb_application_empty_trash:
 * @application : a #FmbApplication.
 * @parent      : the parent, which can either be the associated
 *                #GtkWidget, the screen on which display the
 *                progress and the confirmation, or %NULL.
 *
 * Deletes all files and folders in the Trash after asking
 * the user to confirm the operation.
 **/
void
fmb_application_empty_trash (FmbApplication *application,
                                gpointer           parent,
                                const gchar       *startup_id)
{
  GtkWidget *dialog;
  GtkWindow *window;
  GdkScreen *screen;
  GList      file_list;
  gint       response;

  _fmb_return_if_fail (FMB_IS_APPLICATION (application));
  _fmb_return_if_fail (parent == NULL || GDK_IS_SCREEN (parent) || GTK_IS_WIDGET (parent));

  /* parse the parent pointer */
  screen = fmb_util_parse_parent (parent, &window);

  /* ask the user to confirm the operation */
  dialog = gtk_message_dialog_new (window,
                                   GTK_DIALOG_MODAL
                                   | GTK_DIALOG_DESTROY_WITH_PARENT,
                                   GTK_MESSAGE_QUESTION,
                                   GTK_BUTTONS_NONE,
                                   _("Remove all files and folders from the Trash?"));
  if (G_UNLIKELY (window == NULL && screen != NULL))
    gtk_window_set_screen (GTK_WINDOW (dialog), screen);
  gtk_window_set_startup_id (GTK_WINDOW (dialog), startup_id);
  gtk_dialog_add_buttons (GTK_DIALOG (dialog),
                          GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                          _("_Empty Trash"), GTK_RESPONSE_YES,
                          NULL);
  gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_YES);
  gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog),
                                            _("If you choose to empty the Trash, all items in it will be permanently lost. "
                                              "Please note that you can also delete them separately."));
  response = gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy (dialog);

  /* check if the user confirmed */
  if (G_LIKELY (response == GTK_RESPONSE_YES))
    {
      /* fake a path list with only the trash root (the root
       * folder itself will never be unlinked, so this is safe)
       */
      file_list.data = fmb_g_file_new_for_trash ();
      file_list.next = NULL;
      file_list.prev = NULL;

      /* launch the operation */
      fmb_application_launch (application, parent, "user-trash",
                                 _("Emptying the Trash..."),
                                 unlink_stub, &file_list, NULL, NULL);

      /* cleanup */
      g_object_unref (file_list.data);
    }
}



/**
 * fmb_application_restore_files:
 * @application       : a #FmbApplication.
 * @parent            : a #GdkScreen, a #GtkWidget or %NULL.
 * @trash_file_list   : a #GList of #FmbFile<!---->s in the trash.
 * @new_files_closure : a #GClosure to connect to the job's "new-files" signal,
 *                      which will be emitted when the job finishes with the
 *                      list of #GFile<!---->s created by the job, or
 *                      %NULL if you're not interested in the signal.
 *
 * Restores all #FmbFile<!---->s in the @trash_file_list to their original
 * location.
 **/
void
fmb_application_restore_files (FmbApplication *application,
                                  gpointer           parent,
                                  GList             *trash_file_list,
                                  GClosure          *new_files_closure)
{
  const gchar *original_uri;
  GError      *err = NULL;
  GFile       *target_path;
  GList       *source_path_list = NULL;
  GList       *target_path_list = NULL;
  GList       *lp;

  _fmb_return_if_fail (parent == NULL || GDK_IS_SCREEN (parent) || GTK_IS_WIDGET (parent));
  _fmb_return_if_fail (FMB_IS_APPLICATION (application));

  for (lp = trash_file_list; lp != NULL; lp = lp->next)
    {
      original_uri = fmb_file_get_original_path (lp->data);
      if (G_UNLIKELY (original_uri == NULL))
        {
          /* no OriginalPath, impossible to continue */
          g_set_error (&err, G_FILE_ERROR, G_FILE_ERROR_INVAL,
                       _("Failed to determine the original path for \"%s\""),
                       fmb_file_get_display_name (lp->data));
          break;
        }

      /* TODO we might have to distinguish between URIs and paths here */
      target_path = g_file_new_for_commandline_arg (original_uri);

      source_path_list = fmb_g_file_list_append (source_path_list, fmb_file_get_file (lp->data));
      target_path_list = fmb_g_file_list_append (target_path_list, target_path);

      g_object_unref (target_path);
    }

  if (G_UNLIKELY (err != NULL))
    {
      /* display an error dialog */
      fmb_dialogs_show_error (parent, err, _("Could not restore \"%s\""), 
                                 fmb_file_get_display_name (lp->data));
      g_error_free (err);
    }
  else
    {
      /* launch the operation */
      fmb_application_launch (application, parent, "stock_folder-move",
                                 _("Restoring files..."), fmb_io_jobs_restore_files,
                                 source_path_list, target_path_list, new_files_closure);
    }

  /* free path lists */
  fmb_g_file_list_free (source_path_list);
  fmb_g_file_list_free (target_path_list);
}



FmbThumbnailCache *
fmb_application_get_thumbnail_cache (FmbApplication *application)
{
  _fmb_return_val_if_fail (FMB_IS_APPLICATION (application), NULL);

  if (application->thumbnail_cache == NULL)
    application->thumbnail_cache = fmb_thumbnail_cache_new ();

  return g_object_ref (application->thumbnail_cache);
}



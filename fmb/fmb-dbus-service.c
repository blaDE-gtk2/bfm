/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2006      Benedikt Meurer <benny@xfce.org>
 * Copyright (c) 2009-2010 Jannis Pohlmann <jannis@xfce.org>
 * Copyright (c) 2010      Daniel Morales <daniel@daniel.com.uy>
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_MEMORY_H
#include <memory.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include <dbus/dbus-glib-lowlevel.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus.h>

#include <glib/gstdio.h>

#include <blxo/blxo.h>

#include <fmb/fmb-application.h>
#include <fmb/fmb-chooser-dialog.h>
#include <fmb/fmb-dbus-service.h>
#include <fmb/fmb-file.h>
#include <fmb/fmb-gdk-extensions.h>
#include <fmb/fmb-preferences-dialog.h>
#include <fmb/fmb-private.h>
#include <fmb/fmb-properties-dialog.h>
#include <fmb/fmb-util.h>


typedef enum
{
  FMB_DBUS_TRANSFER_MODE_COPY_TO,
  FMB_DBUS_TRANSFER_MODE_COPY_INTO,
  FMB_DBUS_TRANSFER_MODE_MOVE_INTO,
  FMB_DBUS_TRANSFER_MODE_LINK_INTO,
} FmbDBusTransferMode;


static void     fmb_dbus_service_finalize                    (GObject                *object);
static gboolean fmb_dbus_service_connect_trash_bin           (FmbDBusService      *dbus_service,
                                                                 GError                **error);
static gboolean fmb_dbus_service_parse_uri_and_display       (FmbDBusService      *dbus_service,
                                                                 const gchar            *uri,
                                                                 const gchar            *display,
                                                                 FmbFile            **file_return,
                                                                 GdkScreen             **screen_return,
                                                                 GError                **error);
static gboolean fmb_dbus_service_transfer_files              (FmbDBusTransferMode  transfer_mode,
                                                                 const gchar            *working_directory,
                                                                 const gchar * const    *source_filenames,
                                                                 const gchar * const    *target_filenames,
                                                                 const gchar            *display,
                                                                 const gchar            *startup_id,
                                                                 GError                **error);
static void     fmb_dbus_service_trash_bin_changed           (FmbDBusService      *dbus_service,
                                                                 FmbFile             *trash_bin);
static gboolean fmb_dbus_service_display_chooser_dialog      (FmbDBusService      *dbus_service,
                                                                 const gchar            *uri,
                                                                 gboolean                open,
                                                                 const gchar            *display,
                                                                 const gchar            *startup_id,
                                                                 GError                **error);
static gboolean fmb_dbus_service_display_folder              (FmbDBusService      *dbus_service,
                                                                 const gchar            *uri,
                                                                 const gchar            *display,
                                                                 const gchar            *startup_id,
                                                                 GError                **error);
static gboolean fmb_dbus_service_display_folder_and_select   (FmbDBusService      *dbus_service,
                                                                 const gchar            *uri,
                                                                 const gchar            *filename,
                                                                 const gchar            *display,
                                                                 const gchar            *startup_id,
                                                                 GError                **error);
static gboolean fmb_dbus_service_display_file_properties     (FmbDBusService      *dbus_service,
                                                                 const gchar            *uri,
                                                                 const gchar            *display,
                                                                 const gchar            *startup_id,
                                                                 GError                **error);
static gboolean fmb_dbus_service_launch                      (FmbDBusService      *dbus_service,
                                                                 const gchar            *uri,
                                                                 const gchar            *display,
                                                                 const gchar            *startup_id,
                                                                 GError                **error);
static gboolean fmb_dbus_service_execute                     (FmbDBusService      *dbus_service,
                                                                 const gchar            *working_directory,
                                                                 const gchar            *uri,
                                                                 const gchar           **files,
                                                                 const gchar            *display,
                                                                 const gchar            *startup_id,
                                                                 GError                **error);
static gboolean fmb_dbus_service_display_preferences_dialog  (FmbDBusService      *dbus_service,
                                                                 const gchar            *display,
                                                                 const gchar            *startup_id,
                                                                 GError                **error);
static gboolean fmb_dbus_service_display_trash               (FmbDBusService      *dbus_service,
                                                                 const gchar            *display,
                                                                 const gchar            *startup_id,
                                                                 GError                **error);
static gboolean fmb_dbus_service_empty_trash                 (FmbDBusService      *dbus_service,
                                                                 const gchar            *display,
                                                                 const gchar            *startup_id,
                                                                 GError                **error);
static gboolean fmb_dbus_service_move_to_trash               (FmbDBusService      *dbus_service,
                                                                 gchar                 **filenames,
                                                                 const gchar            *display,
                                                                 const gchar            *startup_id,
                                                                 GError                **error);
static gboolean fmb_dbus_service_query_trash                 (FmbDBusService      *dbus_service,
                                                                 gboolean               *empty,
                                                                 GError                **error);
static gboolean fmb_dbus_service_bulk_rename                 (FmbDBusService      *dbus_service,
                                                                 const gchar            *working_directory,
                                                                 gchar                 **filenames,
                                                                 gboolean                standalone,
                                                                 const gchar            *display,
                                                                 const gchar            *startup_id,
                                                                 GError                **error);
static gboolean fmb_dbus_service_launch_files                (FmbDBusService      *dbus_service,
                                                                 const gchar            *working_directory,
                                                                 gchar                 **filenames,
                                                                 const gchar            *display,
                                                                 const gchar            *startup_id,
                                                                 GError                **error);
static gboolean fmb_dbus_service_rename_file                 (FmbDBusService      *dbus_service,
                                                                 const gchar            *uri,
                                                                 const gchar            *display,
                                                                 const gchar            *startup_id,
                                                                 GError                **error);
static gboolean fmb_dbus_service_create_file                 (FmbDBusService      *dbus_service,
                                                                 const gchar            *parent_directory,
                                                                 const gchar            *content_type,
                                                                 const gchar            *display,
                                                                 const gchar            *startup_id,
                                                                 GError                **error);
static gboolean fmb_dbus_service_create_file_from_template   (FmbDBusService      *dbus_service,
                                                                 const gchar            *parent_directory,
                                                                 const gchar            *template_uri,
                                                                 const gchar            *display,
                                                                 const gchar            *startup_id,
                                                                 GError                **error);
static gboolean fmb_dbus_service_copy_to                     (FmbDBusService      *dbus_service,
                                                                 const gchar            *working_directory,
                                                                 gchar                 **source_filenames,
                                                                 gchar                 **target_filenames,
                                                                 const gchar            *display,
                                                                 const gchar            *startup_id,
                                                                 GError                **error);
static gboolean fmb_dbus_service_copy_into                   (FmbDBusService      *dbus_service,
                                                                 const gchar            *working_directory,
                                                                 gchar                 **source_filenames,
                                                                 const gchar            *target_filename,
                                                                 const gchar            *display,
                                                                 const gchar            *startup_id,
                                                                 GError                **error);
static gboolean fmb_dbus_service_move_into                   (FmbDBusService      *dbus_service,
                                                                 const gchar            *working_directory,
                                                                 gchar                 **source_filenames,
                                                                 const gchar            *target_filenames,
                                                                 const gchar            *display,
                                                                 const gchar            *startup_id,
                                                                 GError                **error);
static gboolean fmb_dbus_service_link_into                   (FmbDBusService      *dbus_service,
                                                                 const gchar            *working_directory,
                                                                 gchar                 **source_filenames,
                                                                 const gchar            *target_filename,
                                                                 const gchar            *display,
                                                                 const gchar            *startup_id,
                                                                 GError                **error);
static gboolean fmb_dbus_service_unlink_files                (FmbDBusService      *dbus_service,
                                                                 const gchar            *working_directory,
                                                                 gchar                 **filenames,
                                                                 const gchar            *display,
                                                                 const gchar            *startup_id,
                                                                 GError                **error);
static gboolean fmb_dbus_service_terminate                   (FmbDBusService      *dbus_service,
                                                                 GError                **error);



/* include generate dbus infos */
#include <fmb/fmb-dbus-service-infos.h>



struct _FmbDBusServiceClass
{
  GObjectClass __parent__;
};

struct _FmbDBusService
{
  GObject __parent__;

  DBusGConnection *connection;
  FmbFile      *trash_bin;
};



G_DEFINE_TYPE (FmbDBusService, fmb_dbus_service, G_TYPE_OBJECT)



static void
fmb_dbus_service_class_init (FmbDBusServiceClass *klass)
{
  GObjectClass *gobject_class;

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = fmb_dbus_service_finalize;

  /* install the D-BUS info for our class */
  dbus_g_object_type_install_info (G_TYPE_FROM_CLASS (klass), 
                                   &dbus_glib_fmb_dbus_service_object_info);

  /**
   * FmbDBusService::trash-changed:
   * @dbus_service : a #FmbDBusService.
   * @full         : %TRUE if the trash bin now contains at least
   *                 one item, %FALSE otherwise.
   *
   * This signal is emitted whenever the state of the trash bin
   * changes. Note that this signal is only emitted after the
   * trash has previously been queried by a D-BUS client.
   **/
  g_signal_new (I_("trash-changed"),
                G_TYPE_FROM_CLASS (klass),
                G_SIGNAL_RUN_LAST,
                0, NULL, NULL,
                g_cclosure_marshal_VOID__BOOLEAN,
                G_TYPE_NONE, 1, G_TYPE_BOOLEAN);
}



static void
fmb_dbus_service_init (FmbDBusService *dbus_service)
{
  GError         *error = NULL;
  DBusConnection *dbus_connection;
  gint            result;

  /* try to connect to the session bus */
  dbus_service->connection = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
  if (G_LIKELY (dbus_service->connection != NULL))
    {
      /* register the /org/blade/FileManager object for Fmb */
      dbus_g_connection_register_g_object (dbus_service->connection, "/org/blade/FileManager", G_OBJECT (dbus_service));

      /* request the org.blade.Fmb name for Fmb */
      dbus_connection = dbus_g_connection_get_connection (dbus_service->connection);
      result = dbus_bus_request_name (dbus_connection, "org.blade.Fmb",
                                      DBUS_NAME_FLAG_ALLOW_REPLACEMENT | DBUS_NAME_FLAG_DO_NOT_QUEUE, NULL);

      /* check if we successfully acquired the name */
      if (result != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER)
        {
          g_printerr ("Fmb: D-BUS name org.blade.Fmb already registered.\n");

          /* unset connection */
          dbus_g_connection_unref (dbus_service->connection);
          dbus_service->connection = NULL;

          return;
        }

      /* request the org.blade.FileManager name for Fmb */
      dbus_bus_request_name (dbus_connection, "org.blade.FileManager", DBUS_NAME_FLAG_REPLACE_EXISTING, NULL);

      /* once we registered, unset dbus variables (bug #8800) */
      g_unsetenv ("DBUS_STARTER_ADDRESS");
      g_unsetenv ("DBUS_STARTER_BUS_TYPE");
    }
  else
    {
      /* notify the user that D-BUS service won't be available */
      g_printerr ("Fmb: Failed to connect to the D-BUS session bus: %s\n", error->message);
      g_error_free (error);
    }
}



static void
fmb_dbus_service_finalize (GObject *object)
{
  FmbDBusService *dbus_service = FMB_DBUS_SERVICE (object);
  DBusConnection    *dbus_connection;

  /* release the D-BUS connection object */
  if (G_LIKELY (dbus_service->connection != NULL))
    {
      /* release the names */
      dbus_connection = dbus_g_connection_get_connection (dbus_service->connection);
      dbus_bus_release_name (dbus_connection, "org.blade.Fmb", NULL);
      dbus_bus_release_name (dbus_connection, "org.blade.FileManager", NULL);

      dbus_g_connection_unref (dbus_service->connection);
    }

  /* check if we are connected to the trash bin */
  if (G_LIKELY (dbus_service->trash_bin != NULL))
    {
      /* unwatch the trash bin */
      fmb_file_unwatch (dbus_service->trash_bin);

      /* release the trash bin */
      g_signal_handlers_disconnect_by_func (G_OBJECT (dbus_service->trash_bin), fmb_dbus_service_trash_bin_changed, dbus_service);
      g_object_unref (G_OBJECT (dbus_service->trash_bin));
    }

  (*G_OBJECT_CLASS (fmb_dbus_service_parent_class)->finalize) (object);
}



static gboolean
fmb_dbus_service_connect_trash_bin (FmbDBusService *dbus_service,
                                       GError           **error)
{
  GFile *trash_bin_path;

  /* check if we're not already connected to the trash bin */
  if (G_UNLIKELY (dbus_service->trash_bin == NULL))
    {
      /* try to connect to the trash bin */
      trash_bin_path = fmb_g_file_new_for_trash ();
      dbus_service->trash_bin = fmb_file_get (trash_bin_path, error);
      if (G_LIKELY (dbus_service->trash_bin != NULL))
        {
          /* watch the trash bin for changes */
          fmb_file_watch (dbus_service->trash_bin);

          /* stay informed about changes to the trash bin */
          g_signal_connect_swapped (G_OBJECT (dbus_service->trash_bin), "changed",
                                    G_CALLBACK (fmb_dbus_service_trash_bin_changed),
                                    dbus_service);
          fmb_file_reload_idle (dbus_service->trash_bin);
        }
      g_object_unref (trash_bin_path);
    }

  return (dbus_service->trash_bin != NULL);
}



static gboolean
fmb_dbus_service_parse_uri_and_display (FmbDBusService *dbus_service,
                                           const gchar       *uri,
                                           const gchar       *display,
                                           FmbFile       **file_return,
                                           GdkScreen        **screen_return,
                                           GError           **error)
{
  /* try to open the display */
  *screen_return = fmb_gdk_screen_open (display, error);
  if (G_UNLIKELY (*screen_return == NULL))
    return FALSE;

  /* try to determine the file for the URI */
  *file_return = fmb_file_get_for_uri (uri, error);
  if (G_UNLIKELY (*file_return == NULL))
    {
      g_object_unref (G_OBJECT (*screen_return));
      return FALSE;
    }

  return TRUE;
}



static void
fmb_dbus_service_trash_bin_changed (FmbDBusService *dbus_service,
                                       FmbFile        *trash_bin)
{
  _fmb_return_if_fail (FMB_IS_DBUS_SERVICE (dbus_service));
  _fmb_return_if_fail (dbus_service->trash_bin == trash_bin);
  _fmb_return_if_fail (FMB_IS_FILE (trash_bin));

  /* emit the "trash-changed" signal with the new state */
  g_signal_emit_by_name (G_OBJECT (dbus_service), "trash-changed", 
                         fmb_file_get_item_count (trash_bin) > 0);
}



static gboolean
fmb_dbus_service_display_chooser_dialog (FmbDBusService *dbus_service,
                                            const gchar       *uri,
                                            gboolean           open,
                                            const gchar       *display,
                                            const gchar       *startup_id,
                                            GError           **error)
{
  FmbFile *file;
  GdkScreen  *screen;

  /* parse uri and display parameters */
  if (!fmb_dbus_service_parse_uri_and_display (dbus_service, uri, display, &file, &screen, error))
    return FALSE;

  /* popup the chooser dialog */
  /* TODO use the startup id! */
  fmb_show_chooser_dialog (screen, file, open);

  /* cleanup */
  g_object_unref (G_OBJECT (screen));
  g_object_unref (G_OBJECT (file));

  return TRUE;
}



static gboolean
fmb_dbus_service_display_folder (FmbDBusService *dbus_service,
                                    const gchar       *uri,
                                    const gchar       *display,
                                    const gchar       *startup_id,
                                    GError           **error)
{
  FmbApplication *application;
  FmbFile        *file;
  GdkScreen         *screen;

  /* parse uri and display parameters */
  if (!fmb_dbus_service_parse_uri_and_display (dbus_service, uri, display, &file, &screen, error))
    return FALSE;

  /* popup a new window for the folder */
  application = fmb_application_get ();
  fmb_application_open_window (application, file, screen, startup_id);
  g_object_unref (G_OBJECT (application));

  /* cleanup */
  g_object_unref (G_OBJECT (screen));
  g_object_unref (G_OBJECT (file));

  return TRUE;
}



static gboolean
fmb_dbus_service_display_folder_and_select (FmbDBusService *dbus_service,
                                               const gchar       *uri,
                                               const gchar       *filename,
                                               const gchar       *display,
                                               const gchar       *startup_id,
                                               GError           **error)
{
  FmbApplication *application;
  FmbFile        *file;
  FmbFile        *folder;
  GdkScreen         *screen;
  GtkWidget         *window;
  GFile             *path;

  /* verify that filename is valid */
  if (G_UNLIKELY (filename == NULL || *filename == '\0' || strchr (filename, '/') != NULL))
    {
      g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_INVAL, _("Invalid filename \"%s\""), filename);
      return FALSE;
    }

  /* parse uri and display parameters */
  if (!fmb_dbus_service_parse_uri_and_display (dbus_service, uri, display, &folder, &screen, error))
    return FALSE;

  /* popup a new window for the folder */
  application = fmb_application_get ();
  window = fmb_application_open_window (application, folder, screen, startup_id);
  g_object_unref (application);

  /* determine the path for the filename relative to the folder */
  path = g_file_resolve_relative_path (fmb_file_get_file (folder), filename);
  if (G_LIKELY (path != NULL))
    {
      /* try to determine the file for the path */
      file = fmb_file_get (path, NULL);
      if (G_LIKELY (file != NULL))
        {
          /* tell the window to scroll to the given file and select it */
          fmb_window_scroll_to_file (FMB_WINDOW (window), file, TRUE, TRUE, 0.5f, 0.5f);

          /* release the file reference */
          g_object_unref (file);
        }

      /* release the path */
      g_object_unref (path);
    }

  /* cleanup */
  g_object_unref (screen);
  g_object_unref (folder);

  return TRUE;
}



static gboolean
fmb_dbus_service_display_file_properties (FmbDBusService *dbus_service,
                                             const gchar       *uri,
                                             const gchar       *display,
                                             const gchar       *startup_id,
                                             GError           **error)
{
  FmbApplication *application;
  FmbFile        *file;
  GdkScreen         *screen;
  GtkWidget         *dialog;

  /* parse uri and display parameters */
  if (!fmb_dbus_service_parse_uri_and_display (dbus_service, uri, display, &file, &screen, error))
    return FALSE;

  /* popup the file properties dialog */
  dialog = fmb_properties_dialog_new (NULL);
  gtk_window_set_screen (GTK_WINDOW (dialog), screen);
  gtk_window_set_startup_id (GTK_WINDOW (dialog), startup_id);
  fmb_properties_dialog_set_file (FMB_PROPERTIES_DIALOG (dialog), file);
  gtk_window_present (GTK_WINDOW (dialog));

  /* let the application take control over the dialog */
  application = fmb_application_get ();
  fmb_application_take_window (application, GTK_WINDOW (dialog));
  g_object_unref (G_OBJECT (application));

  /* cleanup */
  g_object_unref (G_OBJECT (screen));
  g_object_unref (G_OBJECT (file));

  return TRUE;
}



static gboolean
fmb_dbus_service_launch (FmbDBusService *dbus_service,
                            const gchar       *uri,
                            const gchar       *display,
                            const gchar       *startup_id,
                            GError           **error)
{
  FmbFile *file;
  GdkScreen  *screen;
  gboolean    result = FALSE;

  /* parse uri and display parameters */
  if (fmb_dbus_service_parse_uri_and_display (dbus_service, uri, display, &file, &screen, error))
    {
      /* try to launch the file on the given screen */
      result = fmb_file_launch (file, screen, startup_id, error);

      /* cleanup */
      g_object_unref (G_OBJECT (screen));
      g_object_unref (G_OBJECT (file));
    }

  return result;
}



static gboolean
fmb_dbus_service_execute (FmbDBusService *dbus_service,
                             const gchar       *working_directory,
                             const gchar       *uri,
                             const gchar      **files,
                             const gchar       *display,
                             const gchar       *startup_id,
                             GError           **error)
{
  FmbFile *file;
  GdkScreen  *screen;
  gboolean    result = FALSE;
  GFile      *working_dir;
  GList      *file_list = NULL;
  gchar      *tmp_working_dir = NULL;
  gchar      *old_working_dir = NULL;
  guint       n;

  /* parse uri and display parameters */
  if (fmb_dbus_service_parse_uri_and_display (dbus_service, uri, display, &file, &screen, error))
    {
      if (working_directory != NULL && *working_directory != '\0')
        old_working_dir = fmb_util_change_working_directory (working_directory);

      for (n = 0; files != NULL && files[n] != NULL; ++n)
        file_list = g_list_prepend (file_list, g_file_new_for_commandline_arg (files[n]));

      file_list = g_list_reverse (file_list);

      if (old_working_dir != NULL)
        {
          tmp_working_dir = fmb_util_change_working_directory (old_working_dir);
          g_free (tmp_working_dir);
          g_free (old_working_dir);
        }

      /* try to launch the file on the given screen */
      working_dir = g_file_new_for_commandline_arg (working_directory);
      result = fmb_file_execute (file, working_dir, screen, file_list, startup_id, error);
      g_object_unref (working_dir);

      /* cleanup */
      g_list_free_full (file_list, g_object_unref);
      g_object_unref (screen);
      g_object_unref (file);
    }

  return result;
}



static gboolean
fmb_dbus_service_display_preferences_dialog (FmbDBusService *dbus_service,
                                                const gchar       *display,
                                                const gchar       *startup_id,
                                                GError           **error)
{
  FmbApplication *application;
  GdkScreen         *screen;
  GtkWidget         *dialog;

  /* try to open the screen for the display name */
  screen = fmb_gdk_screen_open (display, error);
  if (G_UNLIKELY (screen == NULL))
    return FALSE;

  /* popup the preferences dialog... */
  dialog = fmb_preferences_dialog_new (NULL);
  gtk_window_set_screen (GTK_WINDOW (dialog), screen);
  gtk_window_set_startup_id (GTK_WINDOW (dialog), startup_id);
  gtk_widget_show (GTK_WIDGET (dialog));

  /* ...and let the application take care of it */
  application = fmb_application_get ();
  fmb_application_take_window (application, GTK_WINDOW (dialog));
  g_object_unref (G_OBJECT (application));

  /* cleanup */
  g_object_unref (G_OBJECT (screen));

  return TRUE;
}



static gboolean
fmb_dbus_service_display_trash (FmbDBusService *dbus_service,
                                   const gchar       *display,
                                   const gchar       *startup_id,
                                   GError           **error)
{
  FmbApplication *application;
  GdkScreen         *screen;

  /* connect to the trash bin on-demand */
  if (!fmb_dbus_service_connect_trash_bin (dbus_service, error))
    return FALSE;

  /* try to open the screen for the display name */
  screen = fmb_gdk_screen_open (display, error);
  if (G_LIKELY (screen != NULL))
    {
      /* tell the application to display the trash bin */
      application = fmb_application_get ();
      fmb_application_open_window (application, dbus_service->trash_bin, screen, startup_id);
      g_object_unref (G_OBJECT (application));

      /* release the screen */
      g_object_unref (G_OBJECT (screen));
      return TRUE;
    }

  return FALSE;
}



static gboolean
fmb_dbus_service_empty_trash (FmbDBusService *dbus_service,
                                 const gchar       *display,
                                 const gchar       *startup_id,
                                 GError           **error)
{
  FmbApplication *application;
  GdkScreen         *screen;

  /* try to open the screen for the display name */
  screen = fmb_gdk_screen_open (display, error);
  if (G_LIKELY (screen != NULL))
    {
      /* tell the application to empty the trash bin */
      application = fmb_application_get ();
      fmb_application_empty_trash (application, screen, startup_id);
      g_object_unref (G_OBJECT (application));

      /* release the screen */
      g_object_unref (G_OBJECT (screen));
      return TRUE;
    }

  return FALSE;
}



static gboolean
fmb_dbus_service_move_to_trash (FmbDBusService *dbus_service,
                                   gchar            **filenames,
                                   const gchar       *display,
                                   const gchar       *startup_id,
                                   GError           **error)
{
  FmbApplication *application;
  GFile             *file;
  GdkScreen         *screen;
  GError            *err = NULL;
  GList             *file_list = NULL;
  gchar             *filename;
  guint              n;

  /* try to open the screen for the display name */
  screen = fmb_gdk_screen_open (display, &err);
  if (G_LIKELY (screen != NULL))
    {
      /* try to parse the specified filenames */
      for (n = 0; err == NULL && filenames[n] != NULL; ++n)
        {
          /* decode the filename (D-BUS uses UTF-8) */
          filename = g_filename_from_utf8 (filenames[n], -1, NULL, NULL, &err);
          if (G_LIKELY (err == NULL))
            {
              /* determine the path for the filename */
              /* TODO Not sure this will work as expected */
              file = g_file_new_for_commandline_arg (filename);
              file_list = fmb_g_file_list_append (file_list, file);
              g_object_unref (file);
            }

          /* cleanup */
          g_free (filename);
        }

      /* check if we succeed */
      if (G_LIKELY (err == NULL))
        {
          /* tell the application to move the specified files to the trash */
          application = fmb_application_get ();
          fmb_application_trash (application, screen, file_list);
          g_object_unref (application);
        }

      /* cleanup */
      fmb_g_file_list_free (file_list);
      g_object_unref (screen);
    }

  /* check if we failed */
  if (G_UNLIKELY (err != NULL))
    {
      /* propagate the error */
      g_propagate_error (error, err);
      return FALSE;
    }

  return TRUE;
}



static gboolean
fmb_dbus_service_query_trash (FmbDBusService *dbus_service,
                                 gboolean          *full,
                                 GError           **error)
{
  /* connect to the trash bin on-demand */
  if (fmb_dbus_service_connect_trash_bin (dbus_service, error))
    {
      /* check whether the trash bin is not empty */
      *full = (fmb_file_get_item_count (dbus_service->trash_bin) > 0);
      return TRUE;
    }

  return FALSE;
}



static gboolean
fmb_dbus_service_bulk_rename (FmbDBusService *dbus_service,
                                 const gchar       *working_directory,
                                 gchar            **filenames,
                                 gboolean           standalone,
                                 const gchar       *display,
                                 const gchar       *startup_id,
                                 GError           **error)
{
  FmbApplication *application;
  GdkScreen         *screen;
  gboolean           result = FALSE;
  gchar             *cwd;

  /* determine a proper working directory */
  cwd = (working_directory != NULL && *working_directory != '\0')
      ? g_strdup (working_directory)
      : g_get_current_dir ();

  /* try to open the screen for the display name */
  screen = fmb_gdk_screen_open (display, error);
  if (G_LIKELY (screen != NULL))
    {
      /* tell the application to display the bulk rename dialog */
      application = fmb_application_get ();
      result = fmb_application_bulk_rename (application, cwd, filenames, standalone, screen, startup_id, error);
      g_object_unref (G_OBJECT (application));

      /* release the screen */
      g_object_unref (G_OBJECT (screen));
    }

  /* release the cwd */
  g_free (cwd);

  return result;
}



static gboolean
fmb_dbus_service_launch_files (FmbDBusService *dbus_service,
                                  const gchar       *working_directory,
                                  gchar            **filenames,
                                  const gchar       *display,
                                  const gchar       *startup_id,
                                  GError           **error)
{
  FmbApplication *application;
  GdkScreen         *screen;
  gboolean           result = FALSE;

  /* verify that a valid working directory is given */
  if (G_UNLIKELY (!g_path_is_absolute (working_directory)))
    {
      /* LaunchFiles() invoked without a valid working directory */
      g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_INVAL, _("The working directory must be an absolute path"));
      return FALSE;
    }

  /* verify that at least one filename is given */
  if (G_UNLIKELY (filenames == NULL || *filenames == NULL))
    {
      /* LaunchFiles() invoked with an empty filename list */
      g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_INVAL, _("At least one filename must be specified"));
      return FALSE;
    }

  /* try to open the screen for the display name */
  screen = fmb_gdk_screen_open (display, error);
  if (G_LIKELY (screen != NULL))
    {
      /* let the application process the filenames */
      application = fmb_application_get ();
      result = fmb_application_process_filenames (application, working_directory, filenames, screen, startup_id, error);
      g_object_unref (G_OBJECT (application));

      /* release the screen */
      g_object_unref (G_OBJECT (screen));
    }

  return result;
}



static gboolean
fmb_dbus_service_rename_file (FmbDBusService *dbus_service,
                                 const gchar       *uri,
                                 const gchar       *display,
                                 const gchar       *startup_id,
                                 GError           **error)
{
  FmbApplication *application;
  FmbFile        *file;
  GdkScreen         *screen;

  /* parse uri and display parameters */
  if (!fmb_dbus_service_parse_uri_and_display (dbus_service, uri, display, &file, &screen, error))
    return FALSE;

  /* popup a new window for the folder */
  application = fmb_application_get ();
  fmb_application_rename_file (application, file, screen, startup_id);
  g_object_unref (G_OBJECT (application));

  /* cleanup */
  g_object_unref (G_OBJECT (screen));
  g_object_unref (G_OBJECT (file));

  return TRUE;
}



static gboolean
fmb_dbus_service_create_file (FmbDBusService *dbus_service,
                                 const gchar       *parent_directory,
                                 const gchar       *content_type,
                                 const gchar       *display,
                                 const gchar       *startup_id,
                                 GError           **error)
{
  FmbApplication *application;
  FmbFile        *file;
  GdkScreen         *screen;

  /* parse uri and display parameters */
  if (!fmb_dbus_service_parse_uri_and_display (dbus_service, parent_directory, display, &file, &screen, error))
    return FALSE;

  /* fall back to plain text file if no content type is provided */
  if (content_type == NULL || *content_type == '\0')
    content_type = "text/plain";

  /* popup a new window for the folder */
  application = fmb_application_get ();
  fmb_application_create_file (application, file, content_type, screen, startup_id);
  g_object_unref (G_OBJECT (application));

  /* cleanup */
  g_object_unref (G_OBJECT (screen));
  g_object_unref (G_OBJECT (file));

  return TRUE;
}



static gboolean
fmb_dbus_service_create_file_from_template (FmbDBusService *dbus_service,
                                               const gchar       *parent_directory,
                                               const gchar       *template_uri,
                                               const gchar       *display,
                                               const gchar       *startup_id,
                                               GError           **error)
{
  FmbApplication *application;
  FmbFile        *file;
  FmbFile        *template_file;
  GdkScreen         *screen;

  /* parse uri and display parameters */
  if (!fmb_dbus_service_parse_uri_and_display (dbus_service, parent_directory, display, &file, &screen, error))
    return FALSE;

  /* try to determine the file for the template URI */
  template_file = fmb_file_get_for_uri (template_uri, error);
  if(template_file == NULL)
    return FALSE;

  /* popup a new window for the folder */
  application = fmb_application_get ();
  fmb_application_create_file_from_template (application, file, template_file, screen, startup_id);
  g_object_unref (G_OBJECT (application));

  /* cleanup */
  g_object_unref (G_OBJECT (screen));
  g_object_unref (G_OBJECT (file));

  return TRUE;
}



static gboolean
fmb_dbus_service_transfer_files (FmbDBusTransferMode transfer_mode,
                                    const gchar           *working_directory,
                                    const gchar * const   *source_filenames,
                                    const gchar * const   *target_filenames,
                                    const gchar           *display,
                                    const gchar           *startup_id,
                                    GError               **error)
{
  FmbApplication *application;
  GdkScreen         *screen;
  GError            *err = NULL;
  GFile             *file;
  GList             *source_file_list = NULL;
  GList             *target_file_list = NULL;
  gchar             *filename;
  gchar             *new_working_dir = NULL;
  gchar             *old_working_dir = NULL;
  guint              n;

  /* verify that at least one file to transfer is given */
  if (source_filenames == NULL || *source_filenames == NULL)
    {
      g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_INVAL, 
                   _("At least one source filename must be specified"));
      return FALSE;
    }

  /* verify that the target filename is set / enough target filenames are given */
  if (transfer_mode == FMB_DBUS_TRANSFER_MODE_COPY_TO) 
    {
      if (g_strv_length ((gchar **)source_filenames) != g_strv_length ((gchar **)target_filenames))
        {
          g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_INVAL,
                       _("The number of source and target filenames must be the same"));
          return FALSE;
        }
    }
  else
    {
      if (target_filenames == NULL || *target_filenames == NULL)
        {
          g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_INVAL,
                       _("A destination directory must be specified"));
          return FALSE;
        }
    }

  /* try to open the screen for the display name */
  screen = fmb_gdk_screen_open (display, &err);
  if (screen != NULL)
    {
      /* change the working directory if necessary */
      if (!blxo_str_is_empty (working_directory))
        old_working_dir = fmb_util_change_working_directory (working_directory);

      /* transform the source filenames into GFile objects */
      for (n = 0; err == NULL && source_filenames[n] != NULL; ++n)
        {
          filename = g_filename_from_utf8 (source_filenames[n], -1, NULL, NULL, &err);
          if (filename != NULL)
            {
              file = g_file_new_for_commandline_arg (filename);
              source_file_list = fmb_g_file_list_append (source_file_list, file);
              g_object_unref (file);
              g_free (filename);
            }
        }

      /* transform the target filename(s) into (a) GFile object(s) */
      for (n = 0; err == NULL && target_filenames[n] != NULL; ++n)
        {
          filename = g_filename_from_utf8 (target_filenames[n], -1, NULL, NULL, &err);
          if (filename != NULL)
            {
              file = g_file_new_for_commandline_arg (filename);
              target_file_list = fmb_g_file_list_append (target_file_list, file);
              g_object_unref (file);
              g_free (filename);
            }
        }

      /* switch back to the previous working directory */
      if (!blxo_str_is_empty (working_directory))
        {
          new_working_dir = fmb_util_change_working_directory (old_working_dir);
          g_free (old_working_dir);
          g_free (new_working_dir);
        }

      if (err == NULL)
        {
          /* let the application process the filenames */
          application = fmb_application_get ();
          switch (transfer_mode)
            {
            case FMB_DBUS_TRANSFER_MODE_COPY_TO:
              fmb_application_copy_to (application, screen, 
                                          source_file_list, target_file_list, 
                                          NULL);
              break;
            case FMB_DBUS_TRANSFER_MODE_COPY_INTO:
              fmb_application_copy_into (application, screen, 
                                            source_file_list, target_file_list->data, 
                                            NULL);
              break;
            case FMB_DBUS_TRANSFER_MODE_MOVE_INTO:
              fmb_application_move_into (application, screen, 
                                            source_file_list, target_file_list->data, 
                                            NULL);
              break;
            case FMB_DBUS_TRANSFER_MODE_LINK_INTO:
              fmb_application_link_into (application, screen, 
                                            source_file_list, target_file_list->data, 
                                            NULL);
              break;
            }
          g_object_unref (application);
        }

      /* free the file lists */
      fmb_g_file_list_free (source_file_list);
      fmb_g_file_list_free (target_file_list);

      /* release the screen */
      g_object_unref (screen);
    }

  if (err != NULL)
    {
      g_propagate_error (error, err);
      return FALSE;
    }

  return TRUE;
}



static gboolean
fmb_dbus_service_copy_to (FmbDBusService *dbus_service,
                             const gchar       *working_directory,
                             gchar            **source_filenames,
                             gchar            **target_filenames,
                             const gchar       *display,
                             const gchar       *startup_id,
                             GError           **error)
{
  return fmb_dbus_service_transfer_files (FMB_DBUS_TRANSFER_MODE_COPY_TO,
                                             working_directory,
                                             (const gchar * const *)source_filenames,
                                             (const gchar * const *)target_filenames,
                                             display,
                                             startup_id,
                                             error);
}



static gboolean
fmb_dbus_service_copy_into (FmbDBusService *dbus_service,
                               const gchar       *working_directory,
                               gchar            **source_filenames,
                               const gchar       *target_filename,
                               const gchar       *display,
                               const gchar       *startup_id,
                               GError           **error)
{
  const gchar *target_filenames[2] = { target_filename, NULL };

  return fmb_dbus_service_transfer_files (FMB_DBUS_TRANSFER_MODE_COPY_INTO,
                                             working_directory,
                                             (const gchar * const *)source_filenames,
                                             target_filenames,
                                             display,
                                             startup_id,
                                             error);
}



static gboolean
fmb_dbus_service_move_into (FmbDBusService *dbus_service,
                               const gchar       *working_directory,
                               gchar            **source_filenames,
                               const gchar       *target_filename,
                               const gchar       *display,
                               const gchar       *startup_id,
                               GError           **error)
{
  const gchar *target_filenames[2] = { target_filename, NULL };

  return fmb_dbus_service_transfer_files (FMB_DBUS_TRANSFER_MODE_MOVE_INTO,
                                             working_directory,
                                             (const gchar * const *)source_filenames,
                                             target_filenames,
                                             display,
                                             startup_id,
                                             error);
}



static gboolean
fmb_dbus_service_link_into (FmbDBusService *dbus_service,
                               const gchar       *working_directory,
                               gchar            **source_filenames,
                               const gchar       *target_filename,
                               const gchar       *display,
                               const gchar       *startup_id,
                               GError           **error)
{
  const gchar *target_filenames[2] = { target_filename, NULL };

  return fmb_dbus_service_transfer_files (FMB_DBUS_TRANSFER_MODE_LINK_INTO,
                                             working_directory,
                                             (const gchar * const *)source_filenames,
                                             target_filenames,
                                             display,
                                             startup_id,
                                             error);
}


static gboolean
fmb_dbus_service_unlink_files (FmbDBusService  *dbus_service,
                                  const gchar        *working_directory,
                                  gchar             **filenames,
                                  const gchar        *display,
                                  const gchar        *startup_id,
                                  GError            **error)
{
  FmbApplication *application;
  FmbFile        *fmb_file;
  GFile             *file;
  GdkScreen         *screen;
  GError            *err = NULL;
  GList             *file_list = NULL;
  gchar             *filename;
  gchar             *new_working_dir = NULL;
  gchar             *old_working_dir = NULL;
  guint              n;

  /* verify that atleast one filename is given */
  if (filenames == NULL || *filenames == NULL)
    {
      g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_INVAL, _("At least one filename must be specified"));
      return FALSE;
    }

  /* try to open the screen for the display name */
  screen = fmb_gdk_screen_open (display, &err);
  if (screen != NULL)
    {
      /* change the working directory if necessary */
      if (!blxo_str_is_empty (working_directory))
        old_working_dir = fmb_util_change_working_directory (working_directory);

      /* try to parse the specified filenames */
      for (n = 0; err == NULL && filenames[n] != NULL; ++n)
        {
          /* decode the filename (D-BUS uses UTF-8) */
          filename = g_filename_from_utf8 (filenames[n], -1, NULL, NULL, &err);
          if (filename != NULL)
            {
              /* determine the path for the filename */
              file = g_file_new_for_commandline_arg (filename);
              fmb_file = fmb_file_get (file, &err);

              if (fmb_file != NULL)
                file_list = g_list_append (file_list, fmb_file);

              g_object_unref (file);
            }

          /* cleanup */
          g_free (filename);
        }

      /* switch back to the previous working directory */
      if (!blxo_str_is_empty (working_directory))
        {
          new_working_dir = fmb_util_change_working_directory (old_working_dir);
          g_free (old_working_dir);
          g_free (new_working_dir);
        }

      /* check if we succeeded */
      if (err == NULL && file_list != NULL)
        {
          /* tell the application to move the specified files to the trash */
          application = fmb_application_get ();
          fmb_application_unlink_files (application, screen, file_list, TRUE);
          g_object_unref (application);
        }

      /* cleanup */
      fmb_g_file_list_free (file_list);
      g_object_unref (screen);
    }

  if (err != NULL)
    {
      g_propagate_error (error, err);
      return FALSE;
    }

  return TRUE;
}



static gboolean
fmb_dbus_service_terminate (FmbDBusService *dbus_service,
                               GError           **error)
{
  /* leave the Gtk main loop as soon as possible */
  gtk_main_quit ();

  /* we cannot fail */
  return TRUE;
}



gboolean
fmb_dbus_service_has_connection (FmbDBusService *dbus_service)
{
  _fmb_return_val_if_fail (FMB_IS_DBUS_SERVICE (dbus_service), FALSE);
  return dbus_service->connection != NULL;
}

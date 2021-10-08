/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2005-2006 Benedikt Meurer <benny@xfce.org>
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_MEMORY_H
#include <memory.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include <fmb/fmb-application.h>
#include <fmb/fmb-browser.h>
#include <fmb/fmb-chooser-dialog.h>
#include <fmb/fmb-dialogs.h>
#include <fmb/fmb-gio-extensions.h>
#include <fmb/fmb-gobject-extensions.h>
#include <fmb/fmb-gtk-extensions.h>
#include <fmb/fmb-launcher.h>
#include <fmb/fmb-launcher-ui.h>
#include <fmb/fmb-private.h>
#include <fmb/fmb-sendto-model.h>
#include <fmb/fmb-stock.h>
#include <fmb/fmb-device-monitor.h>
#include <fmb/fmb-window.h>



typedef struct _FmbLauncherMountData FmbLauncherMountData;
typedef struct _FmbLauncherPokeData FmbLauncherPokeData;



/* Property identifiers */
enum
{
  PROP_0,
  PROP_CURRENT_DIRECTORY,
  PROP_SELECTED_FILES,
  PROP_UI_MANAGER,
  PROP_WIDGET,
  N_PROPERTIES
};



static void                    fmb_launcher_component_init             (FmbComponentIface     *iface);
static void                    fmb_launcher_navigator_init             (FmbNavigatorIface     *iface);
static void                    fmb_launcher_dispose                    (GObject                  *object);
static void                    fmb_launcher_finalize                   (GObject                  *object);
static void                    fmb_launcher_get_property               (GObject                  *object,
                                                                           guint                     prop_id,
                                                                           GValue                   *value,
                                                                           GParamSpec               *pspec);
static void                    fmb_launcher_set_property               (GObject                  *object,
                                                                           guint                     prop_id,
                                                                           const GValue             *value,
                                                                           GParamSpec               *pspec);
static FmbFile             *fmb_launcher_get_current_directory      (FmbNavigator          *navigator);
static void                    fmb_launcher_set_current_directory      (FmbNavigator          *navigator,
                                                                           FmbFile               *current_directory);
static GList                  *fmb_launcher_get_selected_files         (FmbComponent          *component);
static void                    fmb_launcher_set_selected_files         (FmbComponent          *component,
                                                                           GList                    *selected_files);
static GtkUIManager           *fmb_launcher_get_ui_manager             (FmbComponent          *component);
static void                    fmb_launcher_set_ui_manager             (FmbComponent          *component,
                                                                           GtkUIManager             *ui_manager);
static void                    fmb_launcher_execute_files              (FmbLauncher           *launcher,
                                                                           GList                    *files);
static void                    fmb_launcher_open_files                 (FmbLauncher           *launcher,
                                                                           GList                    *files);
static void                    fmb_launcher_open_paths                 (GAppInfo                 *app_info,
                                                                           GList                    *file_list,
                                                                           FmbLauncher           *launcher);
static void                    fmb_launcher_open_windows               (FmbLauncher           *launcher,
                                                                           GList                    *directories);
static void                    fmb_launcher_update                     (FmbLauncher           *launcher);
static void                    fmb_launcher_action_open                (GtkAction                *action,
                                                                           FmbLauncher           *launcher);
static void                    fmb_launcher_action_open_with_other     (GtkAction                *action,
                                                                           FmbLauncher           *launcher);
static void                    fmb_launcher_action_open_in_new_window  (GtkAction                *action,
                                                                           FmbLauncher           *launcher);
static void                    fmb_launcher_action_open_in_new_tab     (GtkAction                *action,
                                                                           FmbLauncher           *launcher);
static void                    fmb_launcher_action_sendto_desktop      (GtkAction                *action,
                                                                           FmbLauncher           *launcher);
static void                    fmb_launcher_action_sendto_device       (GtkAction                *action,
                                                                           FmbLauncher           *launcher);
static void                    fmb_launcher_widget_destroyed           (FmbLauncher           *launcher,
                                                                           GtkWidget                *widget);
static gboolean                fmb_launcher_sendto_idle                (gpointer                  user_data);
static void                    fmb_launcher_sendto_idle_destroy        (gpointer                  user_data);
static void                    fmb_launcher_mount_data_free            (FmbLauncherMountData  *data);
static void                    fmb_launcher_poke_files                 (FmbLauncher           *launcher,
                                                                           FmbLauncherPokeData   *poke_data);
static void                    fmb_launcher_poke_files_finish          (FmbBrowser            *browser,
                                                                           FmbFile               *file,
                                                                           FmbFile               *target_file,
                                                                           GError                   *error,
                                                                           gpointer                  user_data);
static FmbLauncherPokeData *fmb_launcher_poke_data_new              (GList                    *files);
static void                    fmb_launcher_poke_data_free             (FmbLauncherPokeData   *data);
static GtkWidget              *fmb_launcher_get_widget                 (const FmbLauncher     *launcher);


struct _FmbLauncherClass
{
  GObjectClass __parent__;
};

struct _FmbLauncher
{
  GObject __parent__;

  FmbFile             *current_directory;
  GList                  *selected_files;

  guint                   launcher_idle_id;

  GtkIconFactory         *icon_factory;
  GtkActionGroup         *action_group;
  GtkUIManager           *ui_manager;
  guint                   ui_merge_id;
  guint                   ui_addons_merge_id;

  GtkAction              *action_open;
  GtkAction              *action_open_with_other;
  GtkAction              *action_open_in_new_window;
  GtkAction              *action_open_in_new_tab;
  GtkAction              *action_open_with_other_in_menu;

  GtkWidget              *widget;

  FmbDeviceMonitor    *device_monitor;
  FmbSendtoModel      *sendto_model;
  guint                   sendto_idle_id;
};

struct _FmbLauncherMountData
{
  FmbLauncher *launcher;
  GList          *files;
};

struct _FmbLauncherPokeData
{
  GList *files;
  GList *resolved_files;
  guint  directories_in_tabs : 1;
};



static const GtkActionEntry action_entries[] =
{
  { "open", GTK_STOCK_OPEN, N_ ("_Open"), "<control>O", NULL, G_CALLBACK (fmb_launcher_action_open), },
  { "open-in-new-tab", NULL, N_ ("Open in New _Tab"), "<control><shift>P", NULL, G_CALLBACK (fmb_launcher_action_open_in_new_tab), },
  { "open-in-new-window", NULL, N_ ("Open in New _Window"), "<control><shift>O", NULL, G_CALLBACK (fmb_launcher_action_open_in_new_window), },
  { "open-with-other", NULL, N_ ("Open With Other _Application..."), NULL, N_ ("Choose another application with which to open the selected file"), G_CALLBACK (fmb_launcher_action_open_with_other), },
  { "open-with-menu", NULL, N_ ("Open With"), NULL, NULL, NULL, },
  { "open-with-other-in-menu", NULL, N_ ("Open With Other _Application..."), NULL, N_ ("Choose another application with which to open the selected file"), G_CALLBACK (fmb_launcher_action_open_with_other), },
  { "sendto-desktop", FMB_STOCK_DESKTOP, "", NULL, NULL, G_CALLBACK (fmb_launcher_action_sendto_desktop), },
};

static GQuark fmb_launcher_handler_quark;



static GParamSpec *launcher_props[N_PROPERTIES] = { NULL, };



G_DEFINE_TYPE_WITH_CODE (FmbLauncher, fmb_launcher, G_TYPE_OBJECT,
    G_IMPLEMENT_INTERFACE (FMB_TYPE_BROWSER, NULL)
    G_IMPLEMENT_INTERFACE (FMB_TYPE_NAVIGATOR, fmb_launcher_navigator_init)
    G_IMPLEMENT_INTERFACE (FMB_TYPE_COMPONENT, fmb_launcher_component_init))



static void
fmb_launcher_class_init (FmbLauncherClass *klass)
{
  GObjectClass *gobject_class;
  gpointer      g_iface;

  /* determine the "fmb-launcher-handler" quark */
  fmb_launcher_handler_quark = g_quark_from_static_string ("fmb-launcher-handler");

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->dispose = fmb_launcher_dispose;
  gobject_class->finalize = fmb_launcher_finalize;
  gobject_class->get_property = fmb_launcher_get_property;
  gobject_class->set_property = fmb_launcher_set_property;

  /**
   * FmbLauncher:widget:
   *
   * The #GtkWidget with which this launcher is associated.
   **/
  launcher_props[PROP_WIDGET] =
      g_param_spec_object ("widget",
                           "widget",
                           "widget",
                           GTK_TYPE_WIDGET,
                           BLXO_PARAM_READWRITE);

  /* Override FmbNavigator's properties */
  g_iface = g_type_default_interface_peek (FMB_TYPE_NAVIGATOR);
  launcher_props[PROP_CURRENT_DIRECTORY] =
      g_param_spec_override ("current-directory",
                             g_object_interface_find_property (g_iface, "current-directory"));

  /* Override FmbComponent's properties */
  g_iface = g_type_default_interface_peek (FMB_TYPE_COMPONENT);
  launcher_props[PROP_SELECTED_FILES] =
      g_param_spec_override ("selected-files",
                             g_object_interface_find_property (g_iface, "selected-files"));

  launcher_props[PROP_UI_MANAGER] =
      g_param_spec_override ("ui-manager",
                             g_object_interface_find_property (g_iface, "ui-manager"));

  /* install properties */
  g_object_class_install_properties (gobject_class, N_PROPERTIES, launcher_props);
}



static void
fmb_launcher_component_init (FmbComponentIface *iface)
{
  iface->get_selected_files = fmb_launcher_get_selected_files;
  iface->set_selected_files = fmb_launcher_set_selected_files;
  iface->get_ui_manager = fmb_launcher_get_ui_manager;
  iface->set_ui_manager = fmb_launcher_set_ui_manager;
}



static void
fmb_launcher_navigator_init (FmbNavigatorIface *iface)
{
  iface->get_current_directory = fmb_launcher_get_current_directory;
  iface->set_current_directory = fmb_launcher_set_current_directory;
}



static void
fmb_launcher_init (FmbLauncher *launcher)
{
  /* setup the action group for the launcher actions */
  launcher->action_group = gtk_action_group_new ("FmbLauncher");
  gtk_action_group_set_translation_domain (launcher->action_group, GETTEXT_PACKAGE);
  gtk_action_group_add_actions (launcher->action_group, action_entries, G_N_ELEMENTS (action_entries), launcher);

  /* determine references to our actions */
  launcher->action_open = gtk_action_group_get_action (launcher->action_group, "open");
  launcher->action_open_with_other = gtk_action_group_get_action (launcher->action_group, "open-with-other");
  launcher->action_open_in_new_window = gtk_action_group_get_action (launcher->action_group, "open-in-new-window");
  launcher->action_open_in_new_tab = gtk_action_group_get_action (launcher->action_group, "open-in-new-tab");
  launcher->action_open_with_other_in_menu = gtk_action_group_get_action (launcher->action_group, "open-with-other-in-menu");

  /* initialize and add our custom icon factory for the application/action icons */
  launcher->icon_factory = gtk_icon_factory_new ();
  gtk_icon_factory_add_default (launcher->icon_factory);

  /* setup the "Send To" support */
  launcher->sendto_model = fmb_sendto_model_get_default ();

  /* the "Send To" menu also displays removable devices from the device monitor */
  launcher->device_monitor = fmb_device_monitor_get ();
  g_signal_connect_swapped (launcher->device_monitor, "device-added", G_CALLBACK (fmb_launcher_update), launcher);
  g_signal_connect_swapped (launcher->device_monitor, "device-removed", G_CALLBACK (fmb_launcher_update), launcher);
}



static void
fmb_launcher_dispose (GObject *object)
{
  FmbLauncher *launcher = FMB_LAUNCHER (object);

  /* reset our properties */
  fmb_navigator_set_current_directory (FMB_NAVIGATOR (launcher), NULL);
  fmb_component_set_ui_manager (FMB_COMPONENT (launcher), NULL);
  fmb_launcher_set_widget (FMB_LAUNCHER (launcher), NULL);

  /* disconnect from the currently selected files */
  fmb_g_file_list_free (launcher->selected_files);
  launcher->selected_files = NULL;

  (*G_OBJECT_CLASS (fmb_launcher_parent_class)->dispose) (object);
}



static void
fmb_launcher_finalize (GObject *object)
{
  FmbLauncher *launcher = FMB_LAUNCHER (object);

  /* be sure to cancel the sendto idle source */
  if (G_UNLIKELY (launcher->sendto_idle_id != 0))
    g_source_remove (launcher->sendto_idle_id);

  /* be sure to cancel the launcher idle source */
  if (G_UNLIKELY (launcher->launcher_idle_id != 0))
    g_source_remove (launcher->launcher_idle_id);

  /* drop our custom icon factory for the application/action icons */
  gtk_icon_factory_remove_default (launcher->icon_factory);
  g_object_unref (launcher->icon_factory);

  /* release the reference on the action group */
  g_object_unref (launcher->action_group);

  /* disconnect from the device monitor used for the "Send To" menu */
  g_signal_handlers_disconnect_by_func (launcher->device_monitor, fmb_launcher_update, launcher);
  g_object_unref (launcher->device_monitor);

  /* release the reference on the sendto model */
  g_object_unref (launcher->sendto_model);

  (*G_OBJECT_CLASS (fmb_launcher_parent_class)->finalize) (object);
}



static void
fmb_launcher_get_property (GObject    *object,
                              guint       prop_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
  switch (prop_id)
    {
    case PROP_CURRENT_DIRECTORY:
      g_value_set_object (value, fmb_navigator_get_current_directory (FMB_NAVIGATOR (object)));
      break;

    case PROP_SELECTED_FILES:
      g_value_set_boxed (value, fmb_component_get_selected_files (FMB_COMPONENT (object)));
      break;

    case PROP_UI_MANAGER:
      g_value_set_object (value, fmb_component_get_ui_manager (FMB_COMPONENT (object)));
      break;

    case PROP_WIDGET:
      g_value_set_object (value, fmb_launcher_get_widget (FMB_LAUNCHER (object)));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static void
fmb_launcher_set_property (GObject      *object,
                              guint         prop_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
  switch (prop_id)
    {
    case PROP_CURRENT_DIRECTORY:
      fmb_navigator_set_current_directory (FMB_NAVIGATOR (object), g_value_get_object (value));
      break;

    case PROP_SELECTED_FILES:
      fmb_component_set_selected_files (FMB_COMPONENT (object), g_value_get_boxed (value));
      break;

    case PROP_UI_MANAGER:
      fmb_component_set_ui_manager (FMB_COMPONENT (object), g_value_get_object (value));
      break;

    case PROP_WIDGET:
      fmb_launcher_set_widget (FMB_LAUNCHER (object), g_value_get_object (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static FmbFile*
fmb_launcher_get_current_directory (FmbNavigator *navigator)
{
  return FMB_LAUNCHER (navigator)->current_directory;
}



static void
fmb_launcher_set_current_directory (FmbNavigator *navigator,
                                       FmbFile      *current_directory)
{
  FmbLauncher *launcher = FMB_LAUNCHER (navigator);

  /* disconnect from the previous directory */
  if (G_LIKELY (launcher->current_directory != NULL))
    g_object_unref (G_OBJECT (launcher->current_directory));

  /* activate the new directory */
  launcher->current_directory = current_directory;

  /* connect to the new directory */
  if (G_LIKELY (current_directory != NULL))
    g_object_ref (G_OBJECT (current_directory));

  /* notify listeners */
  g_object_notify_by_pspec (G_OBJECT (launcher), launcher_props[PROP_CURRENT_DIRECTORY]);
}



static GList*
fmb_launcher_get_selected_files (FmbComponent *component)
{
  return FMB_LAUNCHER (component)->selected_files;
}



static void
fmb_launcher_set_selected_files (FmbComponent *component,
                                    GList           *selected_files)
{
  FmbLauncher *launcher = FMB_LAUNCHER (component);
  GList          *np;
  GList          *op;

  /* compare the old and the new list of selected files */
  for (np = selected_files, op = launcher->selected_files; np != NULL && op != NULL; np = np->next, op = op->next)
    if (G_UNLIKELY (np->data != op->data))
      break;

  /* check if the list of selected files really changed */
  if (G_UNLIKELY (np != NULL || op != NULL))
    {
      /* disconnect from the previously selected files */
      fmb_g_file_list_free (launcher->selected_files);

      /* connect to the new selected files list */
      launcher->selected_files = fmb_g_file_list_copy (selected_files);

      /* update the launcher actions */
      fmb_launcher_update (launcher);

      /* notify listeners */
      g_object_notify_by_pspec (G_OBJECT (launcher), launcher_props[PROP_SELECTED_FILES]);
    }
}



static GtkUIManager*
fmb_launcher_get_ui_manager (FmbComponent *component)
{
  return FMB_LAUNCHER (component)->ui_manager;
}



static void
fmb_launcher_set_ui_manager (FmbComponent *component,
                                GtkUIManager    *ui_manager)
{
  FmbLauncher *launcher = FMB_LAUNCHER (component);
  GError         *error = NULL;

  /* disconnect from the previous UI manager */
  if (G_UNLIKELY (launcher->ui_manager != NULL))
    {
      /* drop our action group from the previous UI manager */
      gtk_ui_manager_remove_action_group (launcher->ui_manager, launcher->action_group);

      /* unmerge our addons ui controls from the previous UI manager */
      if (G_LIKELY (launcher->ui_addons_merge_id != 0))
        {
          gtk_ui_manager_remove_ui (launcher->ui_manager, launcher->ui_addons_merge_id);
          launcher->ui_addons_merge_id = 0;
        }

      /* unmerge our ui controls from the previous UI manager */
      gtk_ui_manager_remove_ui (launcher->ui_manager, launcher->ui_merge_id);

      /* drop the reference on the previous UI manager */
      g_object_unref (G_OBJECT (launcher->ui_manager));
    }

  /* activate the new UI manager */
  launcher->ui_manager = ui_manager;

  /* connect to the new UI manager */
  if (G_LIKELY (ui_manager != NULL))
    {
      /* we keep a reference on the new manager */
      g_object_ref (G_OBJECT (ui_manager));

      /* add our action group to the new manager */
      gtk_ui_manager_insert_action_group (ui_manager, launcher->action_group, -1);

      /* merge our UI control items with the new manager */
      launcher->ui_merge_id = gtk_ui_manager_add_ui_from_string (ui_manager, fmb_launcher_ui, fmb_launcher_ui_length, &error);
      if (G_UNLIKELY (launcher->ui_merge_id == 0))
        {
          g_error ("Failed to merge FmbLauncher menus: %s", error->message);
          g_error_free (error);
        }

      /* update the user interface */
      fmb_launcher_update (launcher);
    }

  /* notify listeners */
  g_object_notify_by_pspec (G_OBJECT (launcher), launcher_props[PROP_UI_MANAGER]);
}



static void
fmb_launcher_execute_files (FmbLauncher *launcher,
                               GList          *files)
{
  GError *error = NULL;
  GFile  *working_directory;
  GList  *lp;

  /* execute all selected files */
  for (lp = files; lp != NULL; lp = lp->next)
    {
      working_directory = fmb_file_get_file (launcher->current_directory);

      if (!fmb_file_execute (lp->data, working_directory, launcher->widget, NULL, NULL, &error))
        {
          /* display an error message to the user */
          fmb_dialogs_show_error (launcher->widget, error, _("Failed to execute file \"%s\""), fmb_file_get_display_name (lp->data));
          g_error_free (error);
          break;
        }
    }
}



static guint
fmb_launcher_g_app_info_hash (gconstpointer app_info)
{
  return 0;
}



static void
fmb_launcher_open_files (FmbLauncher *launcher,
                            GList          *files)
{
  GHashTable *applications;
  GAppInfo   *app_info;
  GList      *file_list;
  GList      *lp;

  /* allocate a hash table to associate applications to URIs. since GIO allocates
   * new GAppInfo objects every time, g_direct_hash does not work. we therefor use
   * a fake hash function to always hit the collision list of the hash table and
   * avoid storing multiple equal GAppInfos by means of g_app_info_equal(). */
  applications = g_hash_table_new_full (fmb_launcher_g_app_info_hash,
                                        (GEqualFunc) g_app_info_equal,
                                        (GDestroyNotify) g_object_unref,
                                        (GDestroyNotify) fmb_g_file_list_free);

  for (lp = files; lp != NULL; lp = lp->next)
    {
      /* determine the default application for the MIME type */
      app_info = fmb_file_get_default_handler (lp->data);

      /* check if we have an application here */
      if (G_LIKELY (app_info != NULL))
        {
          /* check if we have that application already */
          file_list = g_hash_table_lookup (applications, app_info);
          if (G_LIKELY (file_list != NULL))
            {
              /* take a copy of the list as the old one will be dropped by the insert */
              file_list = fmb_g_file_list_copy (file_list);
            }

          /* append our new URI to the list */
          file_list = fmb_g_file_list_append (file_list, fmb_file_get_file (lp->data));

          /* (re)insert the URI list for the application */
          g_hash_table_insert (applications, app_info, file_list);
        }
      else
        {
          /* display a chooser dialog for the file and stop */
          fmb_show_chooser_dialog (launcher->widget, lp->data, TRUE);
          break;
        }
    }

  /* run all collected applications */
  g_hash_table_foreach (applications, (GHFunc) fmb_launcher_open_paths, launcher);

  /* drop the applications hash table */
  g_hash_table_destroy (applications);
}



static void
fmb_launcher_open_paths (GAppInfo       *app_info,
                            GList          *path_list,
                            FmbLauncher *launcher)
{
  GdkAppLaunchContext *context;
  GdkScreen           *screen;
  GError              *error = NULL;
  GFile               *working_directory = NULL;
  gchar               *message;
  gchar               *name;
  guint                n;

  /* determine the screen on which to launch the application */
  screen = (launcher->widget != NULL) ? gtk_widget_get_screen (launcher->widget) : NULL;

  /* create launch context */
  context = gdk_app_launch_context_new ();
  gdk_app_launch_context_set_screen (context, screen);
  gdk_app_launch_context_set_timestamp (context, gtk_get_current_event_time ());
  gdk_app_launch_context_set_icon (context, g_app_info_get_icon (app_info));

  /* determine the working directory */
  if (launcher->current_directory != NULL)
    working_directory = fmb_file_get_file (launcher->current_directory);

  /* try to execute the application with the given URIs */
  if (!fmb_g_app_info_launch (app_info, working_directory, path_list, G_APP_LAUNCH_CONTEXT (context), &error))
    {
      /* figure out the appropriate error message */
      n = g_list_length (path_list);
      if (G_LIKELY (n == 1))
        {
          /* we can give a precise error message here */
          name = g_filename_display_name (g_file_get_basename (path_list->data));
          message = g_strdup_printf (_("Failed to open file \"%s\""), name);
          g_free (name);
        }
      else
        {
          /* we can just tell that n files failed to open */
          message = g_strdup_printf (ngettext ("Failed to open %d file", "Failed to open %d files", n), n);
        }

      /* display an error dialog to the user */
      fmb_dialogs_show_error (launcher->widget, error, "%s", message);
      g_error_free (error);
      g_free (message);
    }

  /* destroy the launch context */
  g_object_unref (context);
}



static void
fmb_launcher_open_windows (FmbLauncher *launcher,
                              GList          *directories)
{
  FmbApplication *application;
  GtkWidget         *dialog;
  GtkWidget         *window;
  GdkScreen         *screen;
  gchar             *label;
  GList             *lp;
  gint               response = GTK_RESPONSE_YES;
  gint               n;

  /* ask the user if we would open more than one new window */
  n = g_list_length (directories);
  if (G_UNLIKELY (n > 1))
    {
      /* open a message dialog */
      window = (launcher->widget != NULL) ? gtk_widget_get_toplevel (launcher->widget) : NULL;
      dialog = gtk_message_dialog_new ((GtkWindow *) window,
                                       GTK_DIALOG_DESTROY_WITH_PARENT
                                       | GTK_DIALOG_MODAL,
                                       GTK_MESSAGE_QUESTION,
                                       GTK_BUTTONS_NONE,
                                       _("Are you sure you want to open all folders?"));
      gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog),
                                                ngettext ("This will open %d separate file manager window.",
                                                          "This will open %d separate file manager windows.",
                                                          n),
                                                n);
      label = g_strdup_printf (ngettext ("Open %d New Window", "Open %d New Windows", n), n);
      gtk_dialog_add_button (GTK_DIALOG (dialog), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);
      gtk_dialog_add_button (GTK_DIALOG (dialog), label, GTK_RESPONSE_YES);
      gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_YES);
      response = gtk_dialog_run (GTK_DIALOG (dialog));
      gtk_widget_destroy (dialog);
      g_free (label);
    }

  /* open n new windows if the user approved it */
  if (G_LIKELY (response == GTK_RESPONSE_YES))
    {
      /* query the application object */
      application = fmb_application_get ();

      /* determine the screen on which to open the new windows */
      screen = (launcher->widget != NULL) ? gtk_widget_get_screen (launcher->widget) : NULL;

      /* open all requested windows */
      for (lp = directories; lp != NULL; lp = lp->next)
        fmb_application_open_window (application, lp->data, screen, NULL);

      /* release the application object */
      g_object_unref (G_OBJECT (application));
    }
}



static gboolean
fmb_launcher_update_idle (gpointer data)
{
  FmbLauncher *launcher = FMB_LAUNCHER (data);
  const gchar    *context_menu_path;
  const gchar    *file_menu_path;
  GtkAction      *action;
  gboolean        default_is_open_with_other = FALSE;
  GList          *applications;
  GList          *actions;
  GList          *lp;
  gchar          *tooltip;
  gchar          *label;
  gchar          *name;
  gint            n_directories = 0;
  gint            n_executables = 0;
  gint            n_regulars = 0;
  gint            n_selected_files = 0;
  gint            n;

  /* verify that we're connected to an UI manager */
  if (G_UNLIKELY (launcher->ui_manager == NULL))
    return FALSE;

  GDK_THREADS_ENTER ();

  /* drop the previous addons ui controls from the UI manager */
  if (G_LIKELY (launcher->ui_addons_merge_id != 0))
    {
      gtk_ui_manager_remove_ui (launcher->ui_manager, launcher->ui_addons_merge_id);
      gtk_ui_manager_ensure_update (launcher->ui_manager);
      launcher->ui_addons_merge_id = 0;
    }

  /* reset the application set for the "Open" action */
  g_object_set_qdata (G_OBJECT (launcher->action_open), fmb_launcher_handler_quark, NULL);

  /* determine the number of files/directories/executables */
  for (lp = launcher->selected_files; lp != NULL; lp = lp->next, ++n_selected_files)
    {
      if (fmb_file_is_directory (lp->data)
          || fmb_file_is_shortcut (lp->data)
          || fmb_file_is_mountable (lp->data))
        {
          ++n_directories;
        }
      else
        {
          if (fmb_file_is_executable (lp->data))
            ++n_executables;
          ++n_regulars;
        }
    }

  /* update the user interface depending on the current selection */
  if (G_LIKELY (n_selected_files == 0 || n_directories > 0))
    {
      /** CASE 1: nothing selected or atleast one directory in the selection
       **
       ** - "Open", "Open in n New Windows" and "Open in n New Tabs" actions
       **/

      /* Prepare "Open" label and icon */
      gtk_action_set_label (launcher->action_open, _("_Open"));
      gtk_action_set_stock_id (launcher->action_open, GTK_STOCK_OPEN);

      if (n_selected_files == n_directories && n_directories >= 1)
        {
          if (n_directories > 1)
            {
              /* turn "Open New Window" into "Open in n New Windows" */
              label = g_strdup_printf (ngettext ("Open in %d New _Window", "Open in %d New _Windows", n_directories), n_directories);
              tooltip = g_strdup_printf (ngettext ("Open the selected directory in %d new window",
                                                   "Open the selected directories in %d new windows",
                                                   n_directories), n_directories);
              g_object_set (G_OBJECT (launcher->action_open_in_new_window),
                            "label", label,
                            "tooltip", tooltip,
                            NULL);
              g_free (tooltip);
              g_free (label);

              /* turn "Open in New Tab" into "Open in x New Tabs" */
              label = g_strdup_printf (ngettext ("Open in %d New _Tab", "Open in %d New _Tabs", n_directories), n_directories);
              tooltip = g_strdup_printf (ngettext ("Open the selected directory in %d new tab",
                                                   "Open the selected directories in %d new tabs",
                                                   n_directories), n_directories);
              g_object_set (G_OBJECT (launcher->action_open_in_new_tab),
                            "label", label,
                            "tooltip", tooltip,
                            NULL);
              g_free (tooltip);
              g_free (label);
            }
          else if (n_directories == 1)
            {
              /* prepare "Open in New Window" */
              g_object_set (G_OBJECT (launcher->action_open_in_new_window),
                            "label", _("Open in New _Window"),
                            "tooltip", _("Open the selected directory in a new window"),
                            NULL);

              /* prepare "Open in New Tab" */
              g_object_set (G_OBJECT (launcher->action_open_in_new_tab),
                            "label", _("Open in New _Tab"),
                            "tooltip", _("Open the selected directory in a new tab"),
                            NULL);

              /* set tooltip that makes sence */
              gtk_action_set_tooltip (launcher->action_open, _("Open the selected directory"));
            }

          /* Show Window/Tab action if there are only directories selected */
          gtk_action_set_visible (launcher->action_open_in_new_window, n_directories > 0);
          gtk_action_set_visible (launcher->action_open_in_new_tab, n_directories > 0);

          /* Show open if there is exactly 1 directory selected */
          gtk_action_set_visible (launcher->action_open, n_directories == 1);
          gtk_action_set_sensitive (launcher->action_open, TRUE);
        }
      else
        {
          /* Hide New Window and Tab action */
          gtk_action_set_visible (launcher->action_open_in_new_window, FALSE);
          gtk_action_set_visible (launcher->action_open_in_new_tab, FALSE);

          /* Normal open action, because there are also directories included */
          gtk_action_set_visible (launcher->action_open, TRUE);
          gtk_action_set_sensitive (launcher->action_open, n_selected_files > 0);
          gtk_action_set_tooltip (launcher->action_open,
                                  ngettext ("Open the selected file",
                                            "Open the selected files",
                                            n_selected_files));
        }

      /* hide the "Open With Other Application" actions */
      gtk_action_set_visible (launcher->action_open_with_other, FALSE);
      gtk_action_set_visible (launcher->action_open_with_other_in_menu, FALSE);
    }
  else
    {
      /** CASE 2: one or more file in the selection
       **
       ** - "Execute" action if all selected files are executable
       ** - No "Open in n New Windows" action
       **/

      /* drop all previous addon actions from the action group */
      actions = gtk_action_group_list_actions (launcher->action_group);
      for (lp = actions; lp != NULL; lp = lp->next)
        if (strncmp (gtk_action_get_name (lp->data), "fmb-launcher-addon-", 22) == 0)
          gtk_action_group_remove_action (launcher->action_group, lp->data);
      g_list_free (actions);

      /* allocate a new merge id from the UI manager */
      launcher->ui_addons_merge_id = gtk_ui_manager_new_merge_id (launcher->ui_manager);

      /* make the "Open" action sensitive */
      gtk_action_set_sensitive (launcher->action_open, TRUE);

      /* hide the "Open in n New Windows/Tabs" action */
      gtk_action_set_visible (launcher->action_open_in_new_window, FALSE);
      gtk_action_set_visible (launcher->action_open_in_new_tab, FALSE);

      /* determine the set of applications that work for all selected files */
      applications = fmb_file_list_get_applications (launcher->selected_files);

      /* reset the desktop actions list */
      actions = NULL;

      /* check if we have only executable files in the selection */
      if (G_UNLIKELY (n_executables == n_selected_files))
        {
          /* turn the "Open" action into "Execute" */
          g_object_set (G_OBJECT (launcher->action_open),
                        "label", _("_Execute"),
                        "stock-id", GTK_STOCK_EXECUTE,
                        "tooltip", ngettext ("Execute the selected file", "Execute the selected files", n_selected_files),
                        NULL);
        }
      else if (G_LIKELY (applications != NULL))
        {
          /* turn the "Open" action into "Open With DEFAULT" */
          label = g_strdup_printf (_("_Open With \"%s\""), g_app_info_get_name (applications->data));
          tooltip = g_strdup_printf (ngettext ("Use \"%s\" to open the selected file",
                                               "Use \"%s\" to open the selected files",
                                               n_selected_files), g_app_info_get_name (applications->data));
          g_object_set (G_OBJECT (launcher->action_open),
                        "label", label,
                        "tooltip", tooltip,
                        NULL);
          g_free (tooltip);
          g_free (label);

          /* load default application icon */
          gtk_action_set_stock_id (launcher->action_open, NULL);
          gtk_action_set_gicon (launcher->action_open, g_app_info_get_icon (applications->data));

          /* remember the default application for the "Open" action */
          g_object_set_qdata_full (G_OBJECT (launcher->action_open), fmb_launcher_handler_quark, applications->data, g_object_unref);

          /* FIXME Add the desktop actions for this application.
           * Unfortunately this is not supported by GIO directly */

          /* drop the default application from the list */
          applications = g_list_delete_link (applications, applications);
        }
      else if (G_UNLIKELY (n_selected_files == 1))
        {
          /* turn the "Open" action into "Open With Other Application" */
          g_object_set (G_OBJECT (launcher->action_open),
                        "label", _("_Open With Other Application..."),
                        "tooltip", _("Choose another application with which to open the selected file"),
                        NULL);
          default_is_open_with_other = TRUE;
        }
      else
        {
          /* we can only show a generic "Open" action */
          g_object_set (G_OBJECT (launcher->action_open),
                        "label", _("_Open With Default Applications"),
                        "tooltip", ngettext ("Open the selected file with the default application",
                                             "Open the selected files with the default applications", n_selected_files),
                        NULL);
        }

      /* place the other applications in the "Open With" submenu if we have more than 2 other applications, or the
       * default action for the file is "Execute", in which case the "Open With" actions aren't that relevant either
       */
      if (G_UNLIKELY (g_list_length (applications) > 2 || n_executables == n_selected_files))
        {
          /* determine the base paths for the actions */
          file_menu_path = "/main-menu/file-menu/placeholder-launcher/open-with-menu/placeholder-applications";
          context_menu_path = "/file-context-menu/placeholder-launcher/open-with-menu/placeholder-applications";

          /* show the "Open With Other Application" in the submenu and hide the toplevel one */
          gtk_action_set_visible (launcher->action_open_with_other, FALSE);
          gtk_action_set_visible (launcher->action_open_with_other_in_menu, (n_selected_files == 1));
        }
      else
        {
          /* determine the base paths for the actions */
          file_menu_path = "/main-menu/file-menu/placeholder-launcher/placeholder-applications";
          context_menu_path = "/file-context-menu/placeholder-launcher/placeholder-applications";

          /* add a separator if we have more than one additional application */
          if (G_LIKELY (applications != NULL))
            {
              /* add separator after the DEFAULT/execute action */
              gtk_ui_manager_add_ui (launcher->ui_manager, launcher->ui_addons_merge_id,
                                     file_menu_path, "separator", NULL,
                                     GTK_UI_MANAGER_SEPARATOR, FALSE);
              gtk_ui_manager_add_ui (launcher->ui_manager, launcher->ui_addons_merge_id,
                                     context_menu_path, "separator", NULL,
                                     GTK_UI_MANAGER_SEPARATOR, FALSE);
            }

          /* show the toplevel "Open With Other Application" (if not already done by the "Open" action) */
          gtk_action_set_visible (launcher->action_open_with_other, !default_is_open_with_other && (n_selected_files == 1));
          gtk_action_set_visible (launcher->action_open_with_other_in_menu, FALSE);
        }

      /* add actions for all remaining applications */
      if (G_LIKELY (applications != NULL))
        {
          /* process all applications and determine the desktop actions */
          for (lp = applications, n = 0; lp != NULL; lp = lp->next, ++n)
            {
              /* FIXME Determine the desktop actions for this application.
               * Unfortunately this is not supported by GIO directly. */

              /* generate a unique label, unique id and tooltip for the application's action */
              name = g_strdup_printf ("fmb-launcher-addon-application%d-%p", n, launcher);
              label = g_strdup_printf (_("Open With \"%s\""), g_app_info_get_name (lp->data));
              tooltip = g_strdup_printf (ngettext ("Use \"%s\" to open the selected file",
                                                   "Use \"%s\" to open the selected files",
                                                   n_selected_files), g_app_info_get_name (lp->data));

              /* allocate a new action for the application */
              action = gtk_action_new (name, label, tooltip, NULL);
              gtk_action_set_gicon (action, g_app_info_get_icon (lp->data));
              gtk_action_group_add_action (launcher->action_group, action);
              g_object_set_qdata_full (G_OBJECT (action), fmb_launcher_handler_quark, lp->data, g_object_unref);
              g_signal_connect (G_OBJECT (action), "activate", G_CALLBACK (fmb_launcher_action_open), launcher);
              gtk_ui_manager_add_ui (launcher->ui_manager, launcher->ui_addons_merge_id,
                                     file_menu_path, name, name,
                                     GTK_UI_MANAGER_MENUITEM, FALSE);
              gtk_ui_manager_add_ui (launcher->ui_manager, launcher->ui_addons_merge_id,
                                     context_menu_path, name, name,
                                     GTK_UI_MANAGER_MENUITEM, FALSE);
              g_object_unref (G_OBJECT (action));

              /* cleanup */
              g_free (tooltip);
              g_free (label);
              g_free (name);
            }

          /* cleanup */
          g_list_free (applications);
        }

      /* FIXME Add desktop actions here. Unfortunately they are not supported by
       * GIO, so we'll have to roll our own thing here */
    }

  /* schedule an update of the "Send To" menu */
  if (G_LIKELY (launcher->sendto_idle_id == 0))
    {
      launcher->sendto_idle_id = g_idle_add_full (G_PRIORITY_LOW, fmb_launcher_sendto_idle,
                                                  launcher, fmb_launcher_sendto_idle_destroy);
    }

  GDK_THREADS_LEAVE ();

  return FALSE;
}



static void
fmb_launcher_update_idle_destroy (gpointer data)
{
  FMB_LAUNCHER (data)->launcher_idle_id = 0;
}



static void
fmb_launcher_update_check (FmbLauncher *launcher,
                              GtkWidget      *menu)
{
  _fmb_return_if_fail (FMB_IS_LAUNCHER (launcher));
  _fmb_return_if_fail (menu == NULL || GTK_IS_MENU (menu));

  /* check if the menu is in a dirty state */
  if (launcher->launcher_idle_id != 0)
    {
      /* stop the timeout */
      g_source_remove (launcher->launcher_idle_id);

      /* force an update */
      fmb_launcher_update_idle (launcher);

      /* ui update */
      gtk_ui_manager_ensure_update (launcher->ui_manager);

      /* make sure the menu is positioned correctly after the
       * interface update */
      if (menu != NULL)
        gtk_menu_reposition (GTK_MENU (menu));
    }
}



static void
fmb_launcher_update (FmbLauncher *launcher)
{
  GSList    *proxies, *lp;
  GtkWidget *menu;
  gboolean   instant_update;

  _fmb_return_if_fail (FMB_IS_LAUNCHER (launcher));

  proxies = gtk_action_get_proxies (launcher->action_open);
  instant_update = (proxies == NULL);
  for (lp = proxies; lp != NULL; lp = lp->next)
    {
      menu = gtk_widget_get_ancestor (lp->data, GTK_TYPE_MENU);
      if (G_LIKELY (menu != NULL))
        {
          /* instant update if a menu is visible */
          if (gtk_widget_get_visible (menu))
            instant_update = TRUE;

          /* watch menu changes */
          g_signal_handlers_disconnect_by_func (G_OBJECT (menu), G_CALLBACK (fmb_launcher_update_check), launcher);
          g_signal_connect_swapped (G_OBJECT (menu), "show", G_CALLBACK (fmb_launcher_update_check), launcher);
        }
    }

  /* stop pending timeouts */
  if (launcher->launcher_idle_id != 0)
    g_source_remove (launcher->launcher_idle_id);

  if (instant_update)
    {
      /* directly update without interruption */
      fmb_launcher_update_idle (launcher);
    }
  else
    {
      /* assume all actions are working */
      gtk_action_set_sensitive (launcher->action_open, TRUE);
      gtk_action_set_visible (launcher->action_open_with_other, TRUE);
      gtk_action_set_visible (launcher->action_open_in_new_window, TRUE);
      gtk_action_set_visible (launcher->action_open_in_new_tab, TRUE);
      gtk_action_set_visible (launcher->action_open_with_other_in_menu, TRUE);

      /* delayed update */
      launcher->launcher_idle_id = g_timeout_add_seconds_full (G_PRIORITY_LOW, 5, fmb_launcher_update_idle,
                                                               launcher, fmb_launcher_update_idle_destroy);
    }
}



static void
fmb_launcher_open_file (FmbLauncher *launcher,
                           FmbFile     *file)
{
  GList files;

  _fmb_return_if_fail (FMB_IS_LAUNCHER (launcher));
  _fmb_return_if_fail (FMB_IS_FILE (file));

  files.data = file;
  files.next = NULL;
  files.prev = NULL;

  if (fmb_file_is_directory (file))
    {
      /* check if we're in a regular view (i.e. current_directory is set) */
      if (G_LIKELY (launcher->current_directory != NULL))
        {
          /* we want to open one directory, so just emit "change-directory" here */
          fmb_navigator_change_directory (FMB_NAVIGATOR (launcher), file);
        }
      else
        {
          /* open the selected directory in a new window */
          fmb_launcher_open_windows (launcher, &files);
        }
    }
  else
    {
      if (fmb_file_is_executable (file))
        {
          /* try to execute the file */
          fmb_launcher_execute_files (launcher, &files);
        }
      else
        {
          /* try to open the file using its default application */
          fmb_launcher_open_files (launcher, &files);
        }
    }
}



static void
fmb_launcher_poke_file_finish (FmbBrowser *browser,
                                  FmbFile    *file,
                                  FmbFile    *target_file,
                                  GError        *error,
                                  gpointer       ignored)
{
  if (error == NULL)
    {
      fmb_launcher_open_file (FMB_LAUNCHER (browser), target_file);
    }
  else
    {
      fmb_dialogs_show_error (FMB_LAUNCHER (browser)->widget, error,
                                 _("Failed to open \"%s\""),
                                 fmb_file_get_display_name (file));
    }
}



static void
fmb_launcher_poke_files (FmbLauncher         *launcher,
                            FmbLauncherPokeData *poke_data)
{
  _fmb_return_if_fail (FMB_IS_LAUNCHER (launcher));
  _fmb_return_if_fail (poke_data != NULL);
  _fmb_return_if_fail (poke_data->files != NULL);

  fmb_browser_poke_file (FMB_BROWSER (launcher), poke_data->files->data,
                            launcher->widget, fmb_launcher_poke_files_finish,
                            poke_data);
}



static void
fmb_launcher_poke_files_finish (FmbBrowser *browser,
                                   FmbFile    *file,
                                   FmbFile    *target_file,
                                   GError        *error,
                                   gpointer       user_data)
{
  FmbLauncherPokeData *poke_data = user_data;
  gboolean                executable = TRUE;
  GList                  *directories = NULL;
  GList                  *files = NULL;
  GList                  *lp;

  _fmb_return_if_fail (FMB_IS_BROWSER (browser));
  _fmb_return_if_fail (FMB_IS_FILE (file));
  _fmb_return_if_fail (poke_data != NULL);
  _fmb_return_if_fail (poke_data->files != NULL);

  /* check if poking succeeded */
  if (error == NULL)
    {
      /* add the resolved file to the list of file to be opened/executed later */
      poke_data->resolved_files = g_list_prepend (poke_data->resolved_files,
                                                  g_object_ref (target_file));
    }

  /* release and remove the just poked file from the list */
  g_object_unref (poke_data->files->data);
  poke_data->files = g_list_delete_link (poke_data->files, poke_data->files);

  if (poke_data->files == NULL)
    {
      /* separate files and directories in the selected files list */
      for (lp = poke_data->resolved_files; lp != NULL; lp = lp->next)
        {
          if (fmb_file_is_directory (lp->data))
            {
              /* add to our directory list */
              directories = g_list_prepend (directories, lp->data);
            }
          else
            {
              /* add to our file list */
              files = g_list_prepend (files, lp->data);

              /* check if the file is executable */
              executable = (executable && fmb_file_is_executable (lp->data));
            }
        }

      /* check if we have any directories to process */
      if (G_LIKELY (directories != NULL))
        {
          if (poke_data->directories_in_tabs)
            {
              /* open new tabs */
              for (lp = directories; lp != NULL; lp = lp->next)
                fmb_navigator_open_new_tab (FMB_NAVIGATOR (browser), lp->data);
            }
          else
            {
              /* open new windows for all directories */
              fmb_launcher_open_windows (FMB_LAUNCHER (browser), directories);
            }
          g_list_free (directories);
        }

      /* check if we have any files to process */
      if (G_LIKELY (files != NULL))
        {
          /* if all files are executable, we just run them here */
          if (G_UNLIKELY (executable))
            {
              /* try to execute all given files */
              fmb_launcher_execute_files (FMB_LAUNCHER (browser), files);
            }
          else
            {
              /* try to open all files using their default applications */
              fmb_launcher_open_files (FMB_LAUNCHER (browser), files);
            }

          /* cleanup */
          g_list_free (files);
        }

      /* free all files allocated for the poke data */
      fmb_launcher_poke_data_free (poke_data);
    }
  else
    {
      /* we need to continue this until all files have been resolved */
      fmb_launcher_poke_files (FMB_LAUNCHER (browser), poke_data);
    }
}



static void
fmb_launcher_action_open (GtkAction      *action,
                             FmbLauncher *launcher)
{
  FmbLauncherPokeData *poke_data;
  GAppInfo               *app_info;
  GList                  *selected_paths;

  _fmb_return_if_fail (GTK_IS_ACTION (action));
  _fmb_return_if_fail (FMB_IS_LAUNCHER (launcher));

  /* force update if still dirty */
  fmb_launcher_update_check (launcher, NULL);
  if (!gtk_action_get_sensitive (action))
    return;

  /* check if we have a mime handler associated with the action */
  app_info = g_object_get_qdata (G_OBJECT (action), fmb_launcher_handler_quark);
  if (G_LIKELY (app_info != NULL))
    {
      /* try to open the selected files using the given application */
      selected_paths = fmb_file_list_to_fmb_g_file_list (launcher->selected_files);
      fmb_launcher_open_paths (app_info, selected_paths, launcher);
      fmb_g_file_list_free (selected_paths);
    }
  else if (launcher->selected_files != NULL)
    {
      if (launcher->selected_files->next == NULL)
        {
          fmb_browser_poke_file (FMB_BROWSER (launcher),
                                    launcher->selected_files->data, launcher->widget,
                                    fmb_launcher_poke_file_finish, NULL);
        }
      else
        {
          /* resolve files one after another until none is left. Open/execute
           * the resolved files/directories when all this is done at a later
           * stage */
           poke_data = fmb_launcher_poke_data_new (launcher->selected_files);
           fmb_launcher_poke_files (launcher, poke_data);
        }
    }
}



static void
fmb_launcher_action_open_with_other (GtkAction      *action,
                                        FmbLauncher *launcher)
{
  _fmb_return_if_fail (GTK_IS_ACTION (action));
  _fmb_return_if_fail (FMB_IS_LAUNCHER (launcher));

  /* force update if still dirty */
  fmb_launcher_update_check (launcher, NULL);
  if (!gtk_action_get_visible (action))
    return;

  /* verify that we have atleast one selected file */
  if (G_LIKELY (launcher->selected_files != NULL))
    {
      /* popup the chooser dialog for the first selected file */
      fmb_show_chooser_dialog (launcher->widget, launcher->selected_files->data, TRUE);
    }
}



static void
fmb_launcher_action_open_in_new_window (GtkAction      *action,
                                           FmbLauncher *launcher)
{
  FmbLauncherPokeData *poke_data;

  _fmb_return_if_fail (GTK_IS_ACTION (action));
  _fmb_return_if_fail (FMB_IS_LAUNCHER (launcher));

  /* force update if still dirty */
  fmb_launcher_update_check (launcher, NULL);
  if (!gtk_action_get_visible (action))
    return;

  /* open the selected directories in new windows */
  poke_data = fmb_launcher_poke_data_new (launcher->selected_files);
  fmb_launcher_poke_files (launcher, poke_data);
}



static void
fmb_launcher_action_open_in_new_tab (GtkAction      *action,
                                        FmbLauncher *launcher)
{
  FmbLauncherPokeData *poke_data;

  _fmb_return_if_fail (GTK_IS_ACTION (action));
  _fmb_return_if_fail (FMB_IS_LAUNCHER (launcher));

  /* force update if still dirty */
  fmb_launcher_update_check (launcher, NULL);
  if (!gtk_action_get_visible (action))
    return;

  /* open all selected directories in a new tab */
  poke_data = fmb_launcher_poke_data_new (launcher->selected_files);
  poke_data->directories_in_tabs = TRUE;
  fmb_launcher_poke_files (launcher, poke_data);
}



static void
fmb_launcher_action_sendto_desktop (GtkAction      *action,
                                       FmbLauncher *launcher)
{
  FmbApplication *application;
  GFile             *desktop_file;
  GList             *files;

  _fmb_return_if_fail (GTK_IS_ACTION (action));
  _fmb_return_if_fail (FMB_IS_LAUNCHER (launcher));

  /* determine the source files */
  files = fmb_file_list_to_fmb_g_file_list (launcher->selected_files);
  if (G_UNLIKELY (files == NULL))
    return;

  /* determine the file to the ~/Desktop folder */
  desktop_file = fmb_g_file_new_for_desktop ();

  /* launch the link job */
  application = fmb_application_get ();
  fmb_application_link_into (application, launcher->widget, files, desktop_file, NULL);
  g_object_unref (G_OBJECT (application));

  /* cleanup */
  g_object_unref (desktop_file);
  fmb_g_file_list_free (files);
}



static FmbLauncherMountData *
fmb_launcher_mount_data_new (FmbLauncher *launcher,
                                GList          *files)
{
  FmbLauncherMountData *data;

  _fmb_return_val_if_fail (FMB_IS_LAUNCHER (launcher), NULL);

  data = g_slice_new0 (FmbLauncherMountData);
  data->launcher = g_object_ref (launcher);
  data->files = fmb_g_file_list_copy (files);

  return data;
}



static void
fmb_launcher_mount_data_free (FmbLauncherMountData *data)
{
  _fmb_return_if_fail (data != NULL);
  _fmb_return_if_fail (FMB_IS_LAUNCHER (data->launcher));

  g_object_unref (data->launcher);
  fmb_g_file_list_free (data->files);
  g_slice_free (FmbLauncherMountData, data);
}



static FmbLauncherPokeData *
fmb_launcher_poke_data_new (GList *files)
{
  FmbLauncherPokeData *data;

  data = g_slice_new0 (FmbLauncherPokeData);
  data->files = fmb_g_file_list_copy (files);
  data->resolved_files = NULL;
  data->directories_in_tabs = FALSE;

  return data;
}



static void
fmb_launcher_poke_data_free (FmbLauncherPokeData *data)
{
  _fmb_return_if_fail (data != NULL);

  fmb_g_file_list_free (data->files);
  fmb_g_file_list_free (data->resolved_files);
  g_slice_free (FmbLauncherPokeData, data);
}



static void
fmb_launcher_sendto_device (FmbLauncher *launcher,
                               FmbDevice   *device,
                               GList          *files)
{
  FmbApplication *application;
  GFile             *mount_point;

  _fmb_return_if_fail (FMB_IS_LAUNCHER (launcher));
  _fmb_return_if_fail (FMB_IS_DEVICE (device));

  if (!fmb_device_is_mounted (device))
    return;

  mount_point = fmb_device_get_root (device);
  if (mount_point != NULL)
    {
      /* copy the files onto the specified device */
      application = fmb_application_get ();
      fmb_application_copy_into (application, launcher->widget, files, mount_point, NULL);
      g_object_unref (application);

      g_object_unref (mount_point);
    }
}



static void
fmb_launcher_sendto_mount_finish (FmbDevice *device,
                                     const GError *error,
                                     gpointer      user_data)
{
  FmbLauncherMountData *data = user_data;
  gchar                   *device_name;

  _fmb_return_if_fail (FMB_IS_DEVICE (device));
  _fmb_return_if_fail (user_data != NULL);
  _fmb_return_if_fail (FMB_IS_LAUNCHER (data->launcher));

  if (error != NULL)
    {
      /* tell the user that we were unable to mount the device, which is
       * required to send files to it */
      device_name = fmb_device_get_name (device);
      fmb_dialogs_show_error (data->launcher->widget, error, _("Failed to mount \"%s\""), device_name);
      g_free (device_name);
    }
  else
    {
      fmb_launcher_sendto_device (data->launcher, device, data->files);
    }

  fmb_launcher_mount_data_free (data);
}



static void
fmb_launcher_action_sendto_device (GtkAction      *action,
                                      FmbLauncher *launcher)
{
  FmbLauncherMountData *data;
  GMountOperation         *mount_operation;
  FmbDevice            *device;
  GList                   *files;

  _fmb_return_if_fail (GTK_IS_ACTION (action));
  _fmb_return_if_fail (FMB_IS_LAUNCHER (launcher));

  /* determine the source paths */
  files = fmb_file_list_to_fmb_g_file_list (launcher->selected_files);
  if (G_UNLIKELY (files == NULL))
    return;

  /* determine the device to which to send */
  device = g_object_get_qdata (G_OBJECT (action), fmb_launcher_handler_quark);
  if (G_UNLIKELY (device == NULL))
    return;

  /* make sure to mount the device first, if it's not already mounted */
  if (!fmb_device_is_mounted (device))
    {
      /* allocate mount data */
      data = fmb_launcher_mount_data_new (launcher, files);

      /* allocate a GTK+ mount operation */
      mount_operation = fmb_gtk_mount_operation_new (launcher->widget);

      /* try to mount the device and later start sending the files */
      fmb_device_mount (device,
                           mount_operation,
                           NULL,
                           fmb_launcher_sendto_mount_finish,
                           data);

      g_object_unref (mount_operation);
    }
  else
    {
      fmb_launcher_sendto_device (launcher, device, files);
    }

  /* cleanup */
  fmb_g_file_list_free (files);
}



static void
fmb_launcher_widget_destroyed (FmbLauncher *launcher,
                                  GtkWidget      *widget)
{
  _fmb_return_if_fail (FMB_IS_LAUNCHER (launcher));
  _fmb_return_if_fail (launcher->widget == widget);
  _fmb_return_if_fail (GTK_IS_WIDGET (widget));

  /* just reset the widget property for the launcher */
  fmb_launcher_set_widget (launcher, NULL);
}



static gboolean
fmb_launcher_sendto_idle (gpointer user_data)
{
  FmbLauncher *launcher = FMB_LAUNCHER (user_data);
  const gchar    *label;
  GtkAction      *action;
  gboolean        linkable = TRUE;
  GIcon          *icon;
  GList          *handlers;
  GList          *devices;
  GList          *lp;
  gchar          *name;
  gchar          *tooltip;
  gchar          *device_name;
  gint            n_selected_files;
  gint            n = 0;
  gboolean        got_devices = FALSE;
  const gchar    *file_menu_path;
  const gchar    *context_menu_path;

  /* verify that we have an UI manager */
  if (launcher->ui_manager == NULL)
    return FALSE;

  GDK_THREADS_ENTER ();

  /* determine the number of selected files and check whether atleast one of these
   * files is located in the trash (to en-/disable the "sendto-desktop" action).
   */
  for (lp = launcher->selected_files, n_selected_files = 0; lp != NULL; lp = lp->next, ++n_selected_files)
    {
      /* check if this file is in trash */
      if (G_UNLIKELY (linkable))
        linkable = !fmb_file_is_trashed (lp->data);
    }

  /* update the "Desktop (Create Link)" sendto action */
  action = gtk_action_group_get_action (launcher->action_group, "sendto-desktop");
  g_object_set (G_OBJECT (action),
                "label", ngettext ("Desktop (Create Link)", "Desktop (Create Links)", n_selected_files),
                "tooltip", ngettext ("Create a link to the selected file on the desktop",
                                     "Create links to the selected files on the desktop",
                                     n_selected_files),
                "visible", (linkable && n_selected_files > 0),
                NULL);

  /* re-add the content to "Send To" if we have any files */
  if (G_LIKELY (n_selected_files > 0))
    {
      /* drop all previous sendto actions from the action group */
      handlers = gtk_action_group_list_actions (launcher->action_group);
      for (lp = handlers; lp != NULL; lp = lp->next)
        if (strncmp (gtk_action_get_name (lp->data), "fmb-launcher-sendto", 22) == 0)
          gtk_action_group_remove_action (launcher->action_group, lp->data);
      g_list_free (handlers);

      /* allocate a new merge id from the UI manager (if not already done) */
      if (G_UNLIKELY (launcher->ui_addons_merge_id == 0))
        launcher->ui_addons_merge_id = gtk_ui_manager_new_merge_id (launcher->ui_manager);

      /* determine the currently active devices */
      devices = fmb_device_monitor_get_devices (launcher->device_monitor);
      got_devices = (devices != NULL);

      /* paths in ui */
      file_menu_path = "/main-menu/file-menu/sendto-menu/placeholder-sendto-actions";
      context_menu_path = "/file-context-menu/sendto-menu/placeholder-sendto-actions";

      /* add removable (and writable) drives and media */
      for (lp = devices; lp != NULL; lp = lp->next, ++n)
        {
          /* generate a unique name and tooltip for the device */
          device_name = fmb_device_get_name (lp->data);
          name = g_strdup_printf ("fmb-launcher-sendto%d-%p", n, launcher);
          tooltip = g_strdup_printf (ngettext ("Send the selected file to \"%s\"",
                                               "Send the selected files to \"%s\"",
                                               n_selected_files), device_name);

          /* allocate a new action for the device */
          action = gtk_action_new (name, device_name, tooltip, NULL);
          g_object_set_qdata_full (G_OBJECT (action), fmb_launcher_handler_quark, lp->data, g_object_unref);
          g_signal_connect (G_OBJECT (action), "activate", G_CALLBACK (fmb_launcher_action_sendto_device), launcher);
          gtk_action_group_add_action (launcher->action_group, action);
          gtk_ui_manager_add_ui (launcher->ui_manager, launcher->ui_addons_merge_id,
                                 file_menu_path, name, name, GTK_UI_MANAGER_MENUITEM, FALSE);
          gtk_ui_manager_add_ui (launcher->ui_manager, launcher->ui_addons_merge_id,
                                 context_menu_path, name, name, GTK_UI_MANAGER_MENUITEM, FALSE);
          g_object_unref (action);

          icon = fmb_device_get_icon (lp->data);
          if (G_LIKELY (icon != NULL))
            {
              gtk_action_set_gicon (action, icon);
              g_object_unref (icon);
            }

          /* cleanup */
          g_free (name);
          g_free (tooltip);
          g_free (device_name);
        }

      /* free the devices list */
      g_list_free (devices);

      /* determine the sendto handlers for the selected files */
      handlers = fmb_sendto_model_get_matching (launcher->sendto_model, launcher->selected_files);
      if (G_LIKELY (handlers != NULL))
        {
          if (got_devices)
            {
              /* add separator between the devices and actions action */
              gtk_ui_manager_add_ui (launcher->ui_manager, launcher->ui_addons_merge_id,
                                     file_menu_path, "separator", NULL,
                                     GTK_UI_MANAGER_SEPARATOR, FALSE);
              gtk_ui_manager_add_ui (launcher->ui_manager, launcher->ui_addons_merge_id,
                                     context_menu_path, "separator", NULL,
                                     GTK_UI_MANAGER_SEPARATOR, FALSE);
            }

          /* add all handlers to the user interface */
          for (lp = handlers; lp != NULL; lp = lp->next, ++n)
            {
              /* generate a unique name and tooltip for the handler */
              label = g_app_info_get_name (lp->data);
              name = g_strdup_printf ("fmb-launcher-sendto%d-%p", n, launcher);
              tooltip = g_strdup_printf (ngettext ("Send the selected file to \"%s\"",
                                                   "Send the selected files to \"%s\"",
                                                   n_selected_files), label);

              /* allocate a new action for the handler */
              action = gtk_action_new (name, label, tooltip, NULL);
              gtk_action_set_gicon (action, g_app_info_get_icon (lp->data));
              g_object_set_qdata_full (G_OBJECT (action), fmb_launcher_handler_quark, lp->data, g_object_unref);
              g_signal_connect (G_OBJECT (action), "activate", G_CALLBACK (fmb_launcher_action_open), launcher);
              gtk_action_group_add_action (launcher->action_group, action);
              gtk_ui_manager_add_ui (launcher->ui_manager, launcher->ui_addons_merge_id,
                                     file_menu_path, name, name, GTK_UI_MANAGER_MENUITEM, FALSE);
              gtk_ui_manager_add_ui (launcher->ui_manager, launcher->ui_addons_merge_id,
                                     context_menu_path, name, name, GTK_UI_MANAGER_MENUITEM, FALSE);
              g_object_unref (G_OBJECT (action));

              /* cleanup */
              g_free (tooltip);
              g_free (name);
            }

          /* release the handler list */
          g_list_free (handlers);
        }
    }

  GDK_THREADS_LEAVE ();

  return FALSE;
}



static void
fmb_launcher_sendto_idle_destroy (gpointer user_data)
{
  FMB_LAUNCHER (user_data)->sendto_idle_id = 0;
}



/**
 * fmb_launcher_new:
 *
 * Allocates a new #FmbLauncher instance.
 *
 * Return value: the newly allocated #FmbLauncher.
 **/
FmbLauncher*
fmb_launcher_new (void)
{
  return g_object_new (FMB_TYPE_LAUNCHER, NULL);
}



/**
 * fmb_launcher_get_widget:
 * @launcher : a #FmbLauncher.
 *
 * Returns the #GtkWidget currently associated with @launcher.
 *
 * Return value: the widget associated with @launcher.
 **/
static GtkWidget*
fmb_launcher_get_widget (const FmbLauncher *launcher)
{
  _fmb_return_val_if_fail (FMB_IS_LAUNCHER (launcher), NULL);
  return launcher->widget;
}



/**
 * fmb_launcher_set_widget:
 * @launcher : a #FmbLauncher.
 * @widget   : a #GtkWidget or %NULL.
 *
 * Associates @launcher with @widget.
 **/
void
fmb_launcher_set_widget (FmbLauncher *launcher,
                            GtkWidget      *widget)
{
  _fmb_return_if_fail (FMB_IS_LAUNCHER (launcher));
  _fmb_return_if_fail (widget == NULL || GTK_IS_WIDGET (widget));

  /* disconnect from the previous widget */
  if (G_UNLIKELY (launcher->widget != NULL))
    {
      g_signal_handlers_disconnect_by_func (G_OBJECT (launcher->widget), fmb_launcher_widget_destroyed, launcher);
      g_object_unref (G_OBJECT (launcher->widget));
    }

  /* activate the new widget */
  launcher->widget = widget;

  /* connect to the new widget */
  if (G_LIKELY (widget != NULL))
    {
      g_object_ref (G_OBJECT (widget));
      g_signal_connect_swapped (G_OBJECT (widget), "destroy", G_CALLBACK (fmb_launcher_widget_destroyed), launcher);
    }

  /* notify listeners */
  g_object_notify_by_pspec (G_OBJECT (launcher), launcher_props[PROP_WIDGET]);
}




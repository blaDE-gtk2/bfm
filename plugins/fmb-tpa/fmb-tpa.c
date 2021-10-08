/*-
 * Copyright (c) 2006 Benedikt Meurer <benny@xfce.org>
 * Copyright (c) 2010 Nick Schermer <nick@xfce.org>
 * Copyright (c) 2010 Jannis Pohlmann <jannis@xfce.org>
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

#include <glib.h>

#include <gtk/gtk.h>

#include <libbladeutil/libbladeutil.h>

#include <libbladeui/libbladeui.h>

#include <libbladebar/libbladebar.h>
#include <libbladebar/blade-bar-macros.h>

#include <fmb-tpa/fmb-tpa-bindings.h>

typedef struct _FmbTpaClass FmbTpaClass;
typedef struct _FmbTpa      FmbTpa;



#define FMB_TYPE_TPA            (fmb_tpa_get_type ())
#define FMB_TPA(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), FMB_TYPE_TPA, FmbTpa))
#define FMB_TPA_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), FMB_TYPE_TPA, FmbTpaClass))
#define FMB_IS_TPA(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FMB_TYPE_TPA))
#define FMB_IS_TPA_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), FMB_TYPE_TPA))
#define FMB_TPA_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), FMB_TYPE_TPA, FmbTpaClass))



GType           fmb_tpa_get_type            (void);
void            fmb_tpa_register_type       (BladeBarTypeModule *type_module);
static void     fmb_tpa_finalize            (GObject             *object);
static void     fmb_tpa_construct           (BladeBarPlugin     *bar_plugin);
static gboolean fmb_tpa_size_changed        (BladeBarPlugin     *bar_plugin,
                                                gint                 size);
static void     fmb_tpa_error               (FmbTpa           *plugin,
                                                GError              *error);
static void     fmb_tpa_state               (FmbTpa           *plugin,
                                                gboolean             full);
static void     fmb_tpa_display_trash_reply (DBusGProxy          *proxy,
                                                GError              *error,
                                                gpointer             user_data);
static void     fmb_tpa_empty_trash_reply   (DBusGProxy          *proxy,
                                                GError              *error,
                                                gpointer             user_data);
static void     fmb_tpa_move_to_trash_reply (DBusGProxy          *proxy,
                                                GError              *error,
                                                gpointer             user_data);
static void     fmb_tpa_query_trash_reply   (DBusGProxy          *proxy,
                                                gboolean             full,
                                                GError              *error,
                                                gpointer             user_data);
static void     fmb_tpa_drag_data_received  (GtkWidget           *button,
                                                GdkDragContext      *context,
                                                gint                 x,
                                                gint                 y,
                                                GtkSelectionData    *selection_data,
                                                guint                info,
                                                guint                time,
                                                FmbTpa           *plugin);
static gboolean fmb_tpa_enter_notify_event  (GtkWidget           *button,
                                                GdkEventCrossing    *event,
                                                FmbTpa           *plugin);
static gboolean fmb_tpa_leave_notify_event  (GtkWidget           *button,
                                                GdkEventCrossing    *event,
                                                FmbTpa           *plugin);
static void     fmb_tpa_trash_changed       (DBusGProxy          *proxy,
                                                gboolean             full,
                                                FmbTpa           *plugin);
static void     fmb_tpa_display_trash       (FmbTpa           *plugin);
static void     fmb_tpa_empty_trash         (FmbTpa           *plugin);
static gboolean fmb_tpa_move_to_trash       (FmbTpa           *plugin,
                                                const gchar        **uri_list);
static void     fmb_tpa_query_trash         (FmbTpa           *plugin);



struct _FmbTpaClass
{
  BladeBarPluginClass __parent__;
};

struct _FmbTpa
{
  BladeBarPlugin __parent__;

  /* widgets */
  GtkWidget      *button;
  GtkWidget      *image;
  GtkWidget      *mi;

  DBusGProxy     *proxy;
  DBusGProxyCall *display_trash_call;
  DBusGProxyCall *empty_trash_call;
  DBusGProxyCall *move_to_trash_call;
  DBusGProxyCall *query_trash_call;
};

/* Target types for dropping to the trash can */
enum
{
  TARGET_TEXT_URI_LIST,
};

static const GtkTargetEntry drop_targets[] =
{
  { "text/uri-list", 0, TARGET_TEXT_URI_LIST, },
};



/* define the plugin */
BLADE_BAR_DEFINE_PLUGIN (FmbTpa, fmb_tpa)



static void
fmb_tpa_class_init (FmbTpaClass *klass)
{
  BladeBarPluginClass *plugin_class;
  GObjectClass         *gobject_class;

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = fmb_tpa_finalize;

  plugin_class = BLADE_BAR_PLUGIN_CLASS (klass);
  plugin_class->construct = fmb_tpa_construct;
  plugin_class->size_changed = fmb_tpa_size_changed;
}



static void
fmb_tpa_init (FmbTpa *plugin)
{
  DBusGConnection *connection;
  GError          *err = NULL;

  /* setup the button for the trash plugin */
  plugin->button = xfce_create_bar_button ();
  blade_bar_plugin_add_action_widget (BLADE_BAR_PLUGIN (plugin), plugin->button);
  gtk_drag_dest_set (plugin->button, GTK_DEST_DEFAULT_ALL, drop_targets, G_N_ELEMENTS (drop_targets), GDK_ACTION_MOVE);
  g_signal_connect_swapped (G_OBJECT (plugin->button), "clicked", G_CALLBACK (fmb_tpa_display_trash), plugin);
  g_signal_connect (G_OBJECT (plugin->button), "drag-data-received", G_CALLBACK (fmb_tpa_drag_data_received), plugin);
  g_signal_connect (G_OBJECT (plugin->button), "enter-notify-event", G_CALLBACK (fmb_tpa_enter_notify_event), plugin);
  g_signal_connect (G_OBJECT (plugin->button), "leave-notify-event", G_CALLBACK (fmb_tpa_leave_notify_event), plugin);
  gtk_container_add (GTK_CONTAINER (plugin), plugin->button);
  gtk_widget_show (plugin->button);

  /* setup the image for the trash plugin */
  plugin->image = gtk_image_new_from_icon_name ("user-trash", GTK_ICON_SIZE_BUTTON);
  gtk_container_add (GTK_CONTAINER (plugin->button), plugin->image);
  gtk_widget_show (plugin->image);

  /* prepare the menu item */
  plugin->mi = gtk_menu_item_new_with_mnemonic (_("_Empty Trash"));
  g_signal_connect_swapped (G_OBJECT (plugin->mi), "activate", G_CALLBACK (fmb_tpa_empty_trash), plugin);
  gtk_widget_show (plugin->mi);

  /* try to connect to the D-BUS session daemon */
  connection = dbus_g_bus_get (DBUS_BUS_SESSION, &err);
  if (G_UNLIKELY (connection == NULL))
    {
      /* we failed to connect, display an error plugin/tooltip */
      fmb_tpa_error (plugin, err);
      g_error_free (err);
    }
  else
    {
      /* grab a proxy for the /org/blade/FileManager object on org.blade.FileManager */
      plugin->proxy = dbus_g_proxy_new_for_name (connection, "org.blade.FileManager", "/org/blade/FileManager", "org.blade.Trash");

      /* connect to the "TrashChanged" signal */
      dbus_g_proxy_add_signal (plugin->proxy, "TrashChanged", G_TYPE_BOOLEAN, G_TYPE_INVALID);
      dbus_g_proxy_connect_signal (plugin->proxy, "TrashChanged", G_CALLBACK (fmb_tpa_trash_changed), plugin, NULL);
    }
}



static void
fmb_tpa_finalize (GObject *object)
{
  FmbTpa *plugin = FMB_TPA (object);

  /* release the proxy object */
  if (G_LIKELY (plugin->proxy != NULL))
    {
      /* cancel any pending calls */
      if (G_UNLIKELY (plugin->display_trash_call != NULL))
        dbus_g_proxy_cancel_call (plugin->proxy, plugin->display_trash_call);
      if (G_UNLIKELY (plugin->empty_trash_call != NULL))
        dbus_g_proxy_cancel_call (plugin->proxy, plugin->empty_trash_call);
      if (G_UNLIKELY (plugin->move_to_trash_call != NULL))
        dbus_g_proxy_cancel_call (plugin->proxy, plugin->move_to_trash_call);
      if (G_UNLIKELY (plugin->query_trash_call != NULL))
        dbus_g_proxy_cancel_call (plugin->proxy, plugin->query_trash_call);

      /* disconnect the signal and release the proxy */
      dbus_g_proxy_disconnect_signal (plugin->proxy, "TrashChanged", G_CALLBACK (fmb_tpa_trash_changed), plugin);
      g_object_unref (G_OBJECT (plugin->proxy));
    }

  (*G_OBJECT_CLASS (fmb_tpa_parent_class)->finalize) (object);
}



static void
fmb_tpa_construct (BladeBarPlugin *bar_plugin)
{
  FmbTpa *plugin = FMB_TPA (bar_plugin);

  /* make the plugin fit a single row */
  blade_bar_plugin_set_small (bar_plugin, TRUE);

  /* add the "Empty Trash" menu item */
  blade_bar_plugin_menu_insert_item (bar_plugin, GTK_MENU_ITEM (plugin->mi));

  /* update the state of the trash plugin */
  fmb_tpa_query_trash (plugin);
}



static gboolean
fmb_tpa_size_changed (BladeBarPlugin *bar_plugin,
                         gint             size)
{
  FmbTpa *plugin = FMB_TPA (bar_plugin);
  gint       image_size;

  /* make the plugin fit a single row */
  size /= blade_bar_plugin_get_nrows (bar_plugin);
  gtk_widget_set_size_request (GTK_WIDGET (bar_plugin), size, size);

#if LIBBLADEBAR_CHECK_VERSION (4,13,0)
  image_size = blade_bar_plugin_get_icon_size (bar_plugin);
#else
  image_size = size - 2; // fall-back for older bar versions
#endif
  gtk_image_set_pixel_size (GTK_IMAGE (plugin->image), image_size);

  return TRUE;
}



static void
fmb_tpa_error (FmbTpa *plugin,
                  GError    *error)
{
  gchar *tooltip;

  /* reset to empty first */
  fmb_tpa_state (plugin, FALSE);

  /* strip off additional whitespace */
  g_strstrip (error->message);

  /* tell the user that we failed to connect to the trash */
  tooltip = g_strdup_printf ("%s: %s.", _("Failed to connect to the Trash"), error->message);
  gtk_widget_set_tooltip_text (plugin->button, tooltip);
  g_free (tooltip);

  /* setup an error plugin */
  gtk_image_set_from_icon_name (GTK_IMAGE (plugin->image), "stock_dialog-error", GTK_ICON_SIZE_BUTTON);
}



static void
fmb_tpa_state (FmbTpa *plugin,
                  gboolean   full)
{
  /* tell the user whether the trash is full or empty */
  gtk_widget_set_tooltip_text (plugin->button, full ? _("Trash contains files") : _("Trash is empty"));

  /* setup the appropriate plugin */
  gtk_image_set_from_icon_name (GTK_IMAGE (plugin->image), full ? "user-trash-full" : "user-trash", GTK_ICON_SIZE_BUTTON);

  /* sensitivity of the menu item */
  gtk_widget_set_sensitive (plugin->mi, full);
}



static void
fmb_tpa_display_trash_reply (DBusGProxy *proxy,
                                GError     *error,
                                gpointer    user_data)
{
  FmbTpa *plugin = FMB_TPA (user_data);

  /* reset the call */
  plugin->display_trash_call = NULL;

  /* check if we failed */
  if (G_UNLIKELY (error != NULL))
    {
      /* display an error message to the user */
      g_strstrip (error->message);
      xfce_dialog_show_error (NULL, error, "%s.", _("Failed to connect to the Trash"));
      g_error_free (error);
    }
}



static void
fmb_tpa_empty_trash_reply (DBusGProxy *proxy,
                              GError     *error,
                              gpointer    user_data)
{
  FmbTpa *plugin = FMB_TPA (user_data);

  /* reset the call */
  plugin->empty_trash_call = NULL;

  /* check if we failed */
  if (G_UNLIKELY (error != NULL))
    {
      /* display an error message to the user */
      g_strstrip (error->message);
      xfce_dialog_show_error (NULL, error, "%s.", _("Failed to connect to the Trash"));
      g_error_free (error);
    }
  else
    {
      /* query the new state of the trash */
      fmb_tpa_query_trash (plugin);
    }
}



static void
fmb_tpa_move_to_trash_reply (DBusGProxy *proxy,
                                GError     *error,
                                gpointer    user_data)
{
  FmbTpa *plugin = FMB_TPA (user_data);

  /* reset the call */
  plugin->move_to_trash_call = NULL;

  /* check if we failed */
  if (G_UNLIKELY (error != NULL))
    {
      /* display an error message to the user */
      g_strstrip (error->message);
      xfce_dialog_show_error (NULL, error, "%s.", _("Failed to connect to the Trash"));
      g_error_free (error);
    }
  else
    {
      /* query the new state of the trash */
      fmb_tpa_query_trash (plugin);
    }
}



static void
fmb_tpa_query_trash_reply (DBusGProxy *proxy,
                              gboolean    full,
                              GError     *error,
                              gpointer    user_data)
{
  FmbTpa *plugin = FMB_TPA (user_data);

  /* reset the call */
  plugin->query_trash_call = NULL;

  /* check if we failed */
  if (G_UNLIKELY (error != NULL))
    {
      /* setup an error tooltip/plugin */
      fmb_tpa_error (plugin, error);
      g_error_free (error);
    }
  else
    {
      /* update the tooltip/plugin accordingly */
      fmb_tpa_state (plugin, full);
    }
}



static void
fmb_tpa_drag_data_received (GtkWidget        *button,
                               GdkDragContext   *context,
                               gint              x,
                               gint              y,
                               GtkSelectionData *selection_data,
                               guint             info,
                               guint             timestamp,
                               FmbTpa        *plugin)
{
  gboolean succeed = FALSE;
  gchar  **uri_list;

  g_return_if_fail (FMB_IS_TPA (plugin));
  g_return_if_fail (plugin->button == button);

  /* determine the type of drop we received */
  if (G_LIKELY (info == TARGET_TEXT_URI_LIST))
    {
      /* check if the data is valid for text/uri-list */
      uri_list = gtk_selection_data_get_uris (selection_data);
      if (G_LIKELY (uri_list != NULL))
        {
          succeed = fmb_tpa_move_to_trash (plugin, (const gchar **) uri_list);
          g_strfreev (uri_list);
        }
    }

  /* finish the drag */
  gtk_drag_finish (context, succeed, TRUE, timestamp);
}



static gboolean
fmb_tpa_enter_notify_event (GtkWidget        *button,
                               GdkEventCrossing *event,
                               FmbTpa        *plugin)
{
  g_return_val_if_fail (FMB_IS_TPA (plugin), FALSE);
  g_return_val_if_fail (plugin->button == button, FALSE);

  /* query the new state of the trash */
  fmb_tpa_query_trash (plugin);

  return FALSE;
}



static gboolean
fmb_tpa_leave_notify_event (GtkWidget        *button,
                               GdkEventCrossing *event,
                               FmbTpa        *plugin)
{
  g_return_val_if_fail (FMB_IS_TPA (plugin), FALSE);
  g_return_val_if_fail (plugin->button == button, FALSE);

  /* query the new state of the trash */
  fmb_tpa_query_trash (plugin);

  return FALSE;
}



static void
fmb_tpa_trash_changed (DBusGProxy *proxy,
                          gboolean    full,
                          FmbTpa  *plugin)
{
  g_return_if_fail (FMB_IS_TPA (plugin));
  g_return_if_fail (plugin->proxy == proxy);

  /* change the status plugin/tooltip appropriately */
  fmb_tpa_state (plugin, full);
}



static void
fmb_tpa_display_trash (FmbTpa *plugin)
{
  gchar *display_name;
  gchar *startup_id;

  g_return_if_fail (FMB_IS_TPA (plugin));

  /* check if we are connected to the bus */
  if (G_LIKELY (plugin->proxy != NULL))
    {
      /* cancel any pending call */
      if (G_UNLIKELY (plugin->display_trash_call != NULL))
        dbus_g_proxy_cancel_call (plugin->proxy, plugin->display_trash_call);

      /* schedule a new call */
      display_name = gdk_screen_make_display_name (gtk_widget_get_screen (GTK_WIDGET (plugin)));
      startup_id = g_strdup_printf ("_TIME%d", gtk_get_current_event_time ());
      plugin->display_trash_call = org_xfce_Trash_display_trash_async (plugin->proxy, display_name, startup_id, fmb_tpa_display_trash_reply, plugin);
      g_free (startup_id);
      g_free (display_name);
    }
}



static void
fmb_tpa_empty_trash (FmbTpa *plugin)
{
  gchar *display_name;
  gchar *startup_id;

  g_return_if_fail (FMB_IS_TPA (plugin));

  /* check if we are connected to the bus */
  if (G_LIKELY (plugin->proxy != NULL))
    {
      /* cancel any pending call */
      if (G_UNLIKELY (plugin->empty_trash_call != NULL))
        dbus_g_proxy_cancel_call (plugin->proxy, plugin->empty_trash_call);

      /* schedule a new call */
      display_name = gdk_screen_make_display_name (gtk_widget_get_screen (GTK_WIDGET (plugin)));
      startup_id = g_strdup_printf ("_TIME%d", gtk_get_current_event_time ());
      plugin->empty_trash_call = org_xfce_Trash_empty_trash_async (plugin->proxy, display_name, startup_id, fmb_tpa_empty_trash_reply, plugin);
      g_free (startup_id);
      g_free (display_name);
    }
}



static gboolean
fmb_tpa_move_to_trash (FmbTpa    *plugin,
                          const gchar **uri_list)
{
  gchar *display_name;
  gchar *startup_id;

  g_return_val_if_fail (FMB_IS_TPA (plugin), FALSE);
  g_return_val_if_fail (uri_list != NULL, FALSE);

  /* check if we are connected to the bus */
  if (G_UNLIKELY (plugin->proxy == NULL))
    return FALSE;

  /* cancel any pending call */
  if (G_UNLIKELY (plugin->move_to_trash_call != NULL))
    dbus_g_proxy_cancel_call (plugin->proxy, plugin->move_to_trash_call);

  /* schedule a new call */
  display_name = gdk_screen_make_display_name (gtk_widget_get_screen (GTK_WIDGET (plugin)));
  startup_id = g_strdup_printf ("_TIME%d", gtk_get_current_event_time ());
  plugin->move_to_trash_call = org_xfce_Trash_move_to_trash_async (plugin->proxy, uri_list, display_name, startup_id, fmb_tpa_move_to_trash_reply, plugin);
  g_free (startup_id);
  g_free (display_name);

  return TRUE;
}



static void
fmb_tpa_query_trash (FmbTpa *plugin)
{
  g_return_if_fail (FMB_IS_TPA (plugin));

  /* check if we are connected to the bus */
  if (G_LIKELY (plugin->proxy != NULL))
    {
      /* cancel any pending call */
      if (G_UNLIKELY (plugin->query_trash_call != NULL))
        dbus_g_proxy_cancel_call (plugin->proxy, plugin->query_trash_call);

      /* schedule a new call */
      plugin->query_trash_call = org_xfce_Trash_query_trash_async (plugin->proxy, fmb_tpa_query_trash_reply, plugin);
    }
}

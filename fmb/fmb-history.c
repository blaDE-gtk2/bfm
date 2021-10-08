/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2005-2006 Benedikt Meurer <benny@xfce.org>
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

#include <fmb/fmb-gobject-extensions.h>
#include <fmb/fmb-history-action.h>
#include <fmb/fmb-history.h>
#include <fmb/fmb-icon-factory.h>
#include <fmb/fmb-navigator.h>
#include <fmb/fmb-private.h>
#include <fmb/fmb-dialogs.h>



/* Property identifiers */
enum
{
  PROP_0,
  PROP_ACTION_GROUP,
  PROP_CURRENT_DIRECTORY,
};



static void            fmb_history_navigator_init         (FmbNavigatorIface *iface);
static void            fmb_history_dispose                (GObject              *object);
static void            fmb_history_finalize               (GObject              *object);
static void            fmb_history_get_property           (GObject              *object,
                                                              guint                 prop_id,
                                                              GValue               *value,
                                                              GParamSpec           *pspec);
static void            fmb_history_set_property           (GObject              *object,
                                                              guint                 prop_id,
                                                              const GValue         *value,
                                                              GParamSpec           *pspec);
static FmbFile     *fmb_history_get_current_directory  (FmbNavigator      *navigator);
static void            fmb_history_set_current_directory  (FmbNavigator      *navigator,
                                                              FmbFile           *current_directory);
static void            fmb_history_go_back                (FmbHistory        *history,
                                                              GFile                *goto_file);
static void            fmb_history_go_forward             (FmbHistory        *history,
                                                              GFile                *goto_file);
static void            fmb_history_action_back            (GtkAction            *action,
                                                              FmbHistory        *history);
static void            fmb_history_action_back_nth        (GtkWidget            *item,
                                                              FmbHistory        *history);
static void            fmb_history_action_forward         (GtkAction            *action,
                                                              FmbHistory        *history);
static void            fmb_history_action_forward_nth     (GtkWidget            *item,
                                                              FmbHistory        *history);
static void            fmb_history_show_menu              (GtkAction            *action,
                                                              GtkWidget            *menu,
                                                              FmbHistory        *history);
static GtkActionGroup *fmb_history_get_action_group       (const FmbHistory  *history);
static void            fmb_history_set_action_group       (FmbHistory        *history,
                                                              GtkActionGroup       *action_group);


struct _FmbHistoryClass
{
  GObjectClass __parent__;
};

struct _FmbHistory
{
  GObject __parent__;

  FmbFile     *current_directory;

  GtkActionGroup *action_group;

  GtkAction      *action_back;
  GtkAction      *action_forward;

  GSList         *back_list;
  GSList         *forward_list;
};



G_DEFINE_TYPE_WITH_CODE (FmbHistory, fmb_history, G_TYPE_OBJECT,
    G_IMPLEMENT_INTERFACE (FMB_TYPE_NAVIGATOR, fmb_history_navigator_init))



static GQuark fmb_history_display_name_quark;
static GQuark fmb_history_gfile_quark;



static void
fmb_history_class_init (FmbHistoryClass *klass)
{
  GObjectClass *gobject_class;

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->dispose = fmb_history_dispose;
  gobject_class->finalize = fmb_history_finalize;
  gobject_class->get_property = fmb_history_get_property;
  gobject_class->set_property = fmb_history_set_property;

  fmb_history_display_name_quark = g_quark_from_static_string ("fmb-history-display-name");
  fmb_history_gfile_quark = g_quark_from_static_string ("fmb-history-gfile");

  /**
   * FmbHistory::action-group:
   *
   * The #GtkActionGroup to which the #FmbHistory<!---->s
   * actions "back" and "forward" should be connected.
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_ACTION_GROUP,
                                   g_param_spec_object ("action-group",
                                                        "action-group",
                                                        "action-group",
                                                        GTK_TYPE_ACTION_GROUP,
                                                        BLXO_PARAM_READWRITE));

  /**
   * FmbHistory::current-directory:
   *
   * Inherited from #FmbNavigator.
   **/
  g_object_class_override_property (gobject_class,
                                    PROP_CURRENT_DIRECTORY,
                                    "current-directory");
}



static void
fmb_history_navigator_init (FmbNavigatorIface *iface)
{
  iface->get_current_directory = fmb_history_get_current_directory;
  iface->set_current_directory = fmb_history_set_current_directory;
}



static void
fmb_history_init (FmbHistory *history)
{
  /* create the "back" action */
  history->action_back = fmb_history_action_new ("back", _("Back"), _("Go to the previous visited folder"), GTK_STOCK_GO_BACK);
  g_signal_connect (G_OBJECT (history->action_back), "activate", G_CALLBACK (fmb_history_action_back), history);
  g_signal_connect (G_OBJECT (history->action_back), "show-menu", G_CALLBACK (fmb_history_show_menu), history);
  gtk_action_set_sensitive (history->action_back, FALSE);

  /* create the "forward" action */
  history->action_forward = fmb_history_action_new ("forward", _("Forward"), _("Go to the next visited folder"), GTK_STOCK_GO_FORWARD);
  g_signal_connect (G_OBJECT (history->action_forward), "activate", G_CALLBACK (fmb_history_action_forward), history);
  g_signal_connect (G_OBJECT (history->action_forward), "show-menu", G_CALLBACK (fmb_history_show_menu), history);
  gtk_action_set_sensitive (history->action_forward, FALSE);
}



static void
fmb_history_dispose (GObject *object)
{
  FmbHistory *history = FMB_HISTORY (object);

  /* disconnect from the current directory */
  fmb_navigator_set_current_directory (FMB_NAVIGATOR (history), NULL);

  /* disconnect from the action group */
  fmb_history_set_action_group (history, NULL);

  (*G_OBJECT_CLASS (fmb_history_parent_class)->dispose) (object);
}



static void
fmb_history_finalize (GObject *object)
{
  FmbHistory *history = FMB_HISTORY (object);

  /* disconnect from the "forward" action */
  g_signal_handlers_disconnect_matched (G_OBJECT (history->action_forward), G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, history);
  g_object_unref (G_OBJECT (history->action_forward));

  /* disconnect from the "back" action */
  g_signal_handlers_disconnect_matched (G_OBJECT (history->action_back), G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, history);
  g_object_unref (G_OBJECT (history->action_back));

  /* release the "forward" and "back" lists */
  g_slist_free_full (history->forward_list, g_object_unref);
  g_slist_free_full (history->back_list, g_object_unref);

  (*G_OBJECT_CLASS (fmb_history_parent_class)->finalize) (object);
}



static void
fmb_history_get_property (GObject    *object,
                             guint       prop_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
  FmbHistory *history = FMB_HISTORY (object);

  switch (prop_id)
    {
    case PROP_ACTION_GROUP:
      g_value_set_object (value, fmb_history_get_action_group (history));
      break;

    case PROP_CURRENT_DIRECTORY:
      g_value_set_object (value, fmb_navigator_get_current_directory (FMB_NAVIGATOR (history)));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static void
fmb_history_set_property (GObject      *object,
                             guint         prop_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
  FmbHistory *history = FMB_HISTORY (object);

  switch (prop_id)
    {
    case PROP_ACTION_GROUP:
      fmb_history_set_action_group (history, g_value_get_object (value));
      break;

    case PROP_CURRENT_DIRECTORY:
      fmb_navigator_set_current_directory (FMB_NAVIGATOR (history), g_value_get_object (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static FmbFile*
fmb_history_get_current_directory (FmbNavigator *navigator)
{
  return FMB_HISTORY (navigator)->current_directory;
}



static GFile *
fmb_history_get_gfile (FmbFile *file)
{
  GFile       *gfile;
  const gchar *display_name;

  _fmb_return_val_if_fail (FMB_IS_FILE (file), NULL);

  gfile = fmb_file_get_file (file);

  display_name = fmb_file_get_display_name (file);
  g_object_set_qdata_full (G_OBJECT (gfile), fmb_history_display_name_quark,
                           g_strdup (display_name), g_free);

  return g_object_ref (gfile);
}



static void
fmb_history_set_current_directory (FmbNavigator *navigator,
                                      FmbFile      *current_directory)
{
  FmbHistory *history = FMB_HISTORY (navigator);
  GFile         *gfile;

  /* verify that we don't already use that directory */
  if (G_UNLIKELY (current_directory == history->current_directory))
    return;

  /* if the new directory is the first one in the forward history, we
   * just move forward one step instead of clearing the whole forward
   * history */
  if (current_directory != NULL
      && history->forward_list != NULL
      && g_file_equal (fmb_file_get_file (current_directory), history->forward_list->data))
    {
      fmb_history_go_forward (history, history->forward_list->data);
    }
  else
    {
      /* clear the "forward" list */
      gtk_action_set_sensitive (history->action_forward, FALSE);
      g_slist_free_full (history->forward_list, g_object_unref);
      history->forward_list = NULL;

      /* prepend the previous current directory to the "back" list */
      if (G_LIKELY (history->current_directory != NULL))
        {
          gfile = fmb_history_get_gfile (history->current_directory);
          history->back_list = g_slist_prepend (history->back_list, gfile);
          gtk_action_set_sensitive (history->action_back, TRUE);

          g_object_unref (history->current_directory);
        }

      /* activate the new current directory */
      history->current_directory = current_directory;

      /* connect to the new current directory */
      if (G_LIKELY (current_directory != NULL))
        g_object_ref (G_OBJECT (current_directory));
    }

  /* notify listeners */
  g_object_notify (G_OBJECT (history), "current-directory");
}



static void
fmb_history_error_not_found (GFile    *goto_file,
                                gpointer  parent)
{
  gchar  *parse_name;
  gchar  *path;
  GError *error = NULL;

  g_set_error_literal (&error, G_FILE_ERROR, G_FILE_ERROR_EXIST,
                       _("The item will be removed from the history"));

  path = g_file_get_path (goto_file);
  if (path == NULL)
    {
      path = g_file_get_uri (goto_file);
      parse_name = g_uri_unescape_string (path, NULL);
      g_free (path);
    }
  else
    parse_name = g_file_get_parse_name (goto_file);

  fmb_dialogs_show_error (parent, error, _("Could not find \"%s\""), parse_name);
  g_free (parse_name);

  g_error_free (error);
}



static void
fmb_history_go_back (FmbHistory  *history,
                        GFile          *goto_file)
{
  GFile      *gfile;
  GSList     *lp;
  GSList     *lnext;
  FmbFile *directory;

  _fmb_return_if_fail (FMB_IS_HISTORY (history));
  _fmb_return_if_fail (G_IS_FILE (goto_file));

  /* check if the directory still exists */
  directory = fmb_file_get (goto_file, NULL);
  if (directory == NULL || ! fmb_file_is_mounted (directory))
    {
      fmb_history_error_not_found (goto_file, NULL);

      /* delete item from the history */
      lp = g_slist_find (history->back_list, goto_file);
      if (lp != NULL)
        {
          g_object_unref (lp->data);
          history->back_list = g_slist_delete_link (history->back_list, lp);
        }

      goto update_actions;
    }

  /* prepend the previous current directory to the "forward" list */
  if (G_LIKELY (history->current_directory != NULL))
    {
      gfile = fmb_history_get_gfile (history->current_directory);
      history->forward_list = g_slist_prepend (history->forward_list, gfile);

      g_object_unref (history->current_directory);
      history->current_directory = NULL;
    }

  /* add all the items of the back list to the "forward" list until
   * the target file is reached  */
  for (lp = history->back_list; lp != NULL; lp = lnext)
    {
      lnext = lp->next;

      if (g_file_equal (goto_file, G_FILE (lp->data)))
        {
          if (directory != NULL)
            history->current_directory = g_object_ref (directory);

          /* remove the new directory from the list */
          g_object_unref (lp->data);
          history->back_list = g_slist_delete_link (history->back_list, lp);

          break;
        }

      /* remove item from the list */
      history->back_list = g_slist_remove_link (history->back_list, lp);

      /* prepend element to the other list */
      lp->next = history->forward_list;
      history->forward_list = lp;
    }

  if (directory != NULL)
    g_object_unref (directory);

  /* tell the other modules to change the current directory */
  if (G_LIKELY (history->current_directory != NULL))
    fmb_navigator_change_directory (FMB_NAVIGATOR (history), history->current_directory);

  update_actions:

  /* update the sensitivity of the actions */
  gtk_action_set_sensitive (history->action_back, (history->back_list != NULL));
  gtk_action_set_sensitive (history->action_forward, (history->forward_list != NULL));
}



static void
fmb_history_go_forward (FmbHistory  *history,
                           GFile          *goto_file)
{
  GFile      *gfile;
  GSList     *lnext;
  GSList     *lp;
  FmbFile *directory;

  _fmb_return_if_fail (FMB_IS_HISTORY (history));
  _fmb_return_if_fail (G_IS_FILE (goto_file));

  /* check if the directory still exists */
  directory = fmb_file_get (goto_file, NULL);
  if (directory == NULL || ! fmb_file_is_mounted (directory))
    {
      fmb_history_error_not_found (goto_file, NULL);

      /* delete item from the history */
      lp = g_slist_find (history->forward_list, goto_file);
      if (lp != NULL)
        {
          g_object_unref (lp->data);
          history->forward_list = g_slist_delete_link (history->forward_list, lp);
        }

      goto update_actions;
    }

  /* prepend the previous current directory to the "back" list */
  if (G_LIKELY (history->current_directory != NULL))
    {
      gfile = fmb_history_get_gfile (history->current_directory);
      history->back_list = g_slist_prepend (history->back_list, gfile);

      g_object_unref (history->current_directory);
      history->current_directory = NULL;
    }

  for (lp = history->forward_list; lp != NULL; lp = lnext)
    {
      lnext = lp->next;

      if (g_file_equal (goto_file, G_FILE (lp->data)))
        {
          if (directory != NULL)
            history->current_directory = g_object_ref (directory);

          /* remove the new dirctory from the list */
          g_object_unref (lp->data);
          history->forward_list = g_slist_delete_link (history->forward_list, lp);

          break;
        }

      /* remove item from the list */
      history->forward_list = g_slist_remove_link (history->forward_list, lp);

      /* prepend item to the back list */
      lp->next = history->back_list;
      history->back_list = lp;
    }

  if (directory != NULL)
    g_object_unref (directory);

  /* tell the other modules to change the current directory */
  if (G_LIKELY (history->current_directory != NULL))
    fmb_navigator_change_directory (FMB_NAVIGATOR (history), history->current_directory);

  update_actions:

  /* update the sensitivity of the actions */
  gtk_action_set_sensitive (history->action_back, (history->back_list != NULL));
  gtk_action_set_sensitive (history->action_forward, (history->forward_list != NULL));
}



static void
fmb_history_action_back (GtkAction     *action,
                            FmbHistory *history)
{
  _fmb_return_if_fail (GTK_IS_ACTION (action));
  _fmb_return_if_fail (FMB_IS_HISTORY (history));

  /* go back one step */
  if (history->back_list != NULL)
    fmb_history_go_back (history, history->back_list->data);
}



static void
fmb_history_action_back_nth (GtkWidget     *item,
                                FmbHistory *history)
{
  GFile *file;

  _fmb_return_if_fail (GTK_IS_MENU_ITEM (item));
  _fmb_return_if_fail (FMB_IS_HISTORY (history));

  file = g_object_get_qdata (G_OBJECT (item), fmb_history_gfile_quark);
  if (G_LIKELY (file != NULL))
    fmb_history_go_back (history, file);
}



static void
fmb_history_action_forward (GtkAction     *action,
                               FmbHistory *history)
{
  _fmb_return_if_fail (GTK_IS_ACTION (action));
  _fmb_return_if_fail (FMB_IS_HISTORY (history));

  /* go forward one step */
  if (history->forward_list != NULL)
    fmb_history_go_forward (history, history->forward_list->data);
}



static void
fmb_history_action_forward_nth (GtkWidget     *item,
                                   FmbHistory *history)
{
  GFile *file;

  _fmb_return_if_fail (GTK_IS_MENU_ITEM (item));
  _fmb_return_if_fail (FMB_IS_HISTORY (history));

  file = g_object_get_qdata (G_OBJECT (item), fmb_history_gfile_quark);
  if (G_LIKELY (file != NULL))
    fmb_history_go_forward (history, file);
}



static void
fmb_history_show_menu (GtkAction     *action,
                          GtkWidget     *menu,
                          FmbHistory *history)
{
  FmbIconFactory *icon_factory;
  GtkIconTheme      *icon_theme;
  GCallback          handler;
  GtkWidget         *image;
  GtkWidget         *item;
  GdkPixbuf         *icon;
  GSList            *lp;
  FmbFile        *file;
  const gchar       *display_name;
  const gchar       *icon_name;
  gchar             *parse_name;

  _fmb_return_if_fail (GTK_IS_ACTION (action));
  _fmb_return_if_fail (GTK_IS_MENU_SHELL (menu));
  _fmb_return_if_fail (FMB_IS_HISTORY (history));

  /* determine the icon factory to use to load the icons */
  icon_theme = gtk_icon_theme_get_for_screen (gtk_widget_get_screen (menu));
  icon_factory = fmb_icon_factory_get_for_icon_theme (icon_theme);

  /* check if we have "Back" or "Forward" here */
  if (action == history->action_back)
    {
      /* display the "back" list */
      lp = history->back_list;
      handler = G_CALLBACK (fmb_history_action_back_nth);
    }
  else
    {
      /* display the "forward" list */
      lp = history->forward_list;
      handler = G_CALLBACK (fmb_history_action_forward_nth);
    }

  /* add menu items for all list items */
  for (;lp != NULL; lp = lp->next)
    {
      /* add an item for this file */
      display_name = g_object_get_qdata (G_OBJECT (lp->data), fmb_history_display_name_quark);
      item = gtk_image_menu_item_new_with_label (display_name);
      g_object_set_qdata (G_OBJECT (item), fmb_history_gfile_quark, lp->data);
      g_signal_connect (G_OBJECT (item), "activate", handler, history);
      gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
      gtk_widget_show (item);

      parse_name = g_file_get_parse_name (lp->data);
      gtk_widget_set_tooltip_text (item, parse_name);
      g_free (parse_name);

      file = fmb_file_cache_lookup (lp->data);
      image = NULL;
      if (file != NULL)
        {
          /* load the icon for the file */
          icon = fmb_icon_factory_load_file_icon (icon_factory, file, FMB_FILE_ICON_STATE_DEFAULT, 16);
          if (icon != NULL)
            {
              /* setup the image for the file */
              image = gtk_image_new_from_pixbuf (icon);
              g_object_unref (G_OBJECT (icon));
            }

          g_object_unref (file);
        }

      if (image == NULL)
        {
          /* some custom likely alternatives */
          if (fmb_g_file_is_home (lp->data))
            icon_name = "user-home";
          else if (!g_file_has_uri_scheme (lp->data, "file"))
            icon_name = "folder-remote";
          else if (fmb_g_file_is_root (lp->data))
            icon_name = "drive-harddisk";
          else
            icon_name = "folder";

          image = gtk_image_new_from_icon_name (icon_name, GTK_ICON_SIZE_MENU);
        }
      gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (item), image);
    }

  /* release the icon factory */
  g_object_unref (G_OBJECT (icon_factory));
}



/**
 * fmb_history_get_action_group:
 * @history : a #FmbHistory.
 *
 * Returns the #GtkActionGroup to which @history
 * is currently attached, or %NULL if @history is
 * not attached to any #GtkActionGroup right now.
 *
 * Return value: the #GtkActionGroup to which
 *               @history is currently attached.
 **/
static GtkActionGroup*
fmb_history_get_action_group (const FmbHistory *history)
{
  _fmb_return_val_if_fail (FMB_IS_HISTORY (history), NULL);
  return history->action_group;
}



/**
 * fmb_history_set_action_group:
 * @history      : a #FmbHistory.
 * @action_group : a #GtkActionGroup or %NULL.
 *
 * Attaches @history to the specified @action_group,
 * and thereby registers the actions "back" and
 * "forward" provided by @history on the given
 * @action_group.
 **/
static void
fmb_history_set_action_group (FmbHistory  *history,
                                 GtkActionGroup *action_group)
{
  _fmb_return_if_fail (FMB_IS_HISTORY (history));
  _fmb_return_if_fail (action_group == NULL || GTK_IS_ACTION_GROUP (action_group));

  /* verify that we don't already use that action group */
  if (G_UNLIKELY (history->action_group == action_group))
    return;

  /* disconnect from the previous action group */
  if (G_UNLIKELY (history->action_group != NULL))
    {
      gtk_action_group_remove_action (history->action_group, history->action_back);
      gtk_action_group_remove_action (history->action_group, history->action_forward);
      g_object_unref (G_OBJECT (history->action_group));
    }

  /* activate the new action group */
  history->action_group = action_group;

  /* connect to the new action group */
  if (G_LIKELY (action_group != NULL))
    {
      g_object_ref (G_OBJECT (action_group));
      gtk_action_group_add_action_with_accel (action_group, history->action_back, "<alt>Left");
      gtk_action_group_add_action_with_accel (action_group, history->action_forward, "<alt>Right");
    }

  /* notify listeners */
  g_object_notify (G_OBJECT (history), "action-group");
}



FmbHistory *
fmb_history_copy (FmbHistory  *history,
                     GtkActionGroup *action_group)
{
  FmbHistory *copy;
  GSList        *lp;

  _fmb_return_val_if_fail (history == NULL || FMB_IS_HISTORY (history), NULL);
  _fmb_return_val_if_fail (action_group == NULL || GTK_IS_ACTION_GROUP (action_group), NULL);

  if (G_UNLIKELY (history == NULL))
    return NULL;

  copy = g_object_new (FMB_TYPE_HISTORY, NULL);

  /* take a ref on the current directory */
  copy->current_directory = g_object_ref (history->current_directory);

  /* set the action group */
  fmb_history_set_action_group (copy, action_group);

  /* copy the back list */
  for (lp = history->back_list; lp != NULL; lp = lp->next)
      copy->back_list = g_slist_append (copy->back_list, g_object_ref (G_OBJECT (lp->data)));

  /* copy the forward list */
  for (lp = history->forward_list; lp != NULL; lp = lp->next)
      copy->forward_list = g_slist_append (copy->forward_list, g_object_ref (G_OBJECT (lp->data)));

  /* update the sensitivity of the actions */
  gtk_action_set_sensitive (copy->action_back, (copy->back_list != NULL));
  gtk_action_set_sensitive (copy->action_forward, (copy->forward_list != NULL));

  return copy;
}



/**
 * fmb_file_history_peek_back:
 * @history : a #FmbHistory.
 *
 * Returns the previous directory in the history.
 *
 * The returned #FmbFile must be released by the caller.
 *
 * Return value: the previous #FmbFile in the history.
 **/
FmbFile *
fmb_history_peek_back (FmbHistory *history)
{
  FmbFile *result = NULL;

  _fmb_return_val_if_fail (FMB_IS_HISTORY (history), NULL);

  /* pick the first (conceptually the last) file in the back list, if there are any */
  if (history->back_list != NULL)
    result = fmb_file_get (history->back_list->data, NULL);

  return result;
}



/**
 * fmb_file_history_peek_forward:
 * @history : a #FmbHistory.
 *
 * Returns the next directory in the history. This often but not always
 * refers to a child of the current directory.
 *
 * The returned #FmbFile must be released by the caller.
 *
 * Return value: the next #FmbFile in the history.
 **/
FmbFile *
fmb_history_peek_forward (FmbHistory *history)
{
  FmbFile *result = NULL;

  _fmb_return_val_if_fail (FMB_IS_HISTORY (history), NULL);

  /* pick the first file in the forward list, if there are any */
  if (history->forward_list != NULL)
    result = fmb_file_get (history->forward_list->data, NULL);

  return result;
}

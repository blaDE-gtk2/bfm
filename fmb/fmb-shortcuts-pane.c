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
#include <fmb/fmb-private.h>
#include <fmb/fmb-shortcuts-model.h>
#include <fmb/fmb-shortcuts-pane.h>
#include <fmb/fmb-shortcuts-pane-ui.h>
#include <fmb/fmb-shortcuts-view.h>
#include <fmb/fmb-side-pane.h>
#include <fmb/fmb-stock.h>



enum
{
  PROP_0,
  PROP_CURRENT_DIRECTORY,
  PROP_SELECTED_FILES,
  PROP_SHOW_HIDDEN,
  PROP_UI_MANAGER,
};



static void          fmb_shortcuts_pane_component_init        (FmbComponentIface     *iface);
static void          fmb_shortcuts_pane_navigator_init        (FmbNavigatorIface     *iface);
static void          fmb_shortcuts_pane_side_pane_init        (FmbSidePaneIface      *iface);
static void          fmb_shortcuts_pane_dispose               (GObject                  *object);
static void          fmb_shortcuts_pane_finalize              (GObject                  *object);
static void          fmb_shortcuts_pane_get_property          (GObject                  *object,
                                                                  guint                     prop_id,
                                                                  GValue                   *value,
                                                                  GParamSpec               *pspec);
static void          fmb_shortcuts_pane_set_property          (GObject                  *object,
                                                                  guint                     prop_id,
                                                                  const GValue             *value,
                                                                  GParamSpec               *pspec);
static FmbFile   *fmb_shortcuts_pane_get_current_directory (FmbNavigator          *navigator);
static void          fmb_shortcuts_pane_set_current_directory (FmbNavigator          *navigator,
                                                                  FmbFile               *current_directory);
static GList        *fmb_shortcuts_pane_get_selected_files    (FmbComponent          *component);
static void          fmb_shortcuts_pane_set_selected_files    (FmbComponent          *component,
                                                                  GList                    *selected_files);
static GtkUIManager *fmb_shortcuts_pane_get_ui_manager        (FmbComponent          *component);
static void          fmb_shortcuts_pane_set_ui_manager        (FmbComponent          *component,
                                                                  GtkUIManager             *ui_manager);
static void          fmb_shortcuts_pane_action_shortcuts_add  (GtkAction                *action,
                                                                  FmbShortcutsPane      *shortcuts_pane);



struct _FmbShortcutsPaneClass
{
  GtkScrolledWindowClass __parent__;
};

struct _FmbShortcutsPane
{
  GtkScrolledWindow __parent__;

  FmbFile       *current_directory;
  GList            *selected_files;

  GtkActionGroup   *action_group;
  GtkUIManager     *ui_manager;
  guint             ui_merge_id;

  GtkWidget        *view;

  guint             idle_select_directory;
};



static const GtkActionEntry action_entries[] =
{
  { "sendto-shortcuts", FMB_STOCK_SHORTCUTS, "", NULL, NULL, G_CALLBACK (fmb_shortcuts_pane_action_shortcuts_add), },
};



G_DEFINE_TYPE_WITH_CODE (FmbShortcutsPane, fmb_shortcuts_pane, GTK_TYPE_SCROLLED_WINDOW,
    G_IMPLEMENT_INTERFACE (FMB_TYPE_NAVIGATOR, fmb_shortcuts_pane_navigator_init)
    G_IMPLEMENT_INTERFACE (FMB_TYPE_COMPONENT, fmb_shortcuts_pane_component_init)
    G_IMPLEMENT_INTERFACE (FMB_TYPE_SIDE_PANE, fmb_shortcuts_pane_side_pane_init))



static void
fmb_shortcuts_pane_class_init (FmbShortcutsPaneClass *klass)
{
  GObjectClass *gobject_class;

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->dispose = fmb_shortcuts_pane_dispose;
  gobject_class->finalize = fmb_shortcuts_pane_finalize;
  gobject_class->get_property = fmb_shortcuts_pane_get_property;
  gobject_class->set_property = fmb_shortcuts_pane_set_property;

  /* override FmbNavigator's properties */
  g_object_class_override_property (gobject_class, PROP_CURRENT_DIRECTORY, "current-directory");

  /* override FmbComponent's properties */
  g_object_class_override_property (gobject_class, PROP_SELECTED_FILES, "selected-files");
  g_object_class_override_property (gobject_class, PROP_UI_MANAGER, "ui-manager");

  /* override FmbSidePane's properties */
  g_object_class_override_property (gobject_class, PROP_SHOW_HIDDEN, "show-hidden");
}



static void
fmb_shortcuts_pane_component_init (FmbComponentIface *iface)
{
  iface->get_selected_files = fmb_shortcuts_pane_get_selected_files;
  iface->set_selected_files = fmb_shortcuts_pane_set_selected_files;
  iface->get_ui_manager = fmb_shortcuts_pane_get_ui_manager;
  iface->set_ui_manager = fmb_shortcuts_pane_set_ui_manager;
}



static void
fmb_shortcuts_pane_navigator_init (FmbNavigatorIface *iface)
{
  iface->get_current_directory = fmb_shortcuts_pane_get_current_directory;
  iface->set_current_directory = fmb_shortcuts_pane_set_current_directory;
}



static void
fmb_shortcuts_pane_side_pane_init (FmbSidePaneIface *iface)
{
  iface->get_show_hidden = (gpointer) blxo_noop_false;
  iface->set_show_hidden = (gpointer) blxo_noop;
}



static void
fmb_shortcuts_pane_init (FmbShortcutsPane *shortcuts_pane)
{
  /* setup the action group for the shortcuts actions */
  shortcuts_pane->action_group = gtk_action_group_new ("FmbShortcutsPane");
  gtk_action_group_set_translation_domain (shortcuts_pane->action_group, GETTEXT_PACKAGE);
  gtk_action_group_add_actions (shortcuts_pane->action_group, action_entries, G_N_ELEMENTS (action_entries), shortcuts_pane);

  /* configure the GtkScrolledWindow */
  gtk_scrolled_window_set_hadjustment (GTK_SCROLLED_WINDOW (shortcuts_pane), NULL);
  gtk_scrolled_window_set_vadjustment (GTK_SCROLLED_WINDOW (shortcuts_pane), NULL);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (shortcuts_pane), GTK_SHADOW_IN);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (shortcuts_pane), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

  /* allocate the shortcuts view */
  shortcuts_pane->view = fmb_shortcuts_view_new ();
  gtk_container_add (GTK_CONTAINER (shortcuts_pane), shortcuts_pane->view);
  gtk_widget_show (shortcuts_pane->view);

  /* connect the "shortcut-activated" signal */
  g_signal_connect_swapped (G_OBJECT (shortcuts_pane->view), "shortcut-activated", G_CALLBACK (fmb_navigator_change_directory), shortcuts_pane);
  g_signal_connect_swapped (G_OBJECT (shortcuts_pane->view), "shortcut-activated-tab", G_CALLBACK (fmb_navigator_open_new_tab), shortcuts_pane);
}



static void
fmb_shortcuts_pane_dispose (GObject *object)
{
  FmbShortcutsPane *shortcuts_pane = FMB_SHORTCUTS_PANE (object);

  fmb_navigator_set_current_directory (FMB_NAVIGATOR (shortcuts_pane), NULL);
  fmb_component_set_selected_files (FMB_COMPONENT (shortcuts_pane), NULL);
  fmb_component_set_ui_manager (FMB_COMPONENT (shortcuts_pane), NULL);

  (*G_OBJECT_CLASS (fmb_shortcuts_pane_parent_class)->dispose) (object);
}



static void
fmb_shortcuts_pane_finalize (GObject *object)
{
  FmbShortcutsPane *shortcuts_pane = FMB_SHORTCUTS_PANE (object);

  /* release our action group */
  g_object_unref (G_OBJECT (shortcuts_pane->action_group));

  (*G_OBJECT_CLASS (fmb_shortcuts_pane_parent_class)->finalize) (object);
}



static void
fmb_shortcuts_pane_get_property (GObject    *object,
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

    case PROP_SHOW_HIDDEN:
      g_value_set_boolean (value, fmb_side_pane_get_show_hidden (FMB_SIDE_PANE (object)));
      break;

    case PROP_UI_MANAGER:
      g_value_set_object (value, fmb_component_get_ui_manager (FMB_COMPONENT (object)));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static void
fmb_shortcuts_pane_set_property (GObject      *object,
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

    case PROP_SHOW_HIDDEN:
      fmb_side_pane_set_show_hidden (FMB_SIDE_PANE (object), g_value_get_boolean (value));
      break;

    case PROP_UI_MANAGER:
      fmb_component_set_ui_manager (FMB_COMPONENT (object), g_value_get_object (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static FmbFile*
fmb_shortcuts_pane_get_current_directory (FmbNavigator *navigator)
{
  return FMB_SHORTCUTS_PANE (navigator)->current_directory;
}



static gboolean
fmb_shortcuts_pane_set_current_directory_idle (gpointer data)
{
  FmbShortcutsPane *shortcuts_pane = FMB_SHORTCUTS_PANE (data);

  if (shortcuts_pane->current_directory != NULL)
    {
      fmb_shortcuts_view_select_by_file (FMB_SHORTCUTS_VIEW (shortcuts_pane->view),
                                            shortcuts_pane->current_directory);
    }

  /* unset id */
  shortcuts_pane->idle_select_directory = 0;

  return FALSE;
}



static void
fmb_shortcuts_pane_set_current_directory (FmbNavigator *navigator,
                                             FmbFile      *current_directory)
{
  FmbShortcutsPane *shortcuts_pane = FMB_SHORTCUTS_PANE (navigator);

  /* disconnect from the previously set current directory */
  if (G_LIKELY (shortcuts_pane->current_directory != NULL))
    g_object_unref (G_OBJECT (shortcuts_pane->current_directory));

  /* remove pending timeout */
  if (shortcuts_pane->idle_select_directory != 0)
    {
      g_source_remove (shortcuts_pane->idle_select_directory);
      shortcuts_pane->idle_select_directory = 0;
    }

  /* activate the new directory */
  shortcuts_pane->current_directory = current_directory;

  /* connect to the new directory */
  if (G_LIKELY (current_directory != NULL))
    {
      /* take a reference on the new directory */
      g_object_ref (G_OBJECT (current_directory));

      /* start idle to select item in sidepane (this to also make
       * the selection work when the bookmarks are loaded idle) */
      shortcuts_pane->idle_select_directory =
        g_idle_add_full (G_PRIORITY_LOW, fmb_shortcuts_pane_set_current_directory_idle,
                         shortcuts_pane, NULL);
    }

  /* notify listeners */
  g_object_notify (G_OBJECT (shortcuts_pane), "current-directory");
}



static GList*
fmb_shortcuts_pane_get_selected_files (FmbComponent *component)
{
  return FMB_SHORTCUTS_PANE (component)->selected_files;
}



static void
fmb_shortcuts_pane_set_selected_files (FmbComponent *component,
                                          GList           *selected_files)
{
  FmbShortcutsPane *shortcuts_pane = FMB_SHORTCUTS_PANE (component);
  GtkTreeModel        *model;
  GtkTreeModel        *child_model;
  GtkAction           *action;
  GList               *lp;
  gint                 n;

  /* disconnect from the previously selected files... */
  fmb_g_file_list_free (shortcuts_pane->selected_files);

  /* ...and take a copy of the newly selected files */
  shortcuts_pane->selected_files = fmb_g_file_list_copy (selected_files);

  /* check if the selection contains only folders */
  for (lp = selected_files, n = 0; lp != NULL; lp = lp->next, ++n)
    if (!fmb_file_is_directory (lp->data))
      break;

  /* change the visibility of the "shortcuts-add" action appropriately */
  action = gtk_action_group_get_action (shortcuts_pane->action_group, "sendto-shortcuts");
  if (lp == NULL && selected_files != NULL)
    {
      /* check if atleast one of the selected folders is not already present in the model */
      model = gtk_tree_view_get_model (GTK_TREE_VIEW (shortcuts_pane->view));
      if (G_LIKELY (model != NULL))
        {
          /* check all selected folders */
          child_model = gtk_tree_model_filter_get_model (GTK_TREE_MODEL_FILTER (model));
          for (lp = selected_files; lp != NULL; lp = lp->next)
            if (!fmb_shortcuts_model_has_bookmark (FMB_SHORTCUTS_MODEL (child_model), fmb_file_get_file (lp->data)))
              break;
        }

      /* display the action and change the label appropriately */
      g_object_set (G_OBJECT (action),
                    "label", ngettext ("Side Pane (Create Shortcut)", "Side Pane (Create Shortcuts)", n),
                    "sensitive", (lp != NULL),
                    "tooltip", ngettext ("Add the selected folder to the shortcuts side pane",
                                         "Add the selected folders to the shortcuts side pane", n),
                    "visible", TRUE,
                    NULL);
    }
  else
    {
      /* hide the action */
      gtk_action_set_visible (action, FALSE);
    }

  /* notify listeners */
  g_object_notify (G_OBJECT (shortcuts_pane), "selected-files");
}



static GtkUIManager*
fmb_shortcuts_pane_get_ui_manager (FmbComponent *component)
{
  return FMB_SHORTCUTS_PANE (component)->ui_manager;
}



static void
fmb_shortcuts_pane_set_ui_manager (FmbComponent *component,
                                      GtkUIManager    *ui_manager)
{
  FmbShortcutsPane *shortcuts_pane = FMB_SHORTCUTS_PANE (component);
  GError              *error = NULL;

  /* disconnect from the previous UI manager */
  if (G_UNLIKELY (shortcuts_pane->ui_manager != NULL))
    {
      /* drop our action group from the previous UI manager */
      gtk_ui_manager_remove_action_group (shortcuts_pane->ui_manager, shortcuts_pane->action_group);

      /* unmerge our ui controls from the previous UI manager */
      gtk_ui_manager_remove_ui (shortcuts_pane->ui_manager, shortcuts_pane->ui_merge_id);

      /* drop our reference on the previous UI manager */
      g_object_unref (G_OBJECT (shortcuts_pane->ui_manager));
    }

  /* activate the new UI manager */
  shortcuts_pane->ui_manager = ui_manager;

  /* connect to the new UI manager */
  if (G_LIKELY (ui_manager != NULL))
    {
      /* we keep a reference on the new manager */
      g_object_ref (G_OBJECT (ui_manager));

      /* add our action group to the new manager */
      gtk_ui_manager_insert_action_group (ui_manager, shortcuts_pane->action_group, -1);

      /* merge our UI control items with the new manager */
      shortcuts_pane->ui_merge_id = gtk_ui_manager_add_ui_from_string (ui_manager, fmb_shortcuts_pane_ui, fmb_shortcuts_pane_ui_length, &error);
      if (G_UNLIKELY (shortcuts_pane->ui_merge_id == 0))
        {
          g_error ("Failed to merge FmbShortcutsPane menus: %s", error->message);
          g_error_free (error);
        }
    }

  /* notify listeners */
  g_object_notify (G_OBJECT (shortcuts_pane), "ui-manager");
}



static void
fmb_shortcuts_pane_action_shortcuts_add (GtkAction           *action,
                                            FmbShortcutsPane *shortcuts_pane)
{
  GtkTreeModel *model;
  GtkTreeModel *child_model;
  GList        *lp;

  _fmb_return_if_fail (GTK_IS_ACTION (action));
  _fmb_return_if_fail (FMB_IS_SHORTCUTS_PANE (shortcuts_pane));

  /* determine the shortcuts model for the view */
  model = gtk_tree_view_get_model (GTK_TREE_VIEW (shortcuts_pane->view));
  if (G_LIKELY (model != NULL))
    {
      /* add all selected folders to the model */
      child_model = gtk_tree_model_filter_get_model (GTK_TREE_MODEL_FILTER (model));
      for (lp = shortcuts_pane->selected_files; lp != NULL; lp = lp->next)
        if (G_LIKELY (fmb_file_is_directory (lp->data)))
          {
            /* append the folder to the shortcuts model */
            fmb_shortcuts_model_add (FMB_SHORTCUTS_MODEL (child_model), NULL, lp->data);
          }

      /* update the user interface to reflect the new action state */
      lp = fmb_g_file_list_copy (shortcuts_pane->selected_files);
      fmb_component_set_selected_files (FMB_COMPONENT (shortcuts_pane), lp);
      fmb_g_file_list_free (lp);
    }
}

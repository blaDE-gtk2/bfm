/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2006 Benedikt Meurer <benny@xfce.org>
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

#include <fmb/fmb-tree-pane.h>
#include <fmb/fmb-tree-view.h>



/* Property identifiers */
enum
{
  PROP_0,
  PROP_CURRENT_DIRECTORY,
  PROP_SELECTED_FILES,
  PROP_SHOW_HIDDEN,
  PROP_UI_MANAGER,
};



static void          fmb_tree_pane_component_init        (FmbComponentIface *iface);
static void          fmb_tree_pane_navigator_init        (FmbNavigatorIface *iface);
static void          fmb_tree_pane_side_pane_init        (FmbSidePaneIface  *iface);
static void          fmb_tree_pane_dispose               (GObject              *object);
static void          fmb_tree_pane_get_property          (GObject              *object,
                                                             guint                 prop_id,
                                                             GValue               *value,
                                                             GParamSpec           *pspec);
static void          fmb_tree_pane_set_property          (GObject              *object,
                                                             guint                 prop_id,
                                                             const GValue         *value,
                                                             GParamSpec           *pspec);
static FmbFile   *fmb_tree_pane_get_current_directory (FmbNavigator      *navigator);
static void          fmb_tree_pane_set_current_directory (FmbNavigator      *navigator,
                                                             FmbFile           *current_directory);
static gboolean      fmb_tree_pane_get_show_hidden       (FmbSidePane       *side_pane);
static void          fmb_tree_pane_set_show_hidden       (FmbSidePane       *side_pane,
                                                             gboolean              show_hidden);



struct _FmbTreePaneClass
{
  GtkScrolledWindowClass __parent__;
};

struct _FmbTreePane
{
  GtkScrolledWindow __parent__;

  FmbFile *current_directory;
  GtkWidget  *view;
  gboolean    show_hidden;
};



G_DEFINE_TYPE_WITH_CODE (FmbTreePane, fmb_tree_pane, GTK_TYPE_SCROLLED_WINDOW,
    G_IMPLEMENT_INTERFACE (FMB_TYPE_NAVIGATOR, fmb_tree_pane_navigator_init)
    G_IMPLEMENT_INTERFACE (FMB_TYPE_COMPONENT, fmb_tree_pane_component_init)
    G_IMPLEMENT_INTERFACE (FMB_TYPE_SIDE_PANE, fmb_tree_pane_side_pane_init))



static void
fmb_tree_pane_class_init (FmbTreePaneClass *klass)
{
  GObjectClass *gobject_class;

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->dispose = fmb_tree_pane_dispose;
  gobject_class->get_property = fmb_tree_pane_get_property;
  gobject_class->set_property = fmb_tree_pane_set_property;

  /* override FmbNavigator's properties */
  g_object_class_override_property (gobject_class, PROP_CURRENT_DIRECTORY, "current-directory");

  /* override FmbComponent's properties */
  g_object_class_override_property (gobject_class, PROP_SELECTED_FILES, "selected-files");
  g_object_class_override_property (gobject_class, PROP_UI_MANAGER, "ui-manager");

  /* override FmbSidePane's properties */
  g_object_class_override_property (gobject_class, PROP_SHOW_HIDDEN, "show-hidden");
}



static void
fmb_tree_pane_component_init (FmbComponentIface *iface)
{
  iface->get_selected_files = (gpointer) blxo_noop_null;
  iface->set_selected_files = (gpointer) blxo_noop;
  iface->get_ui_manager = (gpointer) blxo_noop_null;
  iface->set_ui_manager = (gpointer) blxo_noop;
}



static void
fmb_tree_pane_navigator_init (FmbNavigatorIface *iface)
{
  iface->get_current_directory = fmb_tree_pane_get_current_directory;
  iface->set_current_directory = fmb_tree_pane_set_current_directory;
}



static void
fmb_tree_pane_side_pane_init (FmbSidePaneIface *iface)
{
  iface->get_show_hidden = fmb_tree_pane_get_show_hidden;
  iface->set_show_hidden = fmb_tree_pane_set_show_hidden;
}



static void
fmb_tree_pane_init (FmbTreePane *tree_pane)
{
  /* configure the GtkScrolledWindow */
  gtk_scrolled_window_set_hadjustment (GTK_SCROLLED_WINDOW (tree_pane), NULL);
  gtk_scrolled_window_set_vadjustment (GTK_SCROLLED_WINDOW (tree_pane), NULL);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (tree_pane), GTK_SHADOW_IN);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (tree_pane), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  /* allocate the tree view */
  tree_pane->view = fmb_tree_view_new ();
  blxo_binding_new (G_OBJECT (tree_pane), "show-hidden", G_OBJECT (tree_pane->view), "show-hidden");
  blxo_binding_new (G_OBJECT (tree_pane), "current-directory", G_OBJECT (tree_pane->view), "current-directory");
  g_signal_connect_swapped (G_OBJECT (tree_pane->view), "change-directory", G_CALLBACK (fmb_navigator_change_directory), tree_pane);
  g_signal_connect_swapped (G_OBJECT (tree_pane->view), "open-new-tab", G_CALLBACK (fmb_navigator_open_new_tab), tree_pane);
  gtk_container_add (GTK_CONTAINER (tree_pane), tree_pane->view);
  gtk_widget_show (tree_pane->view);
}



static void
fmb_tree_pane_dispose (GObject *object)
{
  FmbTreePane *tree_pane = FMB_TREE_PANE (object);

  fmb_navigator_set_current_directory (FMB_NAVIGATOR (tree_pane), NULL);
  fmb_component_set_selected_files (FMB_COMPONENT (tree_pane), NULL);
  fmb_component_set_ui_manager (FMB_COMPONENT (tree_pane), NULL);

  (*G_OBJECT_CLASS (fmb_tree_pane_parent_class)->dispose) (object);
}



static void
fmb_tree_pane_get_property (GObject    *object,
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
fmb_tree_pane_set_property (GObject      *object,
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
fmb_tree_pane_get_current_directory (FmbNavigator *navigator)
{
  return FMB_TREE_PANE (navigator)->current_directory;
}



static void
fmb_tree_pane_set_current_directory (FmbNavigator *navigator,
                                        FmbFile      *current_directory)
{
  FmbTreePane *tree_pane = FMB_TREE_PANE (navigator);

  /* disconnect from the previously set current directory */
  if (G_LIKELY (tree_pane->current_directory != NULL))
    g_object_unref (G_OBJECT (tree_pane->current_directory));

  /* activate the new directory */
  tree_pane->current_directory = current_directory;

  /* connect to the new directory */
  if (G_LIKELY (current_directory != NULL))
    g_object_ref (G_OBJECT (current_directory));

  /* notify listeners */
  g_object_notify (G_OBJECT (tree_pane), "current-directory");
}



static gboolean
fmb_tree_pane_get_show_hidden (FmbSidePane *side_pane)
{
  return FMB_TREE_PANE (side_pane)->show_hidden;
}



static void
fmb_tree_pane_set_show_hidden (FmbSidePane *side_pane,
                                  gboolean        show_hidden)
{
  FmbTreePane *tree_pane = FMB_TREE_PANE (side_pane);

  show_hidden = !!show_hidden;

  /* check if we have a new setting */
  if (G_UNLIKELY (tree_pane->show_hidden != show_hidden))
    {
      /* remember the new setting */
      tree_pane->show_hidden = show_hidden;

      /* notify listeners */
      g_object_notify (G_OBJECT (tree_pane), "show-hidden");
    }
}



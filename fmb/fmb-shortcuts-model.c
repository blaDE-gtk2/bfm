/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2005-2006 Benedikt Meurer <benny@xfce.org>
 * Copyright (c) 2009 Jannis Pohlmann <jannis@xfce.org>
 * Copyright (c) 2012      Nick Schermer <nick@xfce.org>
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

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif

#include <glib.h>
#include <glib/gstdio.h>

#include <fmb/fmb-file.h>
#include <fmb/fmb-shortcuts-model.h>
#include <fmb/fmb-device-monitor.h>
#include <fmb/fmb-preferences.h>
#include <fmb/fmb-util.h>
#include <fmb/fmb-private.h>

#define SPINNER_CYCLE_DURATION 1000
#define SPINNER_NUM_STEPS      12



#define FMB_SHORTCUT(obj) ((FmbShortcut *) (obj))



typedef struct _FmbShortcut FmbShortcut;



enum
{
  PROP_0,
  PROP_HIDDEN_BOOKMARKS,
  PROP_FILE_SIZE_BINARY
};



static void               fmb_shortcuts_model_tree_model_init    (GtkTreeModelIface         *iface);
static void               fmb_shortcuts_model_drag_source_init   (GtkTreeDragSourceIface    *iface);
static void               fmb_shortcuts_model_get_property       (GObject                   *object,
                                                                     guint                      prop_id,
                                                                     GValue                    *value,
                                                                     GParamSpec                *pspec);
static void               fmb_shortcuts_model_set_property       (GObject                   *object,
                                                                     guint                      prop_id,
                                                                     const GValue              *value,
                                                                     GParamSpec                *pspec);
static void               fmb_shortcuts_model_finalize           (GObject                   *object);
static GtkTreeModelFlags  fmb_shortcuts_model_get_flags          (GtkTreeModel              *tree_model);
static gint               fmb_shortcuts_model_get_n_columns      (GtkTreeModel              *tree_model);
static GType              fmb_shortcuts_model_get_column_type    (GtkTreeModel              *tree_model,
                                                                     gint                       idx);
static gboolean           fmb_shortcuts_model_get_iter           (GtkTreeModel              *tree_model,
                                                                     GtkTreeIter               *iter,
                                                                     GtkTreePath               *path);
static GtkTreePath       *fmb_shortcuts_model_get_path           (GtkTreeModel              *tree_model,
                                                                     GtkTreeIter               *iter);
static void               fmb_shortcuts_model_get_value          (GtkTreeModel              *tree_model,
                                                                     GtkTreeIter               *iter,
                                                                     gint                       column,
                                                                     GValue                    *value);
static gboolean           fmb_shortcuts_model_iter_next          (GtkTreeModel              *tree_model,
                                                                     GtkTreeIter               *iter);
static gboolean           fmb_shortcuts_model_iter_children      (GtkTreeModel              *tree_model,
                                                                     GtkTreeIter               *iter,
                                                                     GtkTreeIter               *parent);
static gboolean           fmb_shortcuts_model_iter_has_child     (GtkTreeModel              *tree_model,
                                                                     GtkTreeIter               *iter);
static gint               fmb_shortcuts_model_iter_n_children    (GtkTreeModel              *tree_model,
                                                                     GtkTreeIter               *iter);
static gboolean           fmb_shortcuts_model_iter_nth_child     (GtkTreeModel              *tree_model,
                                                                     GtkTreeIter               *iter,
                                                                     GtkTreeIter               *parent,
                                                                     gint                       n);
static gboolean           fmb_shortcuts_model_iter_parent        (GtkTreeModel              *tree_model,
                                                                     GtkTreeIter               *iter,
                                                                     GtkTreeIter               *child);
static gboolean           fmb_shortcuts_model_row_draggable      (GtkTreeDragSource         *source,
                                                                     GtkTreePath               *path);
static gboolean           fmb_shortcuts_model_drag_data_get      (GtkTreeDragSource         *source,
                                                                     GtkTreePath               *path,
                                                                     GtkSelectionData          *selection_data);
static gboolean           fmb_shortcuts_model_drag_data_delete   (GtkTreeDragSource         *source,
                                                                     GtkTreePath               *path);
static void               fmb_shortcuts_model_header_visibility  (FmbShortcutsModel      *model);
static void               fmb_shortcuts_model_shortcut_devices   (FmbShortcutsModel      *model);
static void               fmb_shortcuts_model_shortcut_places    (FmbShortcutsModel      *model);
static void               fmb_shortcuts_model_shortcut_network   (FmbShortcutsModel      *model);
static gboolean           fmb_shortcuts_model_get_hidden         (FmbShortcutsModel      *model,
                                                                     FmbShortcut            *shortcut);
static void               fmb_shortcuts_model_add_shortcut       (FmbShortcutsModel      *model,
                                                                     FmbShortcut            *shortcut);
static void               fmb_shortcuts_model_remove_shortcut    (FmbShortcutsModel      *model,
                                                                     FmbShortcut            *shortcut);
static gboolean           fmb_shortcuts_model_load               (gpointer                   data);
static void               fmb_shortcuts_model_save               (FmbShortcutsModel      *model);
static void               fmb_shortcuts_model_monitor            (GFileMonitor              *monitor,
                                                                     GFile                     *file,
                                                                     GFile                     *other_file,
                                                                     GFileMonitorEvent          event_type,
                                                                     gpointer                   user_data);
static void               fmb_shortcuts_model_file_changed       (FmbFile                *file,
                                                                     FmbShortcutsModel      *model);
static void               fmb_shortcuts_model_file_destroy       (FmbFile                *file,
                                                                     FmbShortcutsModel      *model);
static void               fmb_shortcuts_model_device_added       (FmbDeviceMonitor       *device_monitor,
                                                                     FmbDevice              *device,
                                                                     FmbShortcutsModel      *model);
static void               fmb_shortcuts_model_device_removed     (FmbDeviceMonitor       *device_monitor,
                                                                     FmbDevice              *device,
                                                                     FmbShortcutsModel      *model);
static void               fmb_shortcuts_model_device_changed     (FmbDeviceMonitor       *device_monitor,
                                                                     FmbDevice              *device,
                                                                     FmbShortcutsModel      *model);

static void               fmb_shortcut_free                      (FmbShortcut            *shortcut,
                                                                     FmbShortcutsModel      *model);



struct _FmbShortcutsModelClass
{
  GObjectClass __parent__;
};

struct _FmbShortcutsModel
{
  GObject         __parent__;

  /* the model stamp is only used when debugging is
   * enabled, to make sure we don't accept iterators
   * generated by another model.
   */
#ifndef NDEBUG
  gint                  stamp;
#endif

  GList                *shortcuts;

  FmbPreferences    *preferences;
  gchar               **hidden_bookmarks;
  gboolean              file_size_binary;

  FmbDeviceMonitor  *device_monitor;

  gint64                bookmarks_time;
  GFile                *bookmarks_file;
  GFileMonitor         *bookmarks_monitor;
  guint                 bookmarks_idle_id;

  guint                 busy_timeout_id;
};

struct _FmbShortcut
{
  FmbShortcutGroup  group;

  gchar               *name;
  GIcon               *gicon;
  gchar               *tooltip;
  gint                 sort_id;

  guint                busy : 1;
  guint                busy_pulse;

  GFile               *location;
  FmbFile          *file;
  FmbDevice        *device;

  guint                hidden : 1;
};



G_DEFINE_TYPE_WITH_CODE (FmbShortcutsModel, fmb_shortcuts_model, G_TYPE_OBJECT,
    G_IMPLEMENT_INTERFACE (GTK_TYPE_TREE_MODEL, fmb_shortcuts_model_tree_model_init)
    G_IMPLEMENT_INTERFACE (GTK_TYPE_TREE_DRAG_SOURCE, fmb_shortcuts_model_drag_source_init))



static void
fmb_shortcuts_model_class_init (FmbShortcutsModelClass *klass)
{
  GObjectClass *gobject_class;

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = fmb_shortcuts_model_finalize;
  gobject_class->get_property = fmb_shortcuts_model_get_property;
  gobject_class->set_property = fmb_shortcuts_model_set_property;

  g_object_class_install_property (gobject_class,
                                   PROP_HIDDEN_BOOKMARKS,
                                   g_param_spec_boxed ("hidden-bookmarks",
                                                       NULL,
                                                       NULL,
                                                       G_TYPE_STRV,
                                                       BLXO_PARAM_READWRITE));

  /**
   * FmbPropertiesDialog:file_size_binary:
   *
   * Whether the file size should be shown in binary or decimal.
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_FILE_SIZE_BINARY,
                                   g_param_spec_boolean ("file-size-binary",
                                                         "FileSizeBinary",
                                                         NULL,
                                                         FALSE,
                                                         BLXO_PARAM_READWRITE));
}



static void
fmb_shortcuts_model_tree_model_init (GtkTreeModelIface *iface)
{
  iface->get_flags = fmb_shortcuts_model_get_flags;
  iface->get_n_columns = fmb_shortcuts_model_get_n_columns;
  iface->get_column_type = fmb_shortcuts_model_get_column_type;
  iface->get_iter = fmb_shortcuts_model_get_iter;
  iface->get_path = fmb_shortcuts_model_get_path;
  iface->get_value = fmb_shortcuts_model_get_value;
  iface->iter_next = fmb_shortcuts_model_iter_next;
  iface->iter_children = fmb_shortcuts_model_iter_children;
  iface->iter_has_child = fmb_shortcuts_model_iter_has_child;
  iface->iter_n_children = fmb_shortcuts_model_iter_n_children;
  iface->iter_nth_child = fmb_shortcuts_model_iter_nth_child;
  iface->iter_parent = fmb_shortcuts_model_iter_parent;
}



static void
fmb_shortcuts_model_drag_source_init (GtkTreeDragSourceIface *iface)
{
  iface->row_draggable = fmb_shortcuts_model_row_draggable;
  iface->drag_data_get = fmb_shortcuts_model_drag_data_get;
  iface->drag_data_delete = fmb_shortcuts_model_drag_data_delete;
}



static void
fmb_shortcuts_model_init (FmbShortcutsModel *model)
{
#ifndef NDEBUG
  model->stamp = g_random_int ();
#endif

  /* hidden bookmarks */
  model->preferences = fmb_preferences_get ();
  blxo_binding_new (G_OBJECT (model->preferences), "hidden-bookmarks",
                   G_OBJECT (model), "hidden-bookmarks");

  /* binary file size */
  blxo_binding_new (G_OBJECT (model->preferences), "misc-file-size-binary",
                   G_OBJECT (model), "file-size-binary");

  /* load volumes */
  fmb_shortcuts_model_shortcut_devices (model);

  /* add network */
  fmb_shortcuts_model_shortcut_network (model);

  /* add bookmarks */
  fmb_shortcuts_model_shortcut_places (model);
}



static void
fmb_shortcuts_model_finalize (GObject *object)
{
  FmbShortcutsModel *model = FMB_SHORTCUTS_MODEL (object);

  _fmb_return_if_fail (FMB_IS_SHORTCUTS_MODEL (model));

  /* stop the busy timeout */
  if (model->busy_timeout_id != 0)
    g_source_remove (model->busy_timeout_id);

  /* stop bookmark load idle */
  if (model->bookmarks_idle_id != 0)
    g_source_remove (model->bookmarks_idle_id);

  /* free all shortcuts */
  g_list_foreach (model->shortcuts, (GFunc) fmb_shortcut_free, model);
  g_list_free (model->shortcuts);

  /* disconnect from the preferences */
  g_object_unref (model->preferences);

  /* free hidden list */
  g_strfreev (model->hidden_bookmarks);

  /* detach from the file monitor */
  if (model->bookmarks_monitor != NULL)
    {
      g_file_monitor_cancel (model->bookmarks_monitor);
      g_object_unref (model->bookmarks_monitor);
    }

  if (model->bookmarks_file != NULL)
    g_object_unref (model->bookmarks_file);

  /* unlink from the device monitor */
  g_signal_handlers_disconnect_matched (model->device_monitor, G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, model);
  g_object_unref (model->device_monitor);

  (*G_OBJECT_CLASS (fmb_shortcuts_model_parent_class)->finalize) (object);
}



static void
fmb_shortcuts_model_get_property (GObject    *object,
                                     guint       prop_id,
                                     GValue     *value,
                                     GParamSpec *pspec)
{
  FmbShortcutsModel *model = FMB_SHORTCUTS_MODEL (object);

  switch (prop_id)
    {
    case PROP_HIDDEN_BOOKMARKS:
      g_value_set_boxed (value, model->hidden_bookmarks);
      break;

    case PROP_FILE_SIZE_BINARY:
      g_value_set_boolean (value, model->file_size_binary);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static void
fmb_shortcuts_model_set_property (GObject      *object,
                                     guint         prop_id,
                                     const GValue *value,
                                     GParamSpec   *pspec)
{
  FmbShortcutsModel *model = FMB_SHORTCUTS_MODEL (object);
  GList                *lp;
  FmbShortcut       *shortcut;
  gboolean              hidden;
  GtkTreeIter           iter;
  GtkTreePath          *path;
  guint                 idx;

  switch (prop_id)
    {
    case PROP_HIDDEN_BOOKMARKS:
      g_strfreev (model->hidden_bookmarks);
      model->hidden_bookmarks = g_value_dup_boxed (value);

      /* update shortcuts */
      for (lp = model->shortcuts, idx = 0; lp != NULL; lp = lp->next, idx++)
        {
          shortcut = lp->data;

          /* skip devices and headers*/
          if (shortcut->device != NULL
              || (shortcut->file == NULL && shortcut->location == NULL))
            continue;

          /* update state if required */
          hidden = fmb_shortcuts_model_get_hidden (model, shortcut);
          if (shortcut->hidden != hidden)
            {
              shortcut->hidden = hidden;

              GTK_TREE_ITER_INIT (iter, model->stamp, lp);

              path = gtk_tree_path_new_from_indices (idx, -1);
              gtk_tree_model_row_changed (GTK_TREE_MODEL (model), path, &iter);
              gtk_tree_path_free (path);
            }
        }

      /* update header visibility */
      fmb_shortcuts_model_header_visibility (model);
      break;

    case PROP_FILE_SIZE_BINARY:
      model->file_size_binary = g_value_get_boolean (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static GtkTreeModelFlags
fmb_shortcuts_model_get_flags (GtkTreeModel *tree_model)
{
  return GTK_TREE_MODEL_ITERS_PERSIST | GTK_TREE_MODEL_LIST_ONLY;
}



static gint
fmb_shortcuts_model_get_n_columns (GtkTreeModel *tree_model)
{
  return FMB_SHORTCUTS_MODEL_N_COLUMNS;
}



static GType
fmb_shortcuts_model_get_column_type (GtkTreeModel *tree_model,
                                        gint          idx)
{
  switch (idx)
    {
    case FMB_SHORTCUTS_MODEL_COLUMN_IS_HEADER:
      return G_TYPE_BOOLEAN;

    case FMB_SHORTCUTS_MODEL_COLUMN_IS_ITEM:
      return G_TYPE_BOOLEAN;

    case FMB_SHORTCUTS_MODEL_COLUMN_VISIBLE:
      return G_TYPE_BOOLEAN;

    case FMB_SHORTCUTS_MODEL_COLUMN_NAME:
      return G_TYPE_STRING;

    case FMB_SHORTCUTS_MODEL_COLUMN_TOOLTIP:
      return G_TYPE_STRING;

    case FMB_SHORTCUTS_MODEL_COLUMN_FILE:
      return FMB_TYPE_FILE;

    case FMB_SHORTCUTS_MODEL_COLUMN_LOCATION:
      return G_TYPE_FILE;

    case FMB_SHORTCUTS_MODEL_COLUMN_GICON:
      return G_TYPE_ICON;

    case FMB_SHORTCUTS_MODEL_COLUMN_DEVICE:
      return FMB_TYPE_DEVICE;

    case FMB_SHORTCUTS_MODEL_COLUMN_MUTABLE:
      return G_TYPE_BOOLEAN;

    case FMB_SHORTCUTS_MODEL_COLUMN_CAN_EJECT:
      return G_TYPE_BOOLEAN;

    case FMB_SHORTCUTS_MODEL_COLUMN_GROUP:
      return G_TYPE_UINT;

    case FMB_SHORTCUTS_MODEL_COLUMN_BUSY:
      return G_TYPE_BOOLEAN;

    case FMB_SHORTCUTS_MODEL_COLUMN_BUSY_PULSE:
      return G_TYPE_UINT;
    }

  _fmb_assert_not_reached ();
  return G_TYPE_INVALID;
}



static gboolean
fmb_shortcuts_model_get_iter (GtkTreeModel *tree_model,
                                 GtkTreeIter  *iter,
                                 GtkTreePath  *path)
{
  FmbShortcutsModel *model = FMB_SHORTCUTS_MODEL (tree_model);
  GList                 *lp;

  _fmb_return_val_if_fail (FMB_IS_SHORTCUTS_MODEL (model), FALSE);
  _fmb_return_val_if_fail (gtk_tree_path_get_depth (path) > 0, FALSE);

  /* determine the list item for the path */
  lp = g_list_nth (model->shortcuts, gtk_tree_path_get_indices (path)[0]);
  if (G_LIKELY (lp != NULL))
    {
      GTK_TREE_ITER_INIT (*iter, model->stamp, lp);
      return TRUE;
    }

  return FALSE;
}



static GtkTreePath*
fmb_shortcuts_model_get_path (GtkTreeModel *tree_model,
                                 GtkTreeIter  *iter)
{
  FmbShortcutsModel *model = FMB_SHORTCUTS_MODEL (tree_model);
  gint                  idx;

  _fmb_return_val_if_fail (FMB_IS_SHORTCUTS_MODEL (model), NULL);
  _fmb_return_val_if_fail (iter->stamp == model->stamp, NULL);

  /* lookup the list item in the shortcuts list */
  idx = g_list_position (model->shortcuts, iter->user_data);
  if (G_LIKELY (idx >= 0))
    return gtk_tree_path_new_from_indices (idx, -1);

  return NULL;
}



static void
fmb_shortcuts_model_get_value (GtkTreeModel *tree_model,
                                  GtkTreeIter  *iter,
                                  gint          column,
                                  GValue       *value)
{
  FmbShortcut *shortcut;
  gboolean        can_eject;
  GFile          *file;
  gchar          *disk_usage;
  guint32         trash_items;
  gchar          *trash_string;
  gchar          *parse_name;
  gboolean        file_size_binary;

  _fmb_return_if_fail (iter->stamp == FMB_SHORTCUTS_MODEL (tree_model)->stamp);
  _fmb_return_if_fail (FMB_IS_SHORTCUTS_MODEL (tree_model));

  /* determine the shortcut for the list item */
  shortcut = FMB_SHORTCUT (((GList *) iter->user_data)->data);
  if (shortcut == NULL)
    return;

  switch (column)
    {
    case FMB_SHORTCUTS_MODEL_COLUMN_IS_HEADER:
      g_value_init (value, G_TYPE_BOOLEAN);
      g_value_set_boolean (value, (shortcut->group & FMB_SHORTCUT_GROUP_HEADER) != 0);
      break;

    case FMB_SHORTCUTS_MODEL_COLUMN_IS_ITEM:
      g_value_init (value, G_TYPE_BOOLEAN);
      g_value_set_boolean (value, (shortcut->group & FMB_SHORTCUT_GROUP_HEADER) == 0);
      break;

    case FMB_SHORTCUTS_MODEL_COLUMN_VISIBLE:
      g_value_init (value, G_TYPE_BOOLEAN);
      g_value_set_boolean (value, !shortcut->hidden);
      break;

    case FMB_SHORTCUTS_MODEL_COLUMN_NAME:
      g_value_init (value, G_TYPE_STRING);
      if (G_UNLIKELY (shortcut->device != NULL))
        g_value_take_string (value, fmb_device_get_name (shortcut->device));
      else if (shortcut->name != NULL)
        g_value_set_static_string (value, shortcut->name);
      else if (shortcut->file != NULL)
        g_value_set_static_string (value, fmb_file_get_display_name (shortcut->file));
      else if (shortcut->location != NULL)
        g_value_take_string (value, fmb_g_file_get_display_name_remote (shortcut->location));
      else
        g_value_set_static_string (value, "");
      break;

    case FMB_SHORTCUTS_MODEL_COLUMN_TOOLTIP:
      g_value_init (value, G_TYPE_STRING);
      if ((shortcut->group & FMB_SHORTCUT_GROUP_DEVICES) != 0)
        {
          if (shortcut->device != NULL)
            file = fmb_device_get_root (shortcut->device);
          else if (shortcut->file != NULL)
            file = g_object_ref (fmb_file_get_file (shortcut->file));
          else
            file = NULL;

          if (file != NULL)
            {
              file_size_binary = FMB_SHORTCUTS_MODEL (tree_model)->file_size_binary;
              disk_usage = fmb_g_file_get_free_space_string (file, file_size_binary);
              g_object_unref (file);
              g_value_take_string (value, disk_usage);
            }
          break;
        }
      else if ((shortcut->group & FMB_SHORTCUT_GROUP_PLACES_TRASH) != 0)
        {
          trash_items = fmb_file_get_item_count (shortcut->file);
          if (trash_items == 0)
            {
              g_value_set_static_string (value, _("Trash is empty"));
            }
          else
            {
              trash_string = g_strdup_printf (ngettext ("Trash contains %d file",
                                                        "Trash contains %d files",
                                                        trash_items), trash_items);
              g_value_take_string (value, trash_string);
            }
          break;
        }
      else if (shortcut->tooltip == NULL)
        {
          if (shortcut->file != NULL)
            file = fmb_file_get_file (shortcut->file);
          else if (shortcut->location != NULL)
            file = shortcut->location;
          else
            file = NULL;

          if (G_LIKELY (file != NULL))
            {
              parse_name = g_file_get_parse_name (file);
              shortcut->tooltip = g_markup_escape_text (parse_name, -1);
              g_free (parse_name);
            }
        }

      g_value_set_static_string (value, shortcut->tooltip);
      break;

    case FMB_SHORTCUTS_MODEL_COLUMN_FILE:
      g_value_init (value, FMB_TYPE_FILE);
      g_value_set_object (value, shortcut->file);
      break;

    case FMB_SHORTCUTS_MODEL_COLUMN_GICON:
      g_value_init (value, G_TYPE_ICON);
      g_value_set_object (value, shortcut->gicon);
      break;

    case FMB_SHORTCUTS_MODEL_COLUMN_DEVICE:
      g_value_init (value, FMB_TYPE_DEVICE);
      g_value_set_object (value, shortcut->device);
      break;

    case FMB_SHORTCUTS_MODEL_COLUMN_LOCATION:
      g_value_init (value, G_TYPE_FILE);
      if (shortcut->location != NULL)
        g_value_set_object (value, shortcut->location);
      else if (shortcut->file != NULL)
        g_value_set_object (value, fmb_file_get_file (shortcut->file));
      break;

    case FMB_SHORTCUTS_MODEL_COLUMN_MUTABLE:
      g_value_init (value, G_TYPE_BOOLEAN);
      g_value_set_boolean (value,
                           shortcut->group == FMB_SHORTCUT_GROUP_PLACES_BOOKMARKS);
      break;

    case FMB_SHORTCUTS_MODEL_COLUMN_CAN_EJECT:
      if (shortcut->device != NULL)
        can_eject = fmb_device_can_eject (shortcut->device);
      else
        can_eject = FALSE;

      g_value_init (value, G_TYPE_BOOLEAN);
      g_value_set_boolean (value, can_eject);
      break;

    case FMB_SHORTCUTS_MODEL_COLUMN_GROUP:
      g_value_init (value, G_TYPE_UINT);
      g_value_set_uint (value, shortcut->group);
      break;

    case FMB_SHORTCUTS_MODEL_COLUMN_BUSY:
      g_value_init (value, G_TYPE_BOOLEAN);
      g_value_set_boolean (value, shortcut->busy);
      break;

    case FMB_SHORTCUTS_MODEL_COLUMN_BUSY_PULSE:
      g_value_init (value, G_TYPE_UINT);
      g_value_set_uint (value, shortcut->busy_pulse);
      break;

    default:
      _fmb_assert_not_reached ();
    }
}



static gboolean
fmb_shortcuts_model_iter_next (GtkTreeModel *tree_model,
                                  GtkTreeIter  *iter)
{
  _fmb_return_val_if_fail (FMB_IS_SHORTCUTS_MODEL (tree_model), FALSE);
  _fmb_return_val_if_fail (iter->stamp == FMB_SHORTCUTS_MODEL (tree_model)->stamp, FALSE);

  iter->user_data = g_list_next (iter->user_data);
  return (iter->user_data != NULL);
}



static gboolean
fmb_shortcuts_model_iter_children (GtkTreeModel *tree_model,
                                      GtkTreeIter  *iter,
                                      GtkTreeIter  *parent)
{
  FmbShortcutsModel *model = FMB_SHORTCUTS_MODEL (tree_model);

  _fmb_return_val_if_fail (FMB_IS_SHORTCUTS_MODEL (model), FALSE);

  if (G_LIKELY (parent == NULL && model->shortcuts != NULL))
    {
      GTK_TREE_ITER_INIT (*iter, model->stamp, model->shortcuts);
      return TRUE;
    }

  return FALSE;
}



static gboolean
fmb_shortcuts_model_iter_has_child (GtkTreeModel *tree_model,
                                       GtkTreeIter  *iter)
{
  return FALSE;
}



static gint
fmb_shortcuts_model_iter_n_children (GtkTreeModel *tree_model,
                                        GtkTreeIter  *iter)
{
  FmbShortcutsModel *model = FMB_SHORTCUTS_MODEL (tree_model);

  _fmb_return_val_if_fail (FMB_IS_SHORTCUTS_MODEL (model), 0);

  return (iter == NULL) ? g_list_length (model->shortcuts) : 0;
}



static gboolean
fmb_shortcuts_model_iter_nth_child (GtkTreeModel *tree_model,
                                       GtkTreeIter  *iter,
                                       GtkTreeIter  *parent,
                                       gint          n)
{
  FmbShortcutsModel *model = FMB_SHORTCUTS_MODEL (tree_model);

  _fmb_return_val_if_fail (FMB_IS_SHORTCUTS_MODEL (model), FALSE);

  if (G_LIKELY (parent == NULL))
    {
      GTK_TREE_ITER_INIT (*iter, model->stamp, g_list_nth (model->shortcuts, n));
      return (iter->user_data != NULL);
    }

  return FALSE;
}



static gboolean
fmb_shortcuts_model_iter_parent (GtkTreeModel *tree_model,
                                    GtkTreeIter  *iter,
                                    GtkTreeIter  *child)
{
  return FALSE;
}



static gboolean
fmb_shortcuts_model_row_draggable (GtkTreeDragSource *source,
                                      GtkTreePath       *path)
{
  FmbShortcutsModel *model = FMB_SHORTCUTS_MODEL (source);
  FmbShortcut       *shortcut;

  _fmb_return_val_if_fail (FMB_IS_SHORTCUTS_MODEL (model), FALSE);
  _fmb_return_val_if_fail (gtk_tree_path_get_depth (path) > 0, FALSE);

  /* lookup the FmbShortcut for the path */
  shortcut = g_list_nth_data (model->shortcuts, gtk_tree_path_get_indices (path)[0]);

  /* special shortcuts cannot be reordered */
  return (shortcut != NULL && shortcut->group == FMB_SHORTCUT_GROUP_PLACES_BOOKMARKS);
}



static gboolean
fmb_shortcuts_model_drag_data_get (GtkTreeDragSource *source,
                                      GtkTreePath       *path,
                                      GtkSelectionData  *selection_data)
{
  /* we simply return FALSE here, as the drag handling is done in
   * the FmbShortcutsView class.
   */
  return FALSE;
}



static gboolean
fmb_shortcuts_model_drag_data_delete (GtkTreeDragSource *source,
                                         GtkTreePath       *path)
{
  /* we simply return FALSE here, as this function can only be
   * called if the user is re-arranging shortcuts within the
   * model, which will be handle by the exchange method.
   */
  return FALSE;
}



static void
fmb_shortcuts_model_header_visibility (FmbShortcutsModel *model)
{
  GList          *lp;
  FmbShortcut *shortcut;
  FmbShortcut *header;
  guint           n_children = 0;
  GList          *lp_header = NULL;
  gint            idx_header = 0;
  gint            i;
  GtkTreePath    *path;
  GtkTreeIter     iter;

  _fmb_return_if_fail (FMB_IS_SHORTCUTS_MODEL (model));

  for (lp = model->shortcuts, i = 0; lp != NULL; lp = lp->next, i++)
    {
      shortcut = lp->data;

      if ((shortcut->group & FMB_SHORTCUT_GROUP_HEADER) != 0)
        {
          if (lp_header != NULL)
            {
              update_header:

              header = lp_header->data;
              if (header->hidden != (n_children == 0))
                {
                  /* set new visibility */
                  header->hidden = (n_children == 0);

                  /* notify view */
                  path = gtk_tree_path_new_from_indices (idx_header, -1);
                  GTK_TREE_ITER_INIT (iter, model->stamp, lp_header);
                  gtk_tree_model_row_changed (GTK_TREE_MODEL (model), path, &iter);
                  gtk_tree_path_free (path);
                }

              if (lp == NULL)
                return;
            }

          /* reset for new */
          idx_header = i;
          lp_header = lp;
          n_children = 0;
        }
      else if (!shortcut->hidden)
        {
          n_children++;
        }
    }

  _fmb_assert (lp == NULL);
  if (lp_header != NULL)
    goto update_header;
}



static void
fmb_shortcuts_model_shortcut_devices (FmbShortcutsModel *model)
{
  FmbShortcut *shortcut;
  GList          *devices;
  GList          *lp;

  /* add the devices heading */
  shortcut = g_slice_new0 (FmbShortcut);
  shortcut->group = FMB_SHORTCUT_GROUP_DEVICES_HEADER;
  shortcut->name = g_strdup (_("DEVICES"));
  fmb_shortcuts_model_add_shortcut (model, shortcut);

  /* the filesystem entry */
  shortcut = g_slice_new0 (FmbShortcut);
  shortcut->group = FMB_SHORTCUT_GROUP_DEVICES_FILESYSTEM;
  shortcut->name = g_strdup (_("File System"));
  shortcut->file = fmb_file_get_for_uri ("file:///", NULL);
  shortcut->gicon = g_themed_icon_new ("drive-harddisk");
  shortcut->hidden = fmb_shortcuts_model_get_hidden (model, shortcut);
  fmb_shortcuts_model_add_shortcut (model, shortcut);

  /* connect to the device monitor */
  model->device_monitor = fmb_device_monitor_get ();

  /* get a list of all devices available */
  devices = fmb_device_monitor_get_devices (model->device_monitor);
  for (lp = devices; lp != NULL; lp = lp->next)
    {
      fmb_shortcuts_model_device_added (NULL, lp->data, model);
      g_object_unref (G_OBJECT (lp->data));
    }
  g_list_free (devices);

  /* monitor for changes */
  g_signal_connect (model->device_monitor, "device-added", G_CALLBACK (fmb_shortcuts_model_device_added), model);
  g_signal_connect (model->device_monitor, "device-removed", G_CALLBACK (fmb_shortcuts_model_device_removed), model);
  g_signal_connect (model->device_monitor, "device-changed", G_CALLBACK (fmb_shortcuts_model_device_changed), model);

  fmb_shortcuts_model_header_visibility (model);
}



static void
fmb_shortcuts_model_shortcut_network (FmbShortcutsModel *model)
{
  FmbShortcut *shortcut;

  /* add the network heading */
  shortcut = g_slice_new0 (FmbShortcut);
  shortcut->group = FMB_SHORTCUT_GROUP_NETWORK_HEADER;
  shortcut->name = g_strdup (_("NETWORK"));
  fmb_shortcuts_model_add_shortcut (model, shortcut);

  /* the browse network entry */
  shortcut = g_slice_new0 (FmbShortcut);
  shortcut->group = FMB_SHORTCUT_GROUP_NETWORK_DEFAULT;
  shortcut->name = g_strdup (_("Browse Network"));
  shortcut->location = g_file_new_for_uri ("network://");
  shortcut->gicon = g_themed_icon_new ("network-workgroup");
  shortcut->hidden = fmb_shortcuts_model_get_hidden (model, shortcut);
  fmb_shortcuts_model_add_shortcut (model, shortcut);
}



static void
fmb_shortcuts_model_shortcut_places (FmbShortcutsModel *model)
{
  FmbShortcut *shortcut;
  GFile          *home;
  GFile          *desktop;
  GFile          *trash;
  FmbFile     *file;

  /* add the places heading */
  shortcut = g_slice_new0 (FmbShortcut);
  shortcut->group = FMB_SHORTCUT_GROUP_PLACES_HEADER;
  shortcut->name = g_strdup (_("PLACES"));
  fmb_shortcuts_model_add_shortcut (model, shortcut);

  /* get home path */
  home = fmb_g_file_new_for_home ();

  /* add home entry */
  file = fmb_file_get (home, NULL);
  if (file != NULL)
    {
      shortcut = g_slice_new0 (FmbShortcut);
      shortcut->group = FMB_SHORTCUT_GROUP_PLACES_DEFAULT;
      shortcut->file = file;
      shortcut->gicon = g_themed_icon_new ("go-home");
      shortcut->sort_id = 0;
      shortcut->hidden = fmb_shortcuts_model_get_hidden (model, shortcut);
      fmb_shortcuts_model_add_shortcut (model, shortcut);
    }

  /* add desktop entry */
  desktop = fmb_g_file_new_for_desktop ();
  if (!g_file_equal (desktop, home))
    {
      file = fmb_file_get (desktop, NULL);
      if (file != NULL)
        {
          shortcut = g_slice_new0 (FmbShortcut);
          shortcut->group = FMB_SHORTCUT_GROUP_PLACES_DEFAULT;
          shortcut->file = file;
          shortcut->sort_id =  1;
          shortcut->hidden = fmb_shortcuts_model_get_hidden (model, shortcut);
          fmb_shortcuts_model_add_shortcut (model, shortcut);
        }
    }
  g_object_unref (desktop);
  g_object_unref (home);

  /* append the trash icon if the trash is supported */
  if (fmb_g_vfs_is_uri_scheme_supported ("trash"))
    {
      trash = fmb_g_file_new_for_trash ();
      file = fmb_file_get (trash, NULL);
      g_object_unref (trash);

      if (file != NULL)
        {
          shortcut = g_slice_new0 (FmbShortcut);
          shortcut->group = FMB_SHORTCUT_GROUP_PLACES_TRASH;
          shortcut->name = g_strdup (_("Trash"));
          shortcut->file = file;
          shortcut->hidden = fmb_shortcuts_model_get_hidden (model, shortcut);
          fmb_shortcuts_model_add_shortcut (model, shortcut);
        }
    }

  /* determine the URI to the Gtk+ bookmarks file */
  model->bookmarks_file = fmb_g_file_new_for_bookmarks ();

  /* register with the alteration monitor for the bookmarks file */
  model->bookmarks_monitor = g_file_monitor_file (model->bookmarks_file, G_FILE_MONITOR_NONE, NULL, NULL);
  if (G_LIKELY (model->bookmarks_monitor != NULL))
    {
      g_signal_connect (model->bookmarks_monitor, "changed",
        G_CALLBACK (fmb_shortcuts_model_monitor), model);
    }

  /* read the Gtk+ bookmarks file */
  model->bookmarks_idle_id = g_idle_add_full (G_PRIORITY_DEFAULT, fmb_shortcuts_model_load, model, NULL);
}



static gboolean
fmb_shortcuts_model_get_hidden (FmbShortcutsModel *model,
                                   FmbShortcut       *shortcut)
{
  gchar    *uri;
  guint     n;
  gboolean  hidden = FALSE;

  _fmb_return_val_if_fail (FMB_IS_SHORTCUTS_MODEL (model), FALSE);
  _fmb_return_val_if_fail (shortcut->device == NULL, FALSE);

  if (model->hidden_bookmarks == NULL)
    return FALSE;

  /* get the uri */
  if (shortcut->file != NULL)
    uri = fmb_file_dup_uri (shortcut->file);
  else if (shortcut->location != NULL)
    uri = g_file_get_uri (shortcut->location);
  else
    _fmb_assert_not_reached ();

  if (uri == NULL)
    return FALSE;

  for (n = 0; !hidden && model->hidden_bookmarks[n] != NULL; n++)
    hidden = (g_strcmp0 (model->hidden_bookmarks[n], uri) == 0);

  g_free (uri);

  return hidden;
}



static gint
fmb_shortcuts_model_sort_func (gconstpointer shortcut_a,
                                  gconstpointer shortcut_b)
{
  const FmbShortcut *a = shortcut_a;
  const FmbShortcut *b = shortcut_b;

  /* sort groups */
  if (a->group != b->group)
    return a->group - b->group;

  /* use sort order */
  if (a->sort_id != b->sort_id)
    return a->sort_id > b->sort_id ? 1 : -1;

  /* properly sort devices by timestamp */
  if (a->device != NULL && b->device != NULL)
    return fmb_device_sort (a->device, b->device);

  return g_strcmp0 (a->name, b->name);
}



static void
fmb_shortcuts_model_add_shortcut_with_path (FmbShortcutsModel *model,
                                               FmbShortcut       *shortcut,
                                               GtkTreePath          *path)
{
  GtkTreeIter  iter;
  GtkTreePath *sorted_path = NULL;

  _fmb_return_if_fail (FMB_IS_SHORTCUTS_MODEL (model));
  _fmb_return_if_fail (shortcut->file == NULL || FMB_IS_FILE (shortcut->file));

  /* we want to stay informed about changes to the file */
  if (G_LIKELY (shortcut->file != NULL))
    {
      /* watch the file for changes */
      fmb_file_watch (shortcut->file);

      /* connect appropriate signals */
      g_signal_connect (G_OBJECT (shortcut->file), "changed",
                        G_CALLBACK (fmb_shortcuts_model_file_changed), model);
      g_signal_connect (G_OBJECT (shortcut->file), "destroy",
                        G_CALLBACK (fmb_shortcuts_model_file_destroy), model);
    }

  if (path == NULL)
    {
      /* insert the new shortcut to the shortcuts list */
      model->shortcuts = g_list_insert_sorted (model->shortcuts, shortcut, fmb_shortcuts_model_sort_func);
      sorted_path = gtk_tree_path_new_from_indices (g_list_index (model->shortcuts, shortcut), -1);
      path = sorted_path;
    }
  else
    {
      model->shortcuts = g_list_insert (model->shortcuts, shortcut, gtk_tree_path_get_indices (path)[0]);
    }

  /* tell everybody that we have a new shortcut */
  gtk_tree_model_get_iter (GTK_TREE_MODEL (model), &iter, path);
  gtk_tree_model_row_inserted (GTK_TREE_MODEL (model), path, &iter);

  if (sorted_path)
    gtk_tree_path_free (sorted_path);
}



static void
fmb_shortcuts_model_add_shortcut (FmbShortcutsModel *model,
                                     FmbShortcut       *shortcut)
{
  _fmb_return_if_fail (FMB_IS_SHORTCUTS_MODEL (model));
  _fmb_return_if_fail (shortcut->file == NULL || FMB_IS_FILE (shortcut->file));

  fmb_shortcuts_model_add_shortcut_with_path (model, shortcut, NULL);
}



static void
fmb_shortcuts_model_remove_shortcut (FmbShortcutsModel *model,
                                        FmbShortcut       *shortcut)
{
  GtkTreePath *path;
  gint         idx;
  gboolean     needs_save;

  /* determine the index of the shortcut */
  idx = g_list_index (model->shortcuts, shortcut);
  if (G_LIKELY (idx >= 0))
    {
      /* unlink the shortcut from the model */
      model->shortcuts = g_list_remove (model->shortcuts, shortcut);

      /* tell everybody that we have lost a shortcut */
      path = gtk_tree_path_new_from_indices (idx, -1);
      gtk_tree_model_row_deleted (GTK_TREE_MODEL (model), path);
      gtk_tree_path_free (path);

      /* check if we need to save */
      needs_save = (shortcut->group == FMB_SHORTCUT_GROUP_PLACES_BOOKMARKS);

      /* actually free the shortcut */
      fmb_shortcut_free (shortcut, model);

      /* the shortcuts list was changed, so write the gtk bookmarks file */
      if (needs_save)
        fmb_shortcuts_model_save (model);

      /* update header visibility */
      fmb_shortcuts_model_header_visibility (model);
    }
}



static gboolean
fmb_shortcuts_model_local_file (GFile *gfile)
{
  _fmb_return_val_if_fail (G_IS_FILE (gfile), FALSE);

  /* schemes we'd like to have as FmbFiles in the model */
  if (g_file_has_uri_scheme (gfile, "file")
      || g_file_has_uri_scheme (gfile, "computer")
      || g_file_has_uri_scheme (gfile, "recent"))
    return TRUE;

  return FALSE;
}



static void
fmb_shortcuts_model_load_line (GFile       *file_path,
                                  const gchar *name,
                                  gint         row_num,
                                  gpointer     user_data)
{
  FmbShortcutsModel *model = FMB_SHORTCUTS_MODEL (user_data);
  FmbShortcut       *shortcut;
  FmbFile           *file;

  _fmb_return_if_fail (G_IS_FILE (file_path));
  _fmb_return_if_fail (FMB_IS_SHORTCUTS_MODEL (model));
  _fmb_return_if_fail (name == NULL || g_utf8_validate (name, -1, NULL));

  /* handle local and remove files differently */
  if (fmb_shortcuts_model_local_file (file_path))
    {
      /* try to open the file corresponding to the uri */
      file = fmb_file_get (file_path, NULL);
      if (G_UNLIKELY (file == NULL))
        return;

      /* make sure the file refers to a directory */
      if (G_UNLIKELY (fmb_file_is_directory (file)))
        {
          /* create the shortcut entry */
          shortcut = g_slice_new0 (FmbShortcut);
          shortcut->group = FMB_SHORTCUT_GROUP_PLACES_BOOKMARKS;
          shortcut->file = file;
          shortcut->sort_id = row_num;
          shortcut->hidden = fmb_shortcuts_model_get_hidden (model, shortcut);
          shortcut->name = g_strdup (name);

          /* append the shortcut to the list */
          fmb_shortcuts_model_add_shortcut (model, shortcut);
        }
      else
        {
          g_object_unref (file);
        }
    }
  else
    {
      /* create the shortcut entry */
      shortcut = g_slice_new0 (FmbShortcut);
      shortcut->group = FMB_SHORTCUT_GROUP_PLACES_BOOKMARKS;
      shortcut->gicon = g_themed_icon_new ("folder-remote");
      shortcut->location = g_object_ref (file_path);
      shortcut->sort_id = row_num;
      shortcut->hidden = fmb_shortcuts_model_get_hidden (model, shortcut);
      shortcut->name = g_strdup (name);

      /* append the shortcut to the list */
      fmb_shortcuts_model_add_shortcut (model, shortcut);
    }
}



static gboolean
fmb_shortcuts_model_load (gpointer data)
{
  FmbShortcutsModel *model = FMB_SHORTCUTS_MODEL (data);

  _fmb_return_val_if_fail (FMB_IS_SHORTCUTS_MODEL (model), FALSE);

  GDK_THREADS_ENTER ();

  /* parse the bookmarks */
  fmb_util_load_bookmarks (model->bookmarks_file,
                              fmb_shortcuts_model_load_line,
                              model);

  /* update the visibility */
  fmb_shortcuts_model_header_visibility (model);

  GDK_THREADS_LEAVE ();

  model->bookmarks_idle_id = 0;

  return FALSE;
}



static gboolean
fmb_shortcuts_model_reload (gpointer data)
{
  FmbShortcutsModel *model = FMB_SHORTCUTS_MODEL (data);
  FmbShortcut       *shortcut;
  GtkTreePath          *path;
  GList                *lp;
  gint                  idx;

  _fmb_return_val_if_fail (FMB_IS_SHORTCUTS_MODEL (model), FALSE);

  GDK_THREADS_ENTER ();

  /* drop all existing user-defined shortcuts from the model */
  for (idx = 0, lp = model->shortcuts; lp != NULL; )
    {
      /* grab the shortcut */
      shortcut = FMB_SHORTCUT (lp->data);

      /* advance to the next list item */
      lp = g_list_next (lp);

      /* drop the shortcut if it is user-defined */
      if (shortcut->group == FMB_SHORTCUT_GROUP_PLACES_BOOKMARKS)
        {
          /* unlink the shortcut from the model */
          model->shortcuts = g_list_remove (model->shortcuts, shortcut);

          /* tell everybody that we have lost a shortcut */
          path = gtk_tree_path_new_from_indices (idx, -1);
          gtk_tree_model_row_deleted (GTK_TREE_MODEL (model), path);
          gtk_tree_path_free (path);

          /* actually free the shortcut */
          fmb_shortcut_free (shortcut, model);
        }
      else
        {
          ++idx;
        }
    }

  GDK_THREADS_LEAVE ();

  /* load new bookmarks */
  return fmb_shortcuts_model_load (data);
}



static void
fmb_shortcuts_model_monitor (GFileMonitor     *monitor,
                                GFile            *file,
                                GFile            *other_file,
                                GFileMonitorEvent event_type,
                                gpointer          user_data)
{
  FmbShortcutsModel *model = FMB_SHORTCUTS_MODEL (user_data);

  _fmb_return_if_fail (FMB_IS_SHORTCUTS_MODEL (model));
  _fmb_return_if_fail (model->bookmarks_monitor == monitor);

  /* leave if we saved less than 2 seconds ago */
  if (model->bookmarks_time + 2 * G_USEC_PER_SEC > g_get_real_time ())
    return;

  /* reload the shortcuts model */
  if (model->bookmarks_idle_id == 0)
    model->bookmarks_idle_id = g_idle_add (fmb_shortcuts_model_reload, model);
}



static void
fmb_shortcuts_model_save (FmbShortcutsModel *model)
{
  GString        *contents;
  FmbShortcut *shortcut;
  gchar          *bookmarks_path;
  gchar          *uri;
  GList          *lp;
  GError         *err = NULL;
  GFile          *parent = NULL;

  _fmb_return_if_fail (FMB_IS_SHORTCUTS_MODEL (model));

  contents = g_string_new (NULL);

  for (lp = model->shortcuts; lp != NULL; lp = lp->next)
    {
      shortcut = FMB_SHORTCUT (lp->data);
      if (shortcut->group == FMB_SHORTCUT_GROUP_PLACES_BOOKMARKS)
        {
          if (shortcut->file != NULL)
            uri = fmb_file_dup_uri (shortcut->file);
          else if (shortcut->location != NULL)
            uri = g_file_get_uri (shortcut->location);
          else
            continue;

          if (G_LIKELY (shortcut->name != NULL))
            g_string_append_printf (contents, "%s %s\n", uri, shortcut->name);
          else
            g_string_append_printf (contents, "%s\n", uri);
          g_free (uri);
        }
    }

  /* create folder if it does not exist */
  parent = g_file_get_parent (model->bookmarks_file);
  if (!g_file_make_directory_with_parents (parent, NULL, &err))
    {
       if (g_error_matches (err, G_IO_ERROR, G_IO_ERROR_EXISTS))
         {
           g_clear_error (&err);
         }
       else
         {
           g_warning ("Failed to create bookmarks folder: %s", err->message);
           g_error_free (err);
         }
    }
  g_clear_object (&parent);

  /* write data to the disk */
  bookmarks_path = g_file_get_path (model->bookmarks_file);
  if (!g_file_set_contents (bookmarks_path, contents->str, contents->len, &err))
    {
      g_warning ("Failed to write \"%s\": %s", bookmarks_path, err->message);
      g_error_free (err);
    }
  g_free (bookmarks_path);
  g_string_free (contents, TRUE);

  /* store the save time */
  model->bookmarks_time = g_get_real_time ();
}



static void
fmb_shortcuts_model_file_changed (FmbFile           *file,
                                     FmbShortcutsModel *model)
{
  FmbShortcut *shortcut;
  GtkTreePath    *path;
  GtkTreeIter     iter;
  GList          *lp;
  gint            idx;

  _fmb_return_if_fail (FMB_IS_FILE (file));
  _fmb_return_if_fail (FMB_IS_SHORTCUTS_MODEL (model));

  /* check if the file still refers to a directory or a not mounted URI,
   * otherwise we cannot keep it on the shortcuts list, and so we'll treat
   * it like the file was destroyed (and thereby removed) */

  if (G_UNLIKELY (!fmb_file_is_directory (file)))
    {
      fmb_shortcuts_model_file_destroy (file, model);
      return;
    }

  for (idx = 0, lp = model->shortcuts; lp != NULL; ++idx, lp = lp->next)
    {
      shortcut = FMB_SHORTCUT (lp->data);
      if (shortcut->file == file)
        {
          GTK_TREE_ITER_INIT (iter, model->stamp, lp);

          path = gtk_tree_path_new_from_indices (idx, -1);
          gtk_tree_model_row_changed (GTK_TREE_MODEL (model), path, &iter);
          gtk_tree_path_free (path);
          break;
        }
    }
}



static void
fmb_shortcuts_model_file_destroy (FmbFile           *file,
                                     FmbShortcutsModel *model)
{
  FmbShortcut *shortcut = NULL;
  GList          *lp;

  _fmb_return_if_fail (FMB_IS_FILE (file));
  _fmb_return_if_fail (FMB_IS_SHORTCUTS_MODEL (model));

  /* lookup the shortcut matching the file */
  for (lp = model->shortcuts; lp != NULL; lp = lp->next)
    {
      shortcut = FMB_SHORTCUT (lp->data);
      if (shortcut->file == file)
        break;
    }

  /* verify that we actually found a shortcut */
  _fmb_assert (lp != NULL);
  _fmb_assert (FMB_IS_FILE (shortcut->file));

  /* drop the shortcut from the model */
  fmb_shortcuts_model_remove_shortcut (model, shortcut);
}



static void
fmb_shortcuts_model_device_added (FmbDeviceMonitor  *device_monitor,
                                     FmbDevice         *device,
                                     FmbShortcutsModel *model)
{
  FmbShortcut *shortcut;

  _fmb_return_if_fail (device_monitor == NULL || FMB_DEVICE_MONITOR (device_monitor));
  _fmb_return_if_fail (device_monitor == NULL || model->device_monitor == device_monitor);
  _fmb_return_if_fail (FMB_IS_DEVICE (device));
  _fmb_return_if_fail (FMB_IS_SHORTCUTS_MODEL (model));

  /* allocate a new shortcut */
  shortcut = g_slice_new0 (FmbShortcut);
  shortcut->device = g_object_ref (device);
  shortcut->hidden = fmb_device_get_hidden (device);

  switch (fmb_device_get_kind (device))
    {
    case FMB_DEVICE_KIND_VOLUME:
      shortcut->group = FMB_SHORTCUT_GROUP_DEVICES_VOLUMES;
      break;

    case FMB_DEVICE_KIND_MOUNT_LOCAL:
      shortcut->group = FMB_SHORTCUT_GROUP_DEVICES_MOUNTS;
      break;

    case FMB_DEVICE_KIND_MOUNT_REMOTE:
      shortcut->group = FMB_SHORTCUT_GROUP_NETWORK_MOUNTS;
      break;
    }

  /* insert in the model */
  fmb_shortcuts_model_add_shortcut (model, shortcut);

  /* header visibility if call is from monitor */
  if (device_monitor != NULL
      && !shortcut->hidden)
    fmb_shortcuts_model_header_visibility (model);
}



static void
fmb_shortcuts_model_device_removed (FmbDeviceMonitor  *device_monitor,
                                       FmbDevice         *device,
                                       FmbShortcutsModel *model)
{
  GList *lp;

  _fmb_return_if_fail (FMB_DEVICE_MONITOR (device_monitor));
  _fmb_return_if_fail (model->device_monitor == device_monitor);
  _fmb_return_if_fail (FMB_IS_DEVICE (device));
  _fmb_return_if_fail (FMB_IS_SHORTCUTS_MODEL (model));

  /* find the shortcut */
  for (lp = model->shortcuts; lp != NULL; lp = lp->next)
    if (FMB_SHORTCUT (lp->data)->device == device)
      break;

  /* something is broken if we don't have a shortcut here */
  _fmb_assert (lp != NULL);
  _fmb_assert (FMB_SHORTCUT (lp->data)->device == device);

  /* drop the shortcut from the model */
  if (G_LIKELY (lp != NULL))
    fmb_shortcuts_model_remove_shortcut (model, lp->data);
}



static void
fmb_shortcuts_model_device_changed (FmbDeviceMonitor  *device_monitor,
                                       FmbDevice         *device,
                                       FmbShortcutsModel *model)
{
  GtkTreeIter     iter;
  GList          *lp;
  gint            idx;
  GtkTreePath    *path;
  FmbShortcut *shortcut;
  gboolean        update_header = FALSE;

  _fmb_return_if_fail (FMB_DEVICE_MONITOR (device_monitor));
  _fmb_return_if_fail (model->device_monitor == device_monitor);
  _fmb_return_if_fail (FMB_IS_DEVICE (device));
  _fmb_return_if_fail (FMB_IS_SHORTCUTS_MODEL (model));

    /* find the shortcut */
  for (lp = model->shortcuts, idx = 0; lp != NULL; lp = lp->next, idx++)
    if (FMB_SHORTCUT (lp->data)->device == device)
      break;

  /* something is broken if we don't have a shortcut here */
  _fmb_assert (lp != NULL);
  _fmb_assert (FMB_SHORTCUT (lp->data)->device == device);

  if (G_LIKELY (lp != NULL))
    {
      shortcut = lp->data;

      /* cleanup tooltip */
      g_free (shortcut->tooltip);
      shortcut->tooltip = NULL;

      /* hidden state */
      if (shortcut->hidden != fmb_device_get_hidden (device))
        {
          shortcut->hidden = fmb_device_get_hidden (device);
          update_header = TRUE;
        }

      /* generate an iterator for the path */
      GTK_TREE_ITER_INIT (iter, model->stamp, lp);

      /* tell the view that the volume has changed in some way */
      path = gtk_tree_path_new_from_indices (idx, -1);
      gtk_tree_model_row_changed (GTK_TREE_MODEL (model), path, &iter);
      gtk_tree_path_free (path);
    }

  /* header visibility */
  if (update_header)
    fmb_shortcuts_model_header_visibility (model);
}



static void
fmb_shortcut_free (FmbShortcut       *shortcut,
                      FmbShortcutsModel *model)
{
  if (G_LIKELY (shortcut->file != NULL))
    {
      /* drop the file watch */
      fmb_file_unwatch (shortcut->file);

      /* unregister from the file */
      g_signal_handlers_disconnect_matched (shortcut->file,
                                            G_SIGNAL_MATCH_DATA, 0,
                                            0, NULL, NULL, model);
      g_object_unref (shortcut->file);
    }

  if (G_LIKELY (shortcut->device != NULL))
    g_object_unref (shortcut->device);

  if (G_LIKELY (shortcut->gicon != NULL))
    g_object_unref (shortcut->gicon);

  if (G_LIKELY (shortcut->location != NULL))
    g_object_unref (shortcut->location);

  g_free (shortcut->name);
  g_free (shortcut->tooltip);

  /* release the shortcut itself */
  g_slice_free (FmbShortcut, shortcut);
}



static gboolean
fmb_shortcuts_model_busy_timeout (gpointer data)
{
  FmbShortcutsModel *model = FMB_SHORTCUTS_MODEL (data);
  gboolean              keep_running = FALSE;
  FmbShortcut       *shortcut;
  GtkTreePath          *path;
  guint                 idx;
  GtkTreeIter           iter;
  GList                *lp;

  _fmb_return_val_if_fail (FMB_IS_SHORTCUTS_MODEL (data), FALSE);

  for (lp = model->shortcuts, idx = 0; lp != NULL; lp = lp->next, idx++)
    {
      shortcut = lp->data;
      if (!shortcut->busy)
        continue;

      /* loop the pulse of the shortcut */
      shortcut->busy_pulse++;
      if (shortcut->busy_pulse >= SPINNER_NUM_STEPS)
        shortcut->busy_pulse = 0;

      /* generate an iterator for the path */
      GTK_TREE_ITER_INIT (iter, model->stamp, lp);

      /* notify the views about the change */
      path = gtk_tree_path_new_from_indices (idx, -1);
      gtk_tree_model_row_changed (GTK_TREE_MODEL (model), path, &iter);
      gtk_tree_path_free (path);

      /* keep the timeout running */
      keep_running = TRUE;
    }

  return keep_running;
}



static void
fmb_shortcuts_model_busy_timeout_destroyed (gpointer data)
{
  FMB_SHORTCUTS_MODEL (data)->busy_timeout_id = 0;
}



/**
 * fmb_shortcuts_model_get_default:
 *
 * Returns the default #FmbShortcutsModel instance shared by
 * all #FmbShortcutsView instances.
 *
 * Call #g_object_unref() on the returned object when you
 * don't need it any longer.
 *
 * Return value: the default #FmbShortcutsModel instance.
 **/
FmbShortcutsModel*
fmb_shortcuts_model_get_default (void)
{
  static FmbShortcutsModel *model = NULL;

  if (G_UNLIKELY (model == NULL))
    {
      model = g_object_new (FMB_TYPE_SHORTCUTS_MODEL, NULL);
      g_object_add_weak_pointer (G_OBJECT (model), (gpointer) &model);
    }
  else
    {
      g_object_ref (G_OBJECT (model));
    }

  return model;
}



/**
 * fmb_shortcuts_model_has_bookmark:
 * @model : a #FmbShortcutsModel instance.
 * @file  : a #ThuanrFile instance.
 *
 * Returns %TRUE if there is a bookmark (not a mount or volume) with
 * @file as destination.
 *
 * Return value: %TRUE if @file was found, else %FALSE.
 **/
gboolean
fmb_shortcuts_model_has_bookmark (FmbShortcutsModel *model,
                                     GFile                *file)
{
  GList          *lp;
  FmbShortcut *shortcut;

  _fmb_return_val_if_fail (FMB_IS_SHORTCUTS_MODEL (model), FALSE);
  _fmb_return_val_if_fail (G_IS_FILE (file), FALSE);

  for (lp = model->shortcuts; lp != NULL; lp = lp->next)
    {
      shortcut = lp->data;

      /* only check bookmarks */
      if (shortcut->group != FMB_SHORTCUT_GROUP_PLACES_BOOKMARKS)
        continue;

      if (shortcut->file != NULL
          && g_file_equal (fmb_file_get_file (shortcut->file), file))
        return TRUE;

      if (shortcut->location != NULL
          && g_file_equal (shortcut->location, file))
        return TRUE;
    }

  return FALSE;
}



/**
 * fmb_shortcuts_model_iter_for_file:
 * @model : a #FmbShortcutsModel instance.
 * @file  : a #ThuanrFile instance.
 * @iter  : pointer to a #GtkTreeIter.
 *
 * Tries to lookup the #GtkTreeIter, that belongs to a shortcut, which
 * refers to @file and stores it to @iter. If no such #GtkTreeIter was
 * found, %FALSE will be returned and @iter won't be changed. Else
 * %TRUE will be returned and @iter will be set appropriately.
 *
 * Return value: %TRUE if @file was found, else %FALSE.
 **/
gboolean
fmb_shortcuts_model_iter_for_file (FmbShortcutsModel *model,
                                      FmbFile           *file,
                                      GtkTreeIter          *iter)
{
  GFile          *mount_point;
  GList          *lp;
  FmbShortcut *shortcut;

  _fmb_return_val_if_fail (FMB_IS_SHORTCUTS_MODEL (model), FALSE);
  _fmb_return_val_if_fail (FMB_IS_FILE (file), FALSE);
  _fmb_return_val_if_fail (iter != NULL, FALSE);

  for (lp = model->shortcuts; lp != NULL; lp = lp->next)
    {
      shortcut = lp->data;

      /* check if we have a location that matches */
      if (shortcut->file == file)
        {
          GTK_TREE_ITER_INIT (*iter, model->stamp, lp);
          return TRUE;
        }

      if (shortcut->location != NULL
          && g_file_equal (shortcut->location, fmb_file_get_file (file)))
        {
          GTK_TREE_ITER_INIT (*iter, model->stamp, lp);
          return TRUE;
        }

      /* but maybe we have a mounted(!) volume with a matching mount point */
      if (shortcut->device != NULL
          && !fmb_device_get_hidden (shortcut->device))
        {
          mount_point = fmb_device_get_root (shortcut->device);
          if (mount_point != NULL)
            {
              if (g_file_equal (mount_point, fmb_file_get_file (file)))
                {
                  GTK_TREE_ITER_INIT (*iter, model->stamp, lp);
                  g_object_unref (mount_point);
                  return TRUE;
                }

              g_object_unref (mount_point);
            }
        }
    }

  return FALSE;
}



/**
 * fmb_shortcuts_model_drop_possible:
 * @model : a #FmbShortcutstModel.
 * @path  : a #GtkTreePath.
 *
 * Determines whether a drop is possible before the given @path, at the same depth
 * as @path. I.e., can we drop data at that location. @path does not have to exist;
 * the return value will almost certainly be FALSE if the parent of @path doesn't
 * exist, though.
 *
 * Return value: %TRUE if it's possible to drop data before @path, else %FALSE.
 **/
gboolean
fmb_shortcuts_model_drop_possible (FmbShortcutsModel *model,
                                      GtkTreePath          *path)
{
  FmbShortcut *shortcut;

  _fmb_return_val_if_fail (FMB_IS_SHORTCUTS_MODEL (model), FALSE);
  _fmb_return_val_if_fail (gtk_tree_path_get_depth (path) > 0, FALSE);

  /* determine the list item for the path */
  shortcut = g_list_nth_data (model->shortcuts, gtk_tree_path_get_indices (path)[0]);

  /* append to the list is not possible */
  if (G_LIKELY (shortcut == NULL))
    return FALSE;

  /* cannot drop before special shortcuts! */
  if (shortcut->group == FMB_SHORTCUT_GROUP_PLACES_BOOKMARKS)
    return TRUE;

  /* we can drop at the end of the bookmarks (before network header) */
  if (shortcut->group == FMB_SHORTCUT_GROUP_NETWORK_HEADER)
    return TRUE;

  return FALSE;
}



/**
 * fmb_shortcuts_model_add:
 * @model    : a #FmbShortcutsModel.
 * @dst_path : the destination path.
 * @file     : the #FmbFile that should be added to the shortcuts list.
 *
 * Adds the shortcut @file to the @model at @dst_path, unless @file is
 * already present in @model in which case no action is performed.
 **/
void
fmb_shortcuts_model_add (FmbShortcutsModel *model,
                            GtkTreePath          *dst_path,
                            gpointer              file)
{
  FmbShortcut *shortcut;
  GFile          *location;

  _fmb_return_if_fail (FMB_IS_SHORTCUTS_MODEL (model));
  _fmb_return_if_fail (dst_path == NULL || gtk_tree_path_get_depth (dst_path) > 0);
  _fmb_return_if_fail (dst_path == NULL || gtk_tree_path_get_indices (dst_path)[0] >= 0);
  _fmb_return_if_fail (dst_path == NULL || gtk_tree_path_get_indices (dst_path)[0] <= (gint) g_list_length (model->shortcuts));
  _fmb_return_if_fail (FMB_IS_FILE (file) || G_IS_FILE (file));

  location = G_IS_FILE (file) ? file : fmb_file_get_file (file);

  /* verify that the file is not already in use as shortcut */
  if (fmb_shortcuts_model_has_bookmark (model, location))
    return;

  /* create the new shortcut that will be inserted */
  shortcut = g_slice_new0 (FmbShortcut);
  shortcut->group = FMB_SHORTCUT_GROUP_PLACES_BOOKMARKS;

  if (fmb_shortcuts_model_local_file (location))
    {
      shortcut->file = fmb_file_get (location, NULL);
    }
  else
    {
      shortcut->location = g_object_ref (G_OBJECT (location));
      shortcut->gicon = g_themed_icon_new ("folder-remote");
    }

  /* add the shortcut to the list at the given position */
  fmb_shortcuts_model_add_shortcut_with_path (model, shortcut, dst_path);

  /* the shortcuts list was changed, so write the gtk bookmarks file */
  fmb_shortcuts_model_save (model);
}



/**
 * fmb_shortcuts_model_move:
 * @model    : a #FmbShortcutsModel.
 * @src_path : the source path.
 * @dst_path : the destination path.
 *
 * Moves the shortcut at @src_path to @dst_path, adjusting other
 * shortcut's positions as required.
 **/
void
fmb_shortcuts_model_move (FmbShortcutsModel *model,
                             GtkTreePath          *src_path,
                             GtkTreePath          *dst_path)
{
  FmbShortcut *shortcut;
  GtkTreePath    *path;
  GList          *lp;
  gint           *order;
  gint            index_src;
  gint            index_dst;
  gint            idx;
  gint            n_shortcuts;

  _fmb_return_if_fail (FMB_IS_SHORTCUTS_MODEL (model));
  _fmb_return_if_fail (gtk_tree_path_get_depth (src_path) > 0);
  _fmb_return_if_fail (gtk_tree_path_get_depth (dst_path) > 0);
  _fmb_return_if_fail (gtk_tree_path_get_indices (src_path)[0] >= 0);
  _fmb_return_if_fail (gtk_tree_path_get_indices (src_path)[0] < (gint) g_list_length (model->shortcuts));
  _fmb_return_if_fail (gtk_tree_path_get_indices (dst_path)[0] > 0);

  index_src = gtk_tree_path_get_indices (src_path)[0];
  index_dst = gtk_tree_path_get_indices (dst_path)[0];

  if (G_UNLIKELY (index_src == index_dst))
    return;

  /* generate the order for the rows prior the dst/src rows */
  n_shortcuts = g_list_length (model->shortcuts);
  order = g_newa (gint, n_shortcuts);
  for (idx = 0, lp = model->shortcuts; idx < index_src && idx < index_dst; ++idx, lp = lp->next)
    order[idx] = idx;

  if (idx == index_src)
    {
      shortcut = FMB_SHORTCUT (lp->data);

      for (; idx < index_dst; ++idx, lp = lp->next)
        {
          if (lp->next == NULL)
            break;
          lp->data = lp->next->data;
          order[idx] = idx + 1;
        }

      lp->data = shortcut;
      order[idx++] = index_src;
    }
  else
    {
      for (; idx < index_src; ++idx, lp = lp->next)
        ;

      _fmb_assert (idx == index_src);

      shortcut = FMB_SHORTCUT (lp->data);

      for (; idx > index_dst; --idx, lp = lp->prev)
        {
          lp->data = lp->prev->data;
          order[idx] = idx - 1;
        }

      _fmb_assert (idx == index_dst);

      lp->data = shortcut;
      order[idx] = index_src;
      idx = index_src + 1;
    }

  /* generate the remaining order */
  for (; idx < n_shortcuts; ++idx)
    order[idx] = idx;

  /* tell all listeners about the reordering just performed */
  path = gtk_tree_path_new_first ();
  gtk_tree_model_rows_reordered (GTK_TREE_MODEL (model), path, NULL, order);
  gtk_tree_path_free (path);

  /* the shortcuts list was changed, so write the gtk bookmarks file */
  fmb_shortcuts_model_save (model);
}



/**
 * fmb_shortcuts_model_remove:
 * @model : a #FmbShortcutsModel.
 * @path  : the #GtkTreePath of the shortcut to remove.
 *
 * Removes the shortcut at @path from the @model and syncs to
 * on-disk storage. @path must refer to a valid, user-defined
 * shortcut, as you cannot remove system-defined entities (they
 * are managed internally).
 **/
void
fmb_shortcuts_model_remove (FmbShortcutsModel *model,
                               GtkTreePath          *path)
{
  FmbShortcut *shortcut;

  _fmb_return_if_fail (FMB_IS_SHORTCUTS_MODEL (model));
  _fmb_return_if_fail (gtk_tree_path_get_depth (path) > 0);
  _fmb_return_if_fail (gtk_tree_path_get_indices (path)[0] >= 0);
  _fmb_return_if_fail (gtk_tree_path_get_indices (path)[0] < (gint) g_list_length (model->shortcuts));

  /* lookup the shortcut for the given path */
  shortcut = g_list_nth_data (model->shortcuts, gtk_tree_path_get_indices (path)[0]);

  /* verify that the shortcut is removable */
  _fmb_assert (shortcut->group == FMB_SHORTCUT_GROUP_PLACES_BOOKMARKS);

  /* remove the shortcut (using the file destroy handler) */
  fmb_shortcuts_model_remove_shortcut (model, shortcut);
}



/**
 * fmb_shortcuts_model_rename:
 * @model : a #FmbShortcutsModel.
 * @iter  : the #GtkTreeIter which refers to the shortcut that
 *          should be renamed to @name.
 * @name  : the new name for the shortcut at @path or %NULL to
 *          return to the default name.
 *
 * Renames the shortcut at @iter to the new @name in @model.
 *
 * @name may be %NULL or an empty to reset the shortcut to
 * its default name.
 **/
void
fmb_shortcuts_model_rename (FmbShortcutsModel *model,
                               GtkTreeIter          *iter,
                               const gchar          *name)
{
  FmbShortcut *shortcut;
  GtkTreePath    *path;

  _fmb_return_if_fail (name == NULL || g_utf8_validate (name, -1, NULL));
  _fmb_return_if_fail (FMB_IS_SHORTCUTS_MODEL (model));
  _fmb_return_if_fail (iter->stamp == model->stamp);
  _fmb_return_if_fail (iter->user_data != NULL);

  /* lookup the shortcut for the given path */
  shortcut = FMB_SHORTCUT (((GList *) iter->user_data)->data);

  /* verify the shortcut */
  _fmb_assert (shortcut->group == FMB_SHORTCUT_GROUP_PLACES_BOOKMARKS);

  /* perform the rename */
  if (G_UNLIKELY (shortcut->name != NULL))
    g_free (shortcut->name);
  if (G_UNLIKELY (name == NULL || *name == '\0'))
    shortcut->name = NULL;
  else
    shortcut->name = g_strdup (name);

  /* notify the views about the change */
  path = gtk_tree_model_get_path (GTK_TREE_MODEL (model), iter);
  gtk_tree_model_row_changed (GTK_TREE_MODEL (model), path, iter);
  gtk_tree_path_free (path);

  /* save the changes to the model */
  fmb_shortcuts_model_save (model);
}



void
fmb_shortcuts_model_set_busy (FmbShortcutsModel *model,
                                 FmbDevice         *device,
                                 gboolean              busy)
{
  FmbShortcut *shortcut;
  GList          *lp;
  guint           idx;
  GtkTreeIter     iter;
  GtkTreePath    *path;

  _fmb_return_if_fail (FMB_IS_SHORTCUTS_MODEL (model));
  _fmb_return_if_fail (FMB_IS_DEVICE (device));

  /* get the device */
  for (lp = model->shortcuts, idx = 0; lp != NULL; lp = lp->next, idx++)
    if (FMB_SHORTCUT (lp->data)->device == device)
      break;

  if (lp == NULL)
    return;

  shortcut = lp->data;
  _fmb_assert (shortcut->device == device);

  if (G_LIKELY (shortcut->busy != busy))
    {
      shortcut->busy = busy;

      if (busy && model->busy_timeout_id == 0)
        {
          /* start the global cycle timeout */
          model->busy_timeout_id =
            gdk_threads_add_timeout_full (G_PRIORITY_DEFAULT,
                                          SPINNER_CYCLE_DURATION / SPINNER_NUM_STEPS,
                                          fmb_shortcuts_model_busy_timeout, model,
                                          fmb_shortcuts_model_busy_timeout_destroyed);
        }
      else if (!busy)
        {
          /* generate an iterator for the path */
          GTK_TREE_ITER_INIT (iter, model->stamp, lp);

          /* notify the views about the change */
          path = gtk_tree_path_new_from_indices (idx, -1);
          gtk_tree_model_row_changed (GTK_TREE_MODEL (model), path, &iter);
          gtk_tree_path_free (path);
        }
    }
}




void
fmb_shortcuts_model_set_hidden (FmbShortcutsModel *model,
                                   GtkTreePath          *path,
                                   gboolean              hidden)
{
  FmbShortcut  *shortcut;
  guint            length;
  gchar          **bookmarks;
  guint            pos;
  gchar           *uri;
  guint            n;

  _fmb_return_if_fail (FMB_IS_SHORTCUTS_MODEL (model));
  _fmb_return_if_fail (gtk_tree_path_get_depth (path) > 0);
  _fmb_return_if_fail (gtk_tree_path_get_indices (path)[0] >= 0);
  _fmb_return_if_fail (gtk_tree_path_get_indices (path)[0] < (gint) g_list_length (model->shortcuts));

  /* lookup the shortcut for the given path */
  shortcut = g_list_nth_data (model->shortcuts, gtk_tree_path_get_indices (path)[0]);
  _fmb_return_if_fail (shortcut != NULL);

  if (shortcut->device != NULL)
    {
      /* if this is a device, store in the device monitor */
      fmb_device_monitor_set_hidden (model->device_monitor, shortcut->device, hidden);
      return;
    }

  /* get the uri */
  if (shortcut->file != NULL)
    uri = fmb_file_dup_uri (shortcut->file);
  else if (shortcut->location != NULL)
    uri = g_file_get_uri (shortcut->location);
  else
    _fmb_assert_not_reached ();

  /* prepare array */
  length = model->hidden_bookmarks != NULL ? g_strv_length (model->hidden_bookmarks) : 0;
  bookmarks = g_new0 (gchar *, length + 2);
  pos = 0;

  /* copy other uuid in the new list */
  if (model->hidden_bookmarks != NULL)
    {
      for (n = 0; model->hidden_bookmarks[n] != NULL; n++)
        if (g_strcmp0 (model->hidden_bookmarks[n], uri) != 0)
          bookmarks[pos++] = g_strdup (model->hidden_bookmarks[n]);
    }

  /* add the new uri if it should hide */
  if (hidden)
    bookmarks[pos++] = uri;
  else
    g_free (uri);

  /* store new list */
  g_object_set (G_OBJECT (model->preferences), "hidden-bookmarks", bookmarks, NULL);
  g_strfreev (bookmarks);

  /* header visibility */
  fmb_shortcuts_model_header_visibility (model);
}

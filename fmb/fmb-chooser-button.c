/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2005-2006 Benedikt Meurer <benny@xfce.org>
 * Copyright (c) 2010      Nick Schermer <nick@xfce.org>
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

#include <fmb/fmb-chooser-button.h>
#include <fmb/fmb-chooser-dialog.h>
#include <fmb/fmb-dialogs.h>
#include <fmb/fmb-gobject-extensions.h>
#include <fmb/fmb-gtk-extensions.h>
#include <fmb/fmb-icon-factory.h>
#include <fmb/fmb-pango-extensions.h>
#include <fmb/fmb-private.h>



/* Property identifiers */
enum
{
  PROP_0,
  PROP_FILE,
};



enum
{
  FMB_CHOOSER_BUTTON_STORE_COLUMN_NAME,
  FMB_CHOOSER_BUTTON_STORE_COLUMN_ICON,
  FMB_CHOOSER_BUTTON_STORE_COLUMN_APPLICATION,
  FMB_CHOOSER_BUTTON_STORE_COLUMN_SENSITIVE,
  FMB_CHOOSER_BUTTON_STORE_COLUMN_STYLE,
  FMB_CHOOSER_BUTTON_STORE_N_COLUMNS
};



static void     fmb_chooser_button_finalize          (GObject             *object);
static void     fmb_chooser_button_get_property      (GObject             *object,
                                                         guint                prop_id,
                                                         GValue              *value,
                                                         GParamSpec          *pspec);
static void     fmb_chooser_button_set_property      (GObject             *object,
                                                         guint                prop_id,
                                                         const GValue        *value,
                                                         GParamSpec          *pspec);
static gboolean fmb_chooser_button_scroll_event      (GtkWidget           *widget,
                                                         GdkEventScroll      *event);
static void     fmb_chooser_button_changed           (GtkComboBox         *combo_box);
static void     fmb_chooser_button_popup             (FmbChooserButton *chooser_button);
static gint     fmb_chooser_button_sort_applications (gconstpointer        a,
                                                         gconstpointer        b);
static gboolean fmb_chooser_button_row_separator     (GtkTreeModel        *model,
                                                         GtkTreeIter         *iter,
                                                         gpointer             data);
static void     fmb_chooser_button_chooser_dialog    (FmbChooserButton *chooser_button);
static void     fmb_chooser_button_file_changed      (FmbChooserButton *chooser_button,
                                                         FmbFile          *file);



struct _FmbChooserButtonClass
{
  GtkComboBoxClass __parent__;
};

struct _FmbChooserButton
{
  GtkComboBox   __parent__;

  GtkListStore *store;
  FmbFile   *file;
  gboolean      has_default_application;
};



G_DEFINE_TYPE (FmbChooserButton, fmb_chooser_button, GTK_TYPE_COMBO_BOX)



static void
fmb_chooser_button_class_init (FmbChooserButtonClass *klass)
{
  GObjectClass   *gobject_class;
  GtkWidgetClass *gtkwidget_class;

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = fmb_chooser_button_finalize;
  gobject_class->get_property = fmb_chooser_button_get_property;
  gobject_class->set_property = fmb_chooser_button_set_property;

  gtkwidget_class = GTK_WIDGET_CLASS (klass);
  gtkwidget_class->scroll_event = fmb_chooser_button_scroll_event;

  /**
   * FmbChooserButton:file:
   *
   * The #FmbFile for which a preferred application should
   * be chosen.
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_FILE,
                                   g_param_spec_object ("file", "file", "file",
                                                        FMB_TYPE_FILE,
                                                        BLXO_PARAM_READWRITE));
}



static void
fmb_chooser_button_init (FmbChooserButton *chooser_button)
{
  GtkCellRenderer *renderer;

  /* allocate a new store for the combo box */
  chooser_button->store = gtk_list_store_new (FMB_CHOOSER_BUTTON_STORE_N_COLUMNS,
                                              G_TYPE_STRING,
                                              G_TYPE_ICON,
                                              G_TYPE_OBJECT,
                                              G_TYPE_BOOLEAN,
                                              PANGO_TYPE_STYLE);
  gtk_combo_box_set_model (GTK_COMBO_BOX (chooser_button), 
                           GTK_TREE_MODEL (chooser_button->store));

  g_signal_connect (chooser_button, "changed", 
                    G_CALLBACK (fmb_chooser_button_changed), NULL);
  g_signal_connect (chooser_button, "popup",
                    G_CALLBACK (fmb_chooser_button_popup), NULL);

  /* set separator function */
  gtk_combo_box_set_row_separator_func (GTK_COMBO_BOX (chooser_button),
                                        fmb_chooser_button_row_separator,
                                        NULL, NULL);

  /* add renderer for the application icon */
  renderer = gtk_cell_renderer_pixbuf_new ();
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (chooser_button), renderer, FALSE);
  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (chooser_button), renderer,
                                  "gicon", 
                                  FMB_CHOOSER_BUTTON_STORE_COLUMN_ICON,
                                  "sensitive", 
                                  FMB_CHOOSER_BUTTON_STORE_COLUMN_SENSITIVE,
                                  NULL);

  /* add renderer for the application name */
  renderer = gtk_cell_renderer_text_new ();
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (chooser_button), renderer, FALSE);
  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (chooser_button), renderer,
                                  "text", 
                                  FMB_CHOOSER_BUTTON_STORE_COLUMN_NAME,
                                  "sensitive", 
                                  FMB_CHOOSER_BUTTON_STORE_COLUMN_SENSITIVE,
                                  "style",
                                  FMB_CHOOSER_BUTTON_STORE_COLUMN_STYLE,
                                  NULL);
}



static void
fmb_chooser_button_finalize (GObject *object)
{
  FmbChooserButton *chooser_button = FMB_CHOOSER_BUTTON (object);

  /* reset the "file" property */
  fmb_chooser_button_set_file (chooser_button, NULL);

  /* release the store */
  g_object_unref (G_OBJECT (chooser_button->store));

  (*G_OBJECT_CLASS (fmb_chooser_button_parent_class)->finalize) (object);
}



static void
fmb_chooser_button_get_property (GObject    *object,
                                    guint       prop_id,
                                    GValue     *value,
                                    GParamSpec *pspec)
{
  FmbChooserButton *chooser_button = FMB_CHOOSER_BUTTON (object);

  switch (prop_id)
    {
    case PROP_FILE:
      g_value_set_object (value, chooser_button->file);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static void
fmb_chooser_button_set_property (GObject      *object,
                                    guint         prop_id,
                                    const GValue *value,
                                    GParamSpec   *pspec)
{
  FmbChooserButton *chooser_button = FMB_CHOOSER_BUTTON (object);

  switch (prop_id)
    {
    case PROP_FILE:
      fmb_chooser_button_set_file (chooser_button, g_value_get_object (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static gboolean
fmb_chooser_button_scroll_event (GtkWidget      *widget,
                                    GdkEventScroll *event)
{
  FmbChooserButton *chooser_button = FMB_CHOOSER_BUTTON (widget);
  GtkTreeIter          iter;
  GObject             *application;
  GtkTreeModel        *model = GTK_TREE_MODEL (chooser_button->store);

  g_return_val_if_fail (FMB_IS_CHOOSER_BUTTON (chooser_button), FALSE);

  /* check if the next application in the store is valid if we scroll down,
   * else drop the event so we don't popup the chooser dailog */
  if (event->direction != GDK_SCROLL_UP
      && gtk_combo_box_get_active_iter (GTK_COMBO_BOX (widget), &iter)
      && gtk_tree_model_iter_next (model, &iter))
    {
      gtk_tree_model_get (model, &iter, 
                          FMB_CHOOSER_BUTTON_STORE_COLUMN_APPLICATION, 
                          &application, -1);

      if (application == NULL)
        return FALSE;

      g_object_unref (G_OBJECT (application));
    }

  return (*GTK_WIDGET_CLASS (fmb_chooser_button_parent_class)->scroll_event) (widget, event);
}



static void
fmb_chooser_button_changed (GtkComboBox *combo_box)
{
  FmbChooserButton *chooser_button = FMB_CHOOSER_BUTTON (combo_box);
  GtkTreeIter          iter;
  const gchar         *content_type;
  GAppInfo            *app_info;
  GError              *error = NULL;

  _fmb_return_if_fail (FMB_IS_CHOOSER_BUTTON (chooser_button));
  _fmb_return_if_fail (GTK_IS_LIST_STORE (chooser_button->store));

  /* verify that we still have a valid file */
  if (G_UNLIKELY (chooser_button->file == NULL))
    return;

  /* get the selected item in the combo box */
  if (!gtk_combo_box_get_active_iter (combo_box, &iter))
    return;

  /* determine the application that was set for the item */
  gtk_tree_model_get (GTK_TREE_MODEL (chooser_button->store), &iter,
                      FMB_CHOOSER_BUTTON_STORE_COLUMN_APPLICATION,
                      &app_info, -1);

  if (G_LIKELY (app_info != NULL))
    {
      /* determine the mime info for the file */
      content_type = fmb_file_get_content_type (chooser_button->file);

      /* try to set application as default for these kind of file */
      if (!g_app_info_set_as_default_for_type (app_info, content_type, &error))
        {
          /* tell the user that it didn't work */
          fmb_dialogs_show_error (GTK_WIDGET (chooser_button), error, 
                                     _("Failed to set default application for \"%s\""),
                                     fmb_file_get_display_name (chooser_button->file));
          g_error_free (error);
        }
      else
        {
          /* emit "changed" on the file, so everybody updates its state */
          fmb_file_changed (chooser_button->file);
        }

      /* release the application */
      g_object_unref (app_info);
    }
  else
    {
      /* no application was found in the store, looks like the other... option */
      fmb_chooser_button_chooser_dialog (chooser_button);
    }
}



static void
fmb_chooser_button_popup (FmbChooserButton *chooser_button)
{
  _fmb_return_if_fail (FMB_IS_CHOOSER_BUTTON (chooser_button));

  if (!chooser_button->has_default_application)
    {
      /* don't show the menu */
      gtk_combo_box_popdown (GTK_COMBO_BOX (chooser_button));

      /* open the chooser dialog if the filetype has no default action */
      fmb_chooser_button_chooser_dialog (chooser_button);
    }
}



static gint
fmb_chooser_button_sort_applications (gconstpointer a,
                                         gconstpointer b)
{
  _fmb_return_val_if_fail (G_IS_APP_INFO (a), -1);
  _fmb_return_val_if_fail (G_IS_APP_INFO (b), -1);

  return g_utf8_collate (g_app_info_get_name (G_APP_INFO (a)),
                         g_app_info_get_name (G_APP_INFO (b)));
}



static gboolean
fmb_chooser_button_row_separator (GtkTreeModel *model,
                                     GtkTreeIter  *iter,
                                     gpointer      data)
{
  gchar *name;

  /* determine the value of the "name" column */
  gtk_tree_model_get (model, iter, FMB_CHOOSER_BUTTON_STORE_COLUMN_NAME, &name, -1);
  if (G_LIKELY (name != NULL))
    {
      g_free (name);
      return FALSE;
    }

  return TRUE;
}



static void
fmb_chooser_button_chooser_dialog (FmbChooserButton *chooser_button)
{
  GtkWidget *toplevel;
  GtkWidget *dialog;

  _fmb_return_if_fail (FMB_IS_CHOOSER_BUTTON (chooser_button));

  /* determine the toplevel window for the chooser */
  toplevel = gtk_widget_get_toplevel (GTK_WIDGET (chooser_button));
  if (G_UNLIKELY (toplevel == NULL))
    return;

  /* popup the application chooser dialog */
  dialog = g_object_new (FMB_TYPE_CHOOSER_DIALOG, "open", FALSE, NULL);
  blxo_binding_new (G_OBJECT (chooser_button), "file", G_OBJECT (dialog), "file");
  gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (toplevel));
  gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
  if (gtk_dialog_run (GTK_DIALOG (dialog)) != GTK_RESPONSE_ACCEPT)
    fmb_chooser_button_file_changed (chooser_button, chooser_button->file);
  gtk_widget_destroy (dialog);
}



static void
fmb_chooser_button_file_changed (FmbChooserButton *chooser_button,
                                    FmbFile          *file)
{
  const gchar *content_type;
  GtkTreeIter  iter;
  GAppInfo    *app_info;
  GList       *app_infos;
  GList       *lp;
  gchar       *description;
  guint        i = 0;

  _fmb_return_if_fail (FMB_IS_CHOOSER_BUTTON (chooser_button));
  _fmb_return_if_fail (chooser_button->file == file);
  _fmb_return_if_fail (FMB_IS_FILE (file));

  /* clear the store */
  gtk_list_store_clear (chooser_button->store);

  /* reset the default application flag */
  chooser_button->has_default_application = FALSE;

  /* block the changed signal for a moment */
  g_signal_handlers_block_by_func (chooser_button,
                                   fmb_chooser_button_changed,
                                   NULL);

  /* determine the content type of the file */
  content_type = fmb_file_get_content_type (file);
  if (content_type != NULL)
    {
      /* setup a useful tooltip for the button */
      description = g_content_type_get_description (content_type);
      fmb_gtk_widget_set_tooltip (GTK_WIDGET (chooser_button),
                                     _("The selected application is used to open "
                                       "this and other files of type \"%s\"."),
                                     description);
      g_free (description);

      /* determine the default application for that content type */
      app_info = g_app_info_get_default_for_type (content_type, FALSE);
      if (G_LIKELY (app_info != NULL))
        {
          /* determine all applications that claim to be able to handle the file */
          app_infos = g_app_info_get_all_for_type (content_type);
          app_infos = g_list_sort (app_infos, fmb_chooser_button_sort_applications);
          
          /* add all possible applications */
          for (lp = app_infos, i = 0; lp != NULL; lp = lp->next, ++i)
            {
              if (fmb_g_app_info_should_show (lp->data))
                {
                  /* insert the item into the store */
                  gtk_list_store_insert_with_values (chooser_button->store, &iter, i,
                                                     FMB_CHOOSER_BUTTON_STORE_COLUMN_NAME,
                                                     g_app_info_get_name (lp->data),
                                                     FMB_CHOOSER_BUTTON_STORE_COLUMN_APPLICATION,
                                                     lp->data,
                                                     FMB_CHOOSER_BUTTON_STORE_COLUMN_ICON,
                                                     g_app_info_get_icon (lp->data),
                                                     FMB_CHOOSER_BUTTON_STORE_COLUMN_SENSITIVE,
                                                     TRUE,
                                                     -1);
                  
                  /* pre-select the default application */
                  if (g_app_info_equal (lp->data, app_info))
                    gtk_combo_box_set_active_iter (GTK_COMBO_BOX (chooser_button), &iter);
                }

              /* release the application */
              g_object_unref (lp->data);
            }

          /* release the application list */
          g_list_free (app_infos);

          /* release the default application */
          g_object_unref (app_info);

          /* assume we have some applications in the list */
          chooser_button->has_default_application = TRUE;
        }
    }
  
  if (content_type == NULL || !chooser_button->has_default_application)
    {
      /* add the "No application selected" item and set as active */
      gtk_list_store_insert_with_values (chooser_button->store, &iter, 0,
                                         FMB_CHOOSER_BUTTON_STORE_COLUMN_NAME,
                                         _("No application selected"),
                                         FMB_CHOOSER_BUTTON_STORE_COLUMN_STYLE,
                                         PANGO_STYLE_ITALIC,
                                         -1);
      gtk_combo_box_set_active_iter (GTK_COMBO_BOX (chooser_button), &iter);
    }

  /* insert empty row that will appear as a separator */
  gtk_list_store_insert_with_values (chooser_button->store, NULL, ++i, -1);

  /* add the "Other Application..." option */
  gtk_list_store_insert_with_values (chooser_button->store, NULL, ++i,
                                     FMB_CHOOSER_BUTTON_STORE_COLUMN_NAME,
                                     _("Other Application..."),
                                     FMB_CHOOSER_BUTTON_STORE_COLUMN_SENSITIVE,
                                     TRUE,
                                     -1);

  /* unblock the changed signal */
  g_signal_handlers_unblock_by_func (chooser_button,
                                     fmb_chooser_button_changed,
                                     NULL);
}



/**
 * fmb_chooser_button_new:
 *
 * Allocates a new #FmbChooserButton instance.
 *
 * Return value: the newly allocated #FmbChooserButton.
 **/
GtkWidget*
fmb_chooser_button_new (void)
{
  return g_object_new (FMB_TYPE_CHOOSER_BUTTON, NULL);
}



/**
 * fmb_chooser_button_set_file:
 * @chooser_button : a #FmbChooserButton instance.
 * @file           : a #FmbFile or %NULL.
 *
 * Associates @chooser_button with the specified @file.
 **/
void
fmb_chooser_button_set_file (FmbChooserButton *chooser_button,
                                FmbFile          *file)
{
  _fmb_return_if_fail (FMB_IS_CHOOSER_BUTTON (chooser_button));
  _fmb_return_if_fail (file == NULL || FMB_IS_FILE (file));

  /* check if we already use that file */
  if (G_UNLIKELY (chooser_button->file == file))
    return;

  /* disconnect from the previous file */
  if (G_UNLIKELY (chooser_button->file != NULL))
    {
      g_signal_handlers_disconnect_by_func (G_OBJECT (chooser_button->file), fmb_chooser_button_file_changed, chooser_button);
      g_object_unref (G_OBJECT (chooser_button->file));
    }

  /* activate the new file */
  chooser_button->file = file;

  /* connect to the new file */
  if (G_LIKELY (file != NULL))
    {
      /* take a reference */
      g_object_ref (G_OBJECT (file));

      /* stay informed about changes */
      g_signal_connect_swapped (G_OBJECT (file), "changed", G_CALLBACK (fmb_chooser_button_file_changed), chooser_button);

      /* update our state now */
      fmb_chooser_button_file_changed (chooser_button, file);
    }

  /* notify listeners */
  g_object_notify (G_OBJECT (chooser_button), "file");
}




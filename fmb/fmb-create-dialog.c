/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2005-2006 Benedikt Meurer <benny@xfce.org>
 * Copyright (c) 2009-2010 Jannis Pohlmann <jannis@xfce.org>
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

#include <fmb/fmb-abstract-dialog.h>
#include <fmb/fmb-create-dialog.h>
#include <fmb/fmb-dialogs.h>
#include <fmb/fmb-gobject-extensions.h>
#include <fmb/fmb-gtk-extensions.h>
#include <fmb/fmb-icon-factory.h>
#include <fmb/fmb-private.h>
#include <fmb/fmb-util.h>



/* Property identifiers */
enum
{
  PROP_0,
  PROP_FILENAME,
  PROP_CONTENT_TYPE,
};



static void         fmb_create_dialog_dispose          (GObject                 *object);
static void         fmb_create_dialog_get_property     (GObject                 *object,
                                                           guint                    prop_id,
                                                           GValue                  *value,
                                                           GParamSpec              *pspec);
static void         fmb_create_dialog_set_property     (GObject                 *object,
                                                           guint                    prop_id,
                                                           const GValue            *value,
                                                           GParamSpec              *pspec);
static void         fmb_create_dialog_realize          (GtkWidget               *widget);
static void         fmb_create_dialog_update_image     (FmbCreateDialog      *dialog);
static void         fmb_create_dialog_text_changed     (GtkWidget               *entry,
                                                           FmbCreateDialog      *dialog);
static const gchar *fmb_create_dialog_get_filename     (const FmbCreateDialog *dialog);
static void         fmb_create_dialog_set_filename     (FmbCreateDialog       *dialog,
                                                           const gchar              *filename);
static void         fmb_create_dialog_set_content_type (FmbCreateDialog       *dialog,
                                                           const gchar              *content_type);



struct _FmbCreateDialogClass
{
  FmbAbstractDialogClass __parent__;
};

struct _FmbCreateDialog
{
  FmbAbstractDialog __parent__;

  GtkWidget  *image;
  GtkWidget  *entry;

  gchar      *content_type;
};



G_DEFINE_TYPE (FmbCreateDialog, fmb_create_dialog, FMB_TYPE_ABSTRACT_DIALOG)



static void
fmb_create_dialog_class_init (FmbCreateDialogClass *klass)
{
  GtkWidgetClass *gtkwidget_class;
  GObjectClass   *gobject_class;

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->dispose = fmb_create_dialog_dispose;
  gobject_class->get_property = fmb_create_dialog_get_property;
  gobject_class->set_property = fmb_create_dialog_set_property;

  gtkwidget_class = GTK_WIDGET_CLASS (klass);
  gtkwidget_class->realize = fmb_create_dialog_realize;

  /**
   * FmbCreateDialog::filename:
   *
   * The filename entered in the dialog's entry box.
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_FILENAME,
                                   g_param_spec_string ("filename", 
                                                        "filename", 
                                                        "filename",
                                                        NULL,
                                                        BLXO_PARAM_READWRITE));

  /**
   * FmbCreateDialog::content-type:
   *
   * The content type of the file to create.
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_CONTENT_TYPE,
                                   g_param_spec_string ("content-type", 
                                                        "content-type", 
                                                        "content-type",
                                                       NULL,
                                                       BLXO_PARAM_READWRITE));
}



static void
fmb_create_dialog_init (FmbCreateDialog *dialog)
{
  GtkWidget *label;
  GtkWidget *table;

  dialog->content_type = NULL;

  /* configure the dialog itself */
  gtk_dialog_add_buttons (GTK_DIALOG (dialog),
                          GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                          _("C_reate"), GTK_RESPONSE_OK,
                          NULL);
  gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);
  gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog), GTK_RESPONSE_OK, FALSE);
  gtk_window_set_default_size (GTK_WINDOW (dialog), 300, -1);

  table = g_object_new (GTK_TYPE_TABLE, "border-width", 6, "column-spacing", 6, "row-spacing", 3, NULL);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), table, TRUE, TRUE, 0);
  gtk_widget_show (table);

  dialog->image = g_object_new (GTK_TYPE_IMAGE, "xpad", 6, "ypad", 6, NULL);
  gtk_table_attach (GTK_TABLE (table), dialog->image, 0, 1, 0, 2, GTK_FILL, GTK_FILL, 0, 0); 
  gtk_widget_show (dialog->image);

  label = g_object_new (GTK_TYPE_LABEL, "label", _("Enter the new name:"), "xalign", 0.0f, NULL);
  gtk_table_attach (GTK_TABLE (table), label, 1, 2, 0, 1, GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0);
  gtk_widget_show (label);

  dialog->entry = g_object_new (GTK_TYPE_ENTRY, "activates-default", TRUE, NULL);
  g_signal_connect (G_OBJECT (dialog->entry), "changed", G_CALLBACK (fmb_create_dialog_text_changed), dialog);
  gtk_table_attach (GTK_TABLE (table), dialog->entry, 1, 2, 1, 2, GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0);
  fmb_gtk_label_set_a11y_relation (GTK_LABEL (label), dialog->entry);
  gtk_widget_show (dialog->entry);
}



static void
fmb_create_dialog_dispose (GObject *object)
{
  FmbCreateDialog *dialog = FMB_CREATE_DIALOG (object);

  /* release the content type */
  fmb_create_dialog_set_content_type (dialog, NULL);

  (*G_OBJECT_CLASS (fmb_create_dialog_parent_class)->dispose) (object);
}



static void
fmb_create_dialog_get_property (GObject    *object,
                                   guint       prop_id,
                                   GValue     *value,
                                   GParamSpec *pspec)
{
  FmbCreateDialog *dialog = FMB_CREATE_DIALOG (object);

  switch (prop_id)
    {
    case PROP_FILENAME:
      g_value_set_string (value, fmb_create_dialog_get_filename (dialog));
      break;

    case PROP_CONTENT_TYPE:
      g_value_set_string (value, dialog->content_type);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static void
fmb_create_dialog_set_property (GObject      *object,
                                   guint         prop_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
  FmbCreateDialog *dialog = FMB_CREATE_DIALOG (object);

  switch (prop_id)
    {
    case PROP_FILENAME:
      fmb_create_dialog_set_filename (dialog, g_value_get_string (value));
      break;

    case PROP_CONTENT_TYPE:
      fmb_create_dialog_set_content_type (dialog, g_value_get_string (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static void
fmb_create_dialog_realize (GtkWidget *widget)
{
  FmbCreateDialog *dialog = FMB_CREATE_DIALOG (widget);

  /* let GtkWidget realize the widget */
  (*GTK_WIDGET_CLASS (fmb_create_dialog_parent_class)->realize) (widget);

  /* update the image */
  fmb_create_dialog_update_image (dialog);
}



static void
fmb_create_dialog_update_image (FmbCreateDialog *dialog)
{
  GIcon *icon = NULL;

  /* try to load the icon */
  if (G_LIKELY (dialog->content_type != NULL))
    icon = g_content_type_get_icon (dialog->content_type);

  /* setup the image */
  if (G_LIKELY (icon != NULL))
    {
      gtk_image_set_from_gicon (GTK_IMAGE (dialog->image), icon, GTK_ICON_SIZE_DIALOG);
      g_object_unref (icon);
      gtk_widget_show (dialog->image);
    }
  else
    gtk_widget_hide (dialog->image);
}



static void
fmb_create_dialog_text_changed (GtkWidget          *entry,
                                   FmbCreateDialog *dialog)
{
  const gchar *text;
  const gchar *p;

  _fmb_return_if_fail (GTK_IS_ENTRY (entry));
  _fmb_return_if_fail (FMB_IS_CREATE_DIALOG (dialog));
  _fmb_return_if_fail (dialog->entry == entry);

  /* verify the new text */
  text = gtk_entry_get_text (GTK_ENTRY (entry));
  for (p = text; *p != '\0'; ++p)
    if (G_UNLIKELY (G_IS_DIR_SEPARATOR (*p)))
      break;

  /* enable/disable the "OK" button appropriately */
  gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog), GTK_RESPONSE_OK, (*text != '\0' && *p == '\0'));
}



/**
 * fmb_create_dialog_get_filename:
 * @dialog : a #FmbCreateDialog.
 *
 * Returns the filename currently selected in @dialog.
 *
 * Return value: the filename currently selected in @dialog.
 **/
const gchar*
fmb_create_dialog_get_filename (const FmbCreateDialog *dialog)
{
  _fmb_return_val_if_fail (FMB_IS_CREATE_DIALOG (dialog), NULL);
  return gtk_entry_get_text (GTK_ENTRY (dialog->entry));
}



/**
 * fmb_create_dialog_set_filename:
 * @dialog   : a #FmbCreateDialog.
 * @filename : the new filename to set.
 *
 * Sets the filename currently selected in @dialog
 * to the specified @filename.
 **/
void
fmb_create_dialog_set_filename (FmbCreateDialog *dialog,
                                   const gchar        *filename)
{
  const gchar *dot;
  glong        offset;

  _fmb_return_if_fail (FMB_IS_CREATE_DIALOG (dialog));
  _fmb_return_if_fail (filename != NULL);
      
  /* setup the new filename */
  gtk_entry_set_text (GTK_ENTRY (dialog->entry), filename);

  /* check if filename contains a dot */
  dot = fmb_util_str_get_extension (filename);
  if (G_LIKELY (dot != NULL))
    {
      /* grab focus to the entry first, else
       * the selection will be altered later
       * when the focus is transferred.
       */
      gtk_widget_grab_focus (dialog->entry);

      /* determine the UTF-8 char offset */
      offset = g_utf8_pointer_to_offset (filename, dot);

      /* select the text prior to the dot */
      if (G_LIKELY (offset > 0))
        gtk_editable_select_region (GTK_EDITABLE (dialog->entry), 0, offset);
    }
  else
    {
      /* select the whole file name */
      gtk_editable_select_region (GTK_EDITABLE (dialog->entry), 0, -1);
    }

  /* notify listeners */
  g_object_notify (G_OBJECT (dialog), "filename");
}



/**
 * fmb_create_dialog_set_content_type:
 * @dialog       : a #FmbCreateDialog.
 * @content_type : the new content type.
 * 
 * Set the content type for @dialog to @content_type.
 **/
void
fmb_create_dialog_set_content_type (FmbCreateDialog *dialog,
                                       const gchar        *content_type)
{
  _fmb_return_if_fail (FMB_IS_CREATE_DIALOG (dialog));

  /* release the previous content type */
  g_free (dialog->content_type);

  /* set the new content type */
  dialog->content_type = g_strdup (content_type);

  /* update the image if we're already realized */
  if (gtk_widget_get_realized (GTK_WIDGET (dialog)))
    fmb_create_dialog_update_image (dialog);

  /* notify listeners */
  g_object_notify (G_OBJECT (dialog), "content-type");
}



/**
 * fmb_show_create_dialog:
 * @parent       : a #GdkScreen, a #GtkWidget or %NULL.
 * @content_type : the content type of the file or folder to create.
 * @filename     : the suggested filename or %NULL.
 * @title        : the dialog title.
 *
 * Constructs and display a #FmbCreateDialog with the specified
 * parameters that asks the user to enter a name for a new file or
 * folder.
 *
 * The caller is responsible to free the returned filename using
 * g_free() when no longer needed.
 *
 * Return value: the filename entered by the user or %NULL if the user
 *               cancelled the dialog.
 **/
gchar*
fmb_show_create_dialog (gpointer     parent,
                           const gchar *content_type,
                           const gchar *filename,
                           const gchar *title)
{
  GtkWidget *dialog;
  GtkWindow *window;
  GdkScreen *screen;
  GError    *error = NULL;
  gchar     *name = NULL;

  _fmb_return_val_if_fail (parent == NULL || GDK_IS_SCREEN (parent) || GTK_IS_WIDGET (parent), NULL);

  /* parse the parent window and screen */
  screen = fmb_util_parse_parent (parent, &window);

  /* display the create dialog */
  dialog = g_object_new (FMB_TYPE_CREATE_DIALOG,
                         "destroy-with-parent", TRUE,
                         "filename", filename,
                         "content-type", content_type,
                         "modal", TRUE,
                         "title", title,
                         NULL);

  if (screen != NULL)
    gtk_window_set_screen (GTK_WINDOW (dialog), screen);

  if (window != NULL)
    gtk_window_set_transient_for (GTK_WINDOW (dialog), window);

  if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_OK)
    {
      /* determine the chosen filename */
      filename = fmb_create_dialog_get_filename (FMB_CREATE_DIALOG (dialog));

      /* convert the UTF-8 filename to the local file system encoding */
      name = g_filename_from_utf8 (filename, -1, NULL, NULL, &error);
      if (G_UNLIKELY (name == NULL))
        {
          /* display an error message */
          fmb_dialogs_show_error (dialog, error, _("Cannot convert filename \"%s\" to the local encoding"), filename);

          /* release the error */
          g_error_free (error);
        }
    }

  /* destroy the dialog */
  gtk_widget_destroy (dialog);

  return name;
}





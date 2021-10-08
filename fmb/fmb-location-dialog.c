/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2005-2006 Benedikt Meurer <benny@xfce.org>
 * Copyright (c) 2010 Jannis Pohlmann <jannis@xfce.org>
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

#include <fmb/fmb-gtk-extensions.h>
#include <fmb/fmb-location-dialog.h>
#include <fmb/fmb-path-entry.h>
#include <fmb/fmb-private.h>



G_DEFINE_TYPE (FmbLocationDialog, fmb_location_dialog, FMB_TYPE_ABSTRACT_DIALOG)



static gboolean
transform_object_to_boolean (const GValue *src_value,
                             GValue       *dst_value,
                             gpointer      user_data)
{
  g_value_set_boolean (dst_value, (g_value_get_object (src_value) != NULL));
  return TRUE;
}



static void
fmb_location_dialog_class_init (FmbLocationDialogClass *klass)
{
}



static void
fmb_location_dialog_init (FmbLocationDialog *location_dialog)
{
  GtkWidget *open_button;
  GtkWidget *hbox;
  GtkWidget *label;

  gtk_box_set_spacing (GTK_BOX (GTK_DIALOG (location_dialog)->vbox), 2);
  gtk_container_set_border_width (GTK_CONTAINER (location_dialog), 5);
  gtk_window_set_default_size (GTK_WINDOW (location_dialog), 350, -1);
  gtk_window_set_title (GTK_WINDOW (location_dialog), _("Open Location"));

  gtk_dialog_add_button (GTK_DIALOG (location_dialog), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);

  open_button = gtk_dialog_add_button (GTK_DIALOG (location_dialog), GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT);
  gtk_window_set_default (GTK_WINDOW (location_dialog), open_button);

  hbox = g_object_new (GTK_TYPE_HBOX,
                       "border-width", 5,
                       "spacing", 12,
                       NULL);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (location_dialog)->vbox), hbox, FALSE, FALSE, 0);
  gtk_widget_show (hbox);

  label = gtk_label_new_with_mnemonic (_("_Location:"));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  location_dialog->entry = fmb_path_entry_new ();
  gtk_entry_set_activates_default (GTK_ENTRY (location_dialog->entry), TRUE);
  gtk_box_pack_start (GTK_BOX (hbox), location_dialog->entry, TRUE, TRUE, 0);
  fmb_gtk_label_set_a11y_relation (GTK_LABEL (label), location_dialog->entry);
  gtk_label_set_mnemonic_widget (GTK_LABEL (label), location_dialog->entry);
  gtk_widget_show (location_dialog->entry);

  /* the "Open" button is only sensitive if a valid file is entered */
  blxo_binding_new_full (G_OBJECT (location_dialog->entry), "current-file",
                        G_OBJECT (open_button), "sensitive",
                        transform_object_to_boolean, NULL, NULL);
}



/**
 * fmb_location_dialog_new:
 * 
 * Allocates a new #FmbLocationDialog instance.
 *
 * Return value: the newly allocated #FmbLocationDialog.
 **/
GtkWidget*
fmb_location_dialog_new (void)
{
  return g_object_new (FMB_TYPE_LOCATION_DIALOG, NULL);
}



/**
 * fmb_location_dialog_get_selected_file:
 * @location_dialog : a #FmbLocationDialog.
 *
 * Returns the file selected for the @dialog or
 * %NULL if the file entered is not valid.
 *
 * Return value: the selected #FmbFile or %NULL.
 **/
FmbFile*
fmb_location_dialog_get_selected_file (FmbLocationDialog *location_dialog)
{
  _fmb_return_val_if_fail (FMB_IS_LOCATION_DIALOG (location_dialog), NULL);

  return fmb_path_entry_get_current_file (FMB_PATH_ENTRY (location_dialog->entry));
}



/**
 * fmb_location_dialog_set_selected_file:
 * @location_dialog : a #FmbLocationDialog.
 * @selected_file   : a #FmbFile or %NULL.
 *
 * Sets the file for @location_dialog to @selected_file.
 **/
void
fmb_location_dialog_set_selected_file (FmbLocationDialog *location_dialog,
                                          FmbFile           *selected_file)
{
  _fmb_return_if_fail (FMB_IS_LOCATION_DIALOG (location_dialog));
  _fmb_return_if_fail (selected_file == NULL || FMB_IS_FILE (selected_file));

  fmb_path_entry_set_current_file (FMB_PATH_ENTRY (location_dialog->entry), 
                                      selected_file);
}


/**
 * fmb_location_dialog_set_working_directory:
 * @location_dialog : a #FmbLocationDialog.
 * @directory       : a #FmbFile or %NULL.
 *
 * Sets the working directory of @location_dialog to @directory.
 **/
void
fmb_location_dialog_set_working_directory (FmbLocationDialog *location_dialog,
                                              FmbFile           *directory)
{
  _fmb_return_if_fail (FMB_IS_LOCATION_DIALOG (location_dialog));
  _fmb_return_if_fail (directory == NULL || FMB_IS_FILE (directory));

  fmb_path_entry_set_working_directory (FMB_PATH_ENTRY (location_dialog->entry),
                                           directory);
}

/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2005-2007 Benedikt Meurer <benny@xfce.org>
 * Copyright (c) 2009-2011 Jannis Pohlmann <jannis@xfce.org>
 * Copyright (c) 2012      Nick Schermer <nick@xfce.org>
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

#ifdef HAVE_MEMORY_H
#include <memory.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include <gdk/gdkkeysyms.h>

#include <blxo/blxo.h>
#include <libbladeui/libbladeui.h>

#include <fmb/fmb-abstract-dialog.h>
#include <fmb/fmb-application.h>
#include <fmb/fmb-chooser-button.h>
#include <fmb/fmb-dialogs.h>
#include <fmb/fmb-emblem-chooser.h>
#include <fmb/fmb-gio-extensions.h>
#include <fmb/fmb-gobject-extensions.h>
#include <fmb/fmb-gtk-extensions.h>
#include <fmb/fmb-icon-factory.h>
#include <fmb/fmb-image.h>
#include <fmb/fmb-io-jobs.h>
#include <fmb/fmb-job.h>
#include <fmb/fmb-marshal.h>
#include <fmb/fmb-pango-extensions.h>
#include <fmb/fmb-permissions-chooser.h>
#include <fmb/fmb-preferences.h>
#include <fmb/fmb-private.h>
#include <fmb/fmb-properties-dialog.h>
#include <fmb/fmb-size-label.h>
#include <fmb/fmb-thumbnailer.h>
#include <fmb/fmb-util.h>



/* Property identifiers */
enum
{
  PROP_0,
  PROP_FILES,
  PROP_FILE_SIZE_BINARY,
};

/* Signal identifiers */
enum
{
  RELOAD,
  LAST_SIGNAL,
};



static void     fmb_properties_dialog_dispose              (GObject                     *object);
static void     fmb_properties_dialog_finalize             (GObject                     *object);
static void     fmb_properties_dialog_get_property         (GObject                     *object,
                                                               guint                        prop_id,
                                                               GValue                      *value,
                                                               GParamSpec                  *pspec);
static void     fmb_properties_dialog_set_property         (GObject                     *object,
                                                               guint                        prop_id,
                                                               const GValue                *value,
                                                               GParamSpec                  *pspec);
static void     fmb_properties_dialog_response             (GtkDialog                   *dialog,
                                                               gint                         response);
static gboolean fmb_properties_dialog_reload               (FmbPropertiesDialog      *dialog);
static void     fmb_properties_dialog_name_activate        (GtkWidget                   *entry,
                                                               FmbPropertiesDialog      *dialog);
static gboolean fmb_properties_dialog_name_focus_out_event (GtkWidget                   *entry,
                                                               GdkEventFocus               *event,
                                                               FmbPropertiesDialog      *dialog);
static void     fmb_properties_dialog_icon_button_clicked  (GtkWidget                   *button,
                                                               FmbPropertiesDialog      *dialog);
static void     fmb_properties_dialog_update               (FmbPropertiesDialog      *dialog);
static void     fmb_properties_dialog_update_providers     (FmbPropertiesDialog      *dialog);
static GList   *fmb_properties_dialog_get_files            (FmbPropertiesDialog      *dialog);


struct _FmbPropertiesDialogClass
{
  FmbAbstractDialogClass __parent__;

  /* signals */
  gboolean (*reload) (FmbPropertiesDialog *dialog);
};

struct _FmbPropertiesDialog
{
  FmbAbstractDialog    __parent__;

  FmbxProviderFactory *provider_factory;
  GList                  *provider_pages;

  FmbPreferences      *preferences;

  GList                  *files;
  gboolean                file_size_binary;

  FmbThumbnailer      *thumbnailer;
  guint                   thumbnail_request;

  GtkWidget              *notebook;
  GtkWidget              *icon_button;
  GtkWidget              *icon_image;
  GtkWidget              *name_entry;
  GtkWidget              *names_label;
  GtkWidget              *single_box;
  GtkWidget              *kind_ebox;
  GtkWidget              *kind_label;
  GtkWidget              *openwith_chooser;
  GtkWidget              *link_label;
  GtkWidget              *location_label;
  GtkWidget              *origin_label;
  GtkWidget              *deleted_label;
  GtkWidget              *modified_label;
  GtkWidget              *accessed_label;
  GtkWidget              *freespace_vbox;
  GtkWidget              *freespace_bar;
  GtkWidget              *freespace_label;
  GtkWidget              *volume_image;
  GtkWidget              *volume_label;
  GtkWidget              *permissions_chooser;
};



G_DEFINE_TYPE (FmbPropertiesDialog, fmb_properties_dialog, FMB_TYPE_ABSTRACT_DIALOG)



static void
fmb_properties_dialog_class_init (FmbPropertiesDialogClass *klass)
{
  GtkDialogClass *gtkdialog_class;
  GtkBindingSet  *binding_set;
  GObjectClass   *gobject_class;

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->dispose = fmb_properties_dialog_dispose;
  gobject_class->finalize = fmb_properties_dialog_finalize;
  gobject_class->get_property = fmb_properties_dialog_get_property;
  gobject_class->set_property = fmb_properties_dialog_set_property;

  gtkdialog_class = GTK_DIALOG_CLASS (klass);
  gtkdialog_class->response = fmb_properties_dialog_response;

  klass->reload = fmb_properties_dialog_reload;

  /**
   * FmbPropertiesDialog:files:
   *
   * The list of currently selected files whose properties are displayed by
   * this #FmbPropertiesDialog. This property may also be %NULL
   * in which case nothing is displayed.
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_FILES,
                                   g_param_spec_boxed ("files", "files", "files",
                                                        FMBX_TYPE_FILE_INFO_LIST,
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

  /**
   * FmbPropertiesDialog::reload:
   * @dialog : a #FmbPropertiesDialog.
   *
   * Emitted whenever the user requests reset the reload the
   * file properties. This is an internal signal used to bind
   * the action to keys.
   **/
  g_signal_new (I_("reload"),
                G_TYPE_FROM_CLASS (klass),
                G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                G_STRUCT_OFFSET (FmbPropertiesDialogClass, reload),
                g_signal_accumulator_true_handled, NULL,
                _fmb_marshal_BOOLEAN__VOID,
                G_TYPE_BOOLEAN, 0);

  /* setup the key bindings for the properties dialog */
  binding_set = gtk_binding_set_by_class (klass);
  gtk_binding_entry_add_signal (binding_set, GDK_F5, 0, "reload", 0);
  gtk_binding_entry_add_signal (binding_set, GDK_r, GDK_CONTROL_MASK, "reload", 0);
}



static void
fmb_properties_dialog_init (FmbPropertiesDialog *dialog)
{
  GtkWidget *chooser;
  GtkWidget *table;
  GtkWidget *label;
  GtkWidget *box;
  GtkWidget *spacer;
  guint      row = 0;
  GtkWidget *image;

  /* acquire a reference on the preferences and monitor the
     "misc-date-style" and "misc-file-size-binary" settings */
  dialog->preferences = fmb_preferences_get ();
  g_signal_connect_swapped (G_OBJECT (dialog->preferences), "notify::misc-date-style",
                            G_CALLBACK (fmb_properties_dialog_reload), dialog);
  blxo_binding_new (G_OBJECT (dialog->preferences), "misc-file-size-binary",
                   G_OBJECT (dialog), "file-size-binary");
  g_signal_connect_swapped (G_OBJECT (dialog->preferences), "notify::misc-file-size-binary",
                            G_CALLBACK (fmb_properties_dialog_reload), dialog);

  /* create a new thumbnailer */
  dialog->thumbnailer = fmb_thumbnailer_get ();
  dialog->thumbnail_request = 0;

  dialog->provider_factory = fmbx_provider_factory_get_default ();

  gtk_dialog_add_buttons (GTK_DIALOG (dialog),
                          GTK_STOCK_HELP, GTK_RESPONSE_HELP,
                          GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
                          NULL);
  gtk_window_set_default_size (GTK_WINDOW (dialog), 500, 550);

  dialog->notebook = gtk_notebook_new ();
  gtk_container_set_border_width (GTK_CONTAINER (dialog->notebook), 6);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), dialog->notebook, TRUE, TRUE, 0);
  gtk_widget_show (dialog->notebook);

  table = gtk_table_new (16, 2, FALSE);
  label = gtk_label_new (_("General"));
  gtk_table_set_col_spacings (GTK_TABLE (table), 12);
  gtk_container_set_border_width (GTK_CONTAINER (table), 6);
  gtk_notebook_append_page (GTK_NOTEBOOK (dialog->notebook), table, label);
  gtk_widget_show (label);
  gtk_widget_show (table);


  /*
     First box (icon, name) for 1 file
   */
  dialog->single_box = gtk_hbox_new (FALSE, 6);
  gtk_table_attach (GTK_TABLE (table), dialog->single_box, 0, 1, row, row + 1, GTK_FILL, GTK_FILL, 0, 3);

  dialog->icon_button = gtk_button_new ();
  g_signal_connect (G_OBJECT (dialog->icon_button), "clicked", G_CALLBACK (fmb_properties_dialog_icon_button_clicked), dialog);
  gtk_box_pack_start (GTK_BOX (dialog->single_box), dialog->icon_button, FALSE, TRUE, 0);
  gtk_widget_show (dialog->icon_button);

  dialog->icon_image = fmb_image_new ();
  gtk_box_pack_start (GTK_BOX (dialog->single_box), dialog->icon_image, FALSE, TRUE, 0);
  gtk_widget_show (dialog->icon_image);

  label = gtk_label_new_with_mnemonic (_("_Name:"));
  gtk_label_set_attributes (GTK_LABEL (label), fmb_pango_attr_list_bold ());
  gtk_misc_set_alignment (GTK_MISC (label), 1.0f, 0.5f);
  gtk_box_pack_end (GTK_BOX (dialog->single_box), label, TRUE, TRUE, 0);
  gtk_widget_show (label);

  dialog->name_entry = g_object_new (GTK_TYPE_ENTRY, "editable", FALSE, NULL);
  gtk_label_set_mnemonic_widget (GTK_LABEL (label), dialog->name_entry);
  g_signal_connect (G_OBJECT (dialog->name_entry), "activate", G_CALLBACK (fmb_properties_dialog_name_activate), dialog);
  g_signal_connect (G_OBJECT (dialog->name_entry), "focus-out-event", G_CALLBACK (fmb_properties_dialog_name_focus_out_event), dialog);
  gtk_table_attach (GTK_TABLE (table), dialog->name_entry, 1, 2, row, row + 1, GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 3);
  blxo_binding_new (G_OBJECT (dialog->single_box), "visible", G_OBJECT (dialog->name_entry), "visible");

  ++row;


  /*
     First box (icon, name) for multiple files
   */
  box = gtk_hbox_new (FALSE, 6);
  gtk_table_attach (GTK_TABLE (table), box, 0, 1, row, row + 1, GTK_FILL, GTK_FILL, 0, 3);
  blxo_binding_new_with_negation (G_OBJECT (dialog->single_box), "visible", G_OBJECT (box), "visible");

  image = gtk_image_new_from_icon_name ("text-x-generic", GTK_ICON_SIZE_DIALOG);
  gtk_box_pack_start (GTK_BOX (box), image, FALSE, TRUE, 0);
  gtk_widget_show (image);

  label = gtk_label_new (_("Names:"));
  gtk_label_set_attributes (GTK_LABEL (label), fmb_pango_attr_list_bold ());
  gtk_misc_set_alignment (GTK_MISC (label), 1.0f, 0.5f);
  gtk_box_pack_end (GTK_BOX (box), label, TRUE, TRUE, 0);
  gtk_widget_show (label);

  dialog->names_label = gtk_label_new ("");
  gtk_misc_set_alignment (GTK_MISC (dialog->names_label), 0.0f, 0.5f);
  gtk_table_attach (GTK_TABLE (table), dialog->names_label, 1, 2, row, row + 1, GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 3);
  gtk_label_set_ellipsize (GTK_LABEL (dialog->names_label), PANGO_ELLIPSIZE_END);
  gtk_label_set_selectable (GTK_LABEL (dialog->names_label), TRUE);
  blxo_binding_new (G_OBJECT (box), "visible", G_OBJECT (dialog->names_label), "visible");


  ++row;


  /*
     Second box (kind, open with, link target)
   */
  label = gtk_label_new (_("Kind:"));
  gtk_label_set_attributes (GTK_LABEL (label), fmb_pango_attr_list_bold ());
  gtk_misc_set_alignment (GTK_MISC (label), 1.0f, 0.5f);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row + 1, GTK_FILL, GTK_FILL, 0, 3);
  gtk_widget_show (label);

  dialog->kind_ebox = gtk_event_box_new ();
  gtk_event_box_set_above_child (GTK_EVENT_BOX (dialog->kind_ebox), TRUE);
  gtk_event_box_set_visible_window (GTK_EVENT_BOX (dialog->kind_ebox), FALSE);
  blxo_binding_new (G_OBJECT (dialog->kind_ebox), "visible", G_OBJECT (label), "visible");
  gtk_table_attach (GTK_TABLE (table), dialog->kind_ebox, 1, 2, row, row + 1, GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 3);
  gtk_widget_show (dialog->kind_ebox);

  dialog->kind_label = g_object_new (GTK_TYPE_LABEL, "xalign", 0.0f, NULL);
  gtk_label_set_selectable (GTK_LABEL (dialog->kind_label), TRUE);
  gtk_label_set_ellipsize (GTK_LABEL (dialog->kind_label), PANGO_ELLIPSIZE_END);
  gtk_container_add (GTK_CONTAINER (dialog->kind_ebox), dialog->kind_label);
  gtk_widget_show (dialog->kind_label);

  ++row;

  label = gtk_label_new_with_mnemonic (_("_Open With:"));
  gtk_label_set_attributes (GTK_LABEL (label), fmb_pango_attr_list_bold ());
  gtk_misc_set_alignment (GTK_MISC (label), 1.0f, 0.5f);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row + 1, GTK_FILL, GTK_FILL, 0, 3);
  gtk_widget_show (label);

  dialog->openwith_chooser = fmb_chooser_button_new ();
  gtk_label_set_mnemonic_widget (GTK_LABEL (label), dialog->openwith_chooser);
  blxo_binding_new (G_OBJECT (dialog->openwith_chooser), "visible", G_OBJECT (label), "visible");
  gtk_table_attach (GTK_TABLE (table), dialog->openwith_chooser, 1, 2, row, row + 1, GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 3);
  gtk_widget_show (dialog->openwith_chooser);

  ++row;

  label = gtk_label_new (_("Link Target:"));
  gtk_label_set_attributes (GTK_LABEL (label), fmb_pango_attr_list_bold ());
  gtk_misc_set_alignment (GTK_MISC (label), 1.0f, 0.5f);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row + 1, GTK_FILL, GTK_FILL, 0, 3);
  gtk_widget_show (label);

  dialog->link_label = g_object_new (GTK_TYPE_LABEL, "ellipsize", PANGO_ELLIPSIZE_START, "xalign", 0.0f, NULL);
  gtk_label_set_selectable (GTK_LABEL (dialog->link_label), TRUE);
  blxo_binding_new (G_OBJECT (dialog->link_label), "visible", G_OBJECT (label), "visible");
  gtk_table_attach (GTK_TABLE (table), dialog->link_label, 1, 2, row, row + 1, GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 3);
  gtk_widget_show (dialog->link_label);

  ++row;

  /* TRANSLATORS: Try to come up with a short translation of "Original Path" (which is the path
   * where the trashed file/folder was located before it was moved to the trash), otherwise the
   * properties dialog width will be messed up.
   */
  label = gtk_label_new (_("Original Path:"));
  gtk_label_set_attributes (GTK_LABEL (label), fmb_pango_attr_list_bold ());
  gtk_misc_set_alignment (GTK_MISC (label), 1.0f, 0.5f);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row + 1, GTK_FILL, GTK_FILL, 0, 3);
  gtk_widget_show (label);

  dialog->origin_label = g_object_new (GTK_TYPE_LABEL, "ellipsize", PANGO_ELLIPSIZE_START, "xalign", 0.0f, NULL);
  gtk_label_set_selectable (GTK_LABEL (dialog->origin_label), TRUE);
  blxo_binding_new (G_OBJECT (dialog->origin_label), "visible", G_OBJECT (label), "visible");
  gtk_table_attach (GTK_TABLE (table), dialog->origin_label, 1, 2, row, row + 1, GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 3);
  gtk_widget_show (dialog->origin_label);

  ++row;

  label = gtk_label_new (_("Location:"));
  gtk_label_set_attributes (GTK_LABEL (label), fmb_pango_attr_list_bold ());
  gtk_misc_set_alignment (GTK_MISC (label), 1.0f, 0.5f);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row + 1, GTK_FILL, GTK_FILL, 0, 3);
  gtk_widget_show (label);

  dialog->location_label = g_object_new (GTK_TYPE_LABEL, "ellipsize", PANGO_ELLIPSIZE_START, "xalign", 0.0f, NULL);
  gtk_label_set_selectable (GTK_LABEL (dialog->location_label), TRUE);
  blxo_binding_new (G_OBJECT (dialog->location_label), "visible", G_OBJECT (label), "visible");
  gtk_table_attach (GTK_TABLE (table), dialog->location_label, 1, 2, row, row + 1, GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 3);
  gtk_widget_show (dialog->location_label);

  ++row;


  spacer = g_object_new (GTK_TYPE_ALIGNMENT, "height-request", 12, NULL);
  gtk_table_attach (GTK_TABLE (table), spacer, 0, 2, row, row + 1, GTK_FILL, GTK_FILL, 0, 3);
  gtk_widget_show (spacer);

  ++row;


  /*
     Third box (deleted, modified, accessed)
   */
  label = gtk_label_new (_("Deleted:"));
  gtk_label_set_attributes (GTK_LABEL (label), fmb_pango_attr_list_bold ());
  gtk_misc_set_alignment (GTK_MISC (label), 1.0f, 0.5f);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row + 1, GTK_FILL, GTK_FILL, 0, 3);
  gtk_widget_show (label);

  dialog->deleted_label = g_object_new (GTK_TYPE_LABEL, "xalign", 0.0f, NULL);
  gtk_label_set_selectable (GTK_LABEL (dialog->deleted_label), TRUE);
  blxo_binding_new (G_OBJECT (dialog->deleted_label), "visible", G_OBJECT (label), "visible");
  gtk_table_attach (GTK_TABLE (table), dialog->deleted_label, 1, 2, row, row + 1, GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 3);
  gtk_widget_show (dialog->deleted_label);

  ++row;

  label = gtk_label_new (_("Modified:"));
  gtk_label_set_attributes (GTK_LABEL (label), fmb_pango_attr_list_bold ());
  gtk_misc_set_alignment (GTK_MISC (label), 1.0f, 0.5f);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row + 1, GTK_FILL, GTK_FILL, 0, 3);
  gtk_widget_show (label);

  dialog->modified_label = g_object_new (GTK_TYPE_LABEL, "xalign", 0.0f, NULL);
  gtk_label_set_selectable (GTK_LABEL (dialog->modified_label), TRUE);
  blxo_binding_new (G_OBJECT (dialog->modified_label), "visible", G_OBJECT (label), "visible");
  gtk_table_attach (GTK_TABLE (table), dialog->modified_label, 1, 2, row, row + 1, GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 3);
  gtk_widget_show (dialog->modified_label);

  ++row;

  label = gtk_label_new (_("Accessed:"));
  gtk_label_set_attributes (GTK_LABEL (label), fmb_pango_attr_list_bold ());
  gtk_misc_set_alignment (GTK_MISC (label), 1.0f, 0.5f);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row + 1, GTK_FILL, GTK_FILL, 0, 3);
  gtk_widget_show (label);

  dialog->accessed_label = g_object_new (GTK_TYPE_LABEL, "xalign", 0.0f, NULL);
  gtk_label_set_selectable (GTK_LABEL (dialog->accessed_label), TRUE);
  blxo_binding_new (G_OBJECT (dialog->accessed_label), "visible", G_OBJECT (label), "visible");
  gtk_table_attach (GTK_TABLE (table), dialog->accessed_label, 1, 2, row, row + 1, GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 3);
  gtk_widget_show (dialog->accessed_label);

  ++row;


  spacer = g_object_new (GTK_TYPE_ALIGNMENT, "height-request", 12, NULL);
  gtk_table_attach (GTK_TABLE (table), spacer, 0, 2, row, row + 1, GTK_FILL, GTK_FILL, 0, 3);
  blxo_binding_new (G_OBJECT (dialog->accessed_label), "visible", G_OBJECT (spacer), "visible");

  ++row;


  /*
     Fourth box (size, volume, free space)
   */
  label = gtk_label_new (_("Size:"));
  gtk_label_set_attributes (GTK_LABEL (label), fmb_pango_attr_list_bold ());
  gtk_misc_set_alignment (GTK_MISC (label), 1.0f, 0.5f);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row + 1, GTK_FILL, GTK_FILL, 0, 3);
  gtk_widget_show (label);

  label = fmb_size_label_new ();
  blxo_binding_new (G_OBJECT (dialog), "files", G_OBJECT (label), "files");
  gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row + 1, GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 3);
  gtk_widget_show (label);

  ++row;

  label = gtk_label_new (_("Volume:"));
  gtk_label_set_attributes (GTK_LABEL (label), fmb_pango_attr_list_bold ());
  gtk_misc_set_alignment (GTK_MISC (label), 1.0f, 0.5f);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row + 1, GTK_FILL, GTK_FILL, 0, 3);
  gtk_widget_show (label);

  box = gtk_hbox_new (FALSE, 6);
  blxo_binding_new (G_OBJECT (box), "visible", G_OBJECT (label), "visible");
  gtk_table_attach (GTK_TABLE (table), box, 1, 2, row, row + 1, GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 3);
  gtk_widget_show (box);

  dialog->volume_image = gtk_image_new ();
  blxo_binding_new (G_OBJECT (dialog->volume_image), "visible", G_OBJECT (box), "visible");
  gtk_box_pack_start (GTK_BOX (box), dialog->volume_image, FALSE, TRUE, 0);
  gtk_widget_show (dialog->volume_image);

  dialog->volume_label = g_object_new (GTK_TYPE_LABEL, "xalign", 0.0f, NULL);
  gtk_label_set_selectable (GTK_LABEL (dialog->volume_label), TRUE);
  blxo_binding_new (G_OBJECT (dialog->volume_label), "visible", G_OBJECT (dialog->volume_image), "visible");
  gtk_box_pack_start (GTK_BOX (box), dialog->volume_label, TRUE, TRUE, 0);
  gtk_widget_show (dialog->volume_label);

  ++row;

  label = gtk_label_new (_("Usage:"));
  gtk_label_set_attributes (GTK_LABEL (label), fmb_pango_attr_list_bold ());
  gtk_misc_set_alignment (GTK_MISC (label), 1.0f, 0.0f);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row + 1, GTK_FILL, GTK_FILL, 0, 3);
  gtk_widget_show (label);

  dialog->freespace_vbox = gtk_vbox_new (FALSE, 4);
  gtk_table_attach (GTK_TABLE (table), dialog->freespace_vbox, 1, 2, row, row + 1, GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 3);
  blxo_binding_new (G_OBJECT (dialog->freespace_vbox), "visible", G_OBJECT (label), "visible");
  gtk_widget_show (dialog->freespace_vbox);

  dialog->freespace_label = g_object_new (GTK_TYPE_LABEL, "xalign", 0.0f, NULL);
  gtk_label_set_selectable (GTK_LABEL (dialog->freespace_label), TRUE);
  gtk_box_pack_start (GTK_BOX (dialog->freespace_vbox), dialog->freespace_label, TRUE, TRUE, 0);
  gtk_widget_show (dialog->freespace_label);

  dialog->freespace_bar = g_object_new (GTK_TYPE_PROGRESS_BAR, NULL);
  gtk_box_pack_start (GTK_BOX (dialog->freespace_vbox), dialog->freespace_bar, TRUE, TRUE, 0);
  gtk_widget_set_size_request (dialog->freespace_bar, -1, 10);
  gtk_widget_show (dialog->freespace_bar);

  ++row;

  spacer = g_object_new (GTK_TYPE_ALIGNMENT, "height-request", 12, NULL);
  gtk_table_attach (GTK_TABLE (table), spacer, 0, 2, row, row + 1, GTK_FILL, GTK_FILL, 0, 3);
  gtk_widget_show (spacer);

  ++row;


  /*
     Emblem chooser
   */
  label = gtk_label_new (_("Emblems"));
  chooser = fmb_emblem_chooser_new ();
  blxo_binding_new (G_OBJECT (dialog), "files", G_OBJECT (chooser), "files");
  gtk_notebook_append_page (GTK_NOTEBOOK (dialog->notebook), chooser, label);
  gtk_widget_show (chooser);
  gtk_widget_show (label);

  /*
     Permissions chooser
   */
  label = gtk_label_new (_("Permissions"));
  dialog->permissions_chooser = fmb_permissions_chooser_new ();
  blxo_binding_new (G_OBJECT (dialog), "files", G_OBJECT (dialog->permissions_chooser), "files");
  gtk_notebook_append_page (GTK_NOTEBOOK (dialog->notebook), dialog->permissions_chooser, label);
  gtk_widget_show (dialog->permissions_chooser);
  gtk_widget_show (label);
}



static void
fmb_properties_dialog_dispose (GObject *object)
{
  FmbPropertiesDialog *dialog = FMB_PROPERTIES_DIALOG (object);

  /* reset the file displayed by the dialog */
  fmb_properties_dialog_set_files (dialog, NULL);

  (*G_OBJECT_CLASS (fmb_properties_dialog_parent_class)->dispose) (object);
}



static void
fmb_properties_dialog_finalize (GObject *object)
{
  FmbPropertiesDialog *dialog = FMB_PROPERTIES_DIALOG (object);

  _fmb_return_if_fail (dialog->files == NULL);

  /* disconnect from the preferences */
  g_signal_handlers_disconnect_by_func (dialog->preferences, fmb_properties_dialog_reload, dialog);
  g_object_unref (dialog->preferences);

  /* cancel any pending thumbnailer requests */
  if (dialog->thumbnail_request > 0)
    {
      fmb_thumbnailer_dequeue (dialog->thumbnailer, dialog->thumbnail_request);
      dialog->thumbnail_request = 0;
    }

  /* release the thumbnailer */
  g_object_unref (dialog->thumbnailer);

  /* release the provider property pages */
  g_list_free_full (dialog->provider_pages, g_object_unref);

  /* drop the reference on the provider factory */
  g_object_unref (dialog->provider_factory);

  (*G_OBJECT_CLASS (fmb_properties_dialog_parent_class)->finalize) (object);
}



static void
fmb_properties_dialog_get_property (GObject    *object,
                                       guint       prop_id,
                                       GValue     *value,
                                       GParamSpec *pspec)
{
  FmbPropertiesDialog *dialog = FMB_PROPERTIES_DIALOG (object);

  switch (prop_id)
    {
    case PROP_FILES:
      g_value_set_boxed (value, fmb_properties_dialog_get_files (dialog));
      break;

    case PROP_FILE_SIZE_BINARY:
      g_value_set_boolean (value, dialog->file_size_binary);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static void
fmb_properties_dialog_set_property (GObject      *object,
                                       guint         prop_id,
                                       const GValue *value,
                                       GParamSpec   *pspec)
{
  FmbPropertiesDialog *dialog = FMB_PROPERTIES_DIALOG (object);

  switch (prop_id)
    {
    case PROP_FILES:
      fmb_properties_dialog_set_files (dialog, g_value_get_boxed (value));
      break;

    case PROP_FILE_SIZE_BINARY:
      dialog->file_size_binary = g_value_get_boolean (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static void
fmb_properties_dialog_response (GtkDialog *dialog,
                                   gint       response)
{
  if (response == GTK_RESPONSE_CLOSE)
    {
      gtk_widget_destroy (GTK_WIDGET (dialog));
    }
  else if (response == GTK_RESPONSE_HELP)
    {
      xfce_dialog_show_help (GTK_WINDOW (dialog), "fmb",
                             "working-with-files-and-folders",
                             "file_properties");
    }
  else if (GTK_DIALOG_CLASS (fmb_properties_dialog_parent_class)->response != NULL)
    {
      (*GTK_DIALOG_CLASS (fmb_properties_dialog_parent_class)->response) (dialog, response);
    }
}



static gboolean
fmb_properties_dialog_reload (FmbPropertiesDialog *dialog)
{
  /* reload the active files */
  g_list_foreach (dialog->files, (GFunc) fmb_file_reload, NULL);

  return dialog->files != NULL;
}



static void
fmb_properties_dialog_rename_error (BlxoJob                 *job,
                                       GError                 *error,
                                       FmbPropertiesDialog *dialog)
{
  _fmb_return_if_fail (BLXO_IS_JOB (job));
  _fmb_return_if_fail (error != NULL);
  _fmb_return_if_fail (FMB_IS_PROPERTIES_DIALOG (dialog));
  _fmb_return_if_fail (g_list_length (dialog->files) == 1);

  /* reset the entry display name to the original name, so the focus
     out event does not trigger the rename again by calling
     fmb_properties_dialog_name_activate */
  gtk_entry_set_text (GTK_ENTRY (dialog->name_entry),
                      fmb_file_get_display_name (FMB_FILE (dialog->files->data)));

  /* display an error message */
  fmb_dialogs_show_error (GTK_WIDGET (dialog), error, _("Failed to rename \"%s\""),
                             fmb_file_get_display_name (FMB_FILE (dialog->files->data)));
}



static void
fmb_properties_dialog_rename_finished (BlxoJob                 *job,
                                          FmbPropertiesDialog *dialog)
{
  _fmb_return_if_fail (BLXO_IS_JOB (job));
  _fmb_return_if_fail (FMB_IS_PROPERTIES_DIALOG (dialog));
  _fmb_return_if_fail (g_list_length (dialog->files) == 1);

  g_signal_handlers_disconnect_matched (job, G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, dialog);
  g_object_unref (job);
}



static void
fmb_properties_dialog_name_activate (GtkWidget              *entry,
                                        FmbPropertiesDialog *dialog)
{
  const gchar *old_name;
  FmbJob   *job;
  gchar       *new_name;
  FmbFile  *file;

  _fmb_return_if_fail (FMB_IS_PROPERTIES_DIALOG (dialog));

  /* check if we still have a valid file and if the user is allowed to rename */
  if (G_UNLIKELY (!gtk_widget_get_sensitive (dialog->name_entry)
      || g_list_length (dialog->files) != 1))
    return;

  /* determine new and old name */
  file = FMB_FILE (dialog->files->data);
  new_name = gtk_editable_get_chars (GTK_EDITABLE (dialog->name_entry), 0, -1);
  old_name = fmb_file_get_display_name (file);
  if (g_utf8_collate (new_name, old_name) != 0)
    {
      job = fmb_io_jobs_rename_file (file, new_name);
      if (job != NULL)
        {
          g_signal_connect (job, "error", G_CALLBACK (fmb_properties_dialog_rename_error), dialog);
          g_signal_connect (job, "finished", G_CALLBACK (fmb_properties_dialog_rename_finished), dialog);
        }
    }
}



static gboolean
fmb_properties_dialog_name_focus_out_event (GtkWidget              *entry,
                                               GdkEventFocus          *event,
                                               FmbPropertiesDialog *dialog)
{
  fmb_properties_dialog_name_activate (entry, dialog);
  return FALSE;
}



static void
fmb_properties_dialog_icon_button_clicked (GtkWidget              *button,
                                              FmbPropertiesDialog *dialog)
{
  GtkWidget   *chooser;
  GError      *err = NULL;
  const gchar *custom_icon;
  gchar       *title;
  gchar       *icon;
  FmbFile  *file;

  _fmb_return_if_fail (FMB_IS_PROPERTIES_DIALOG (dialog));
  _fmb_return_if_fail (GTK_IS_BUTTON (button));
  _fmb_return_if_fail (g_list_length (dialog->files) == 1);

  /* make sure we still have a file */
  if (G_UNLIKELY (dialog->files == NULL))
    return;

  file = FMB_FILE (dialog->files->data);

  /* allocate the icon chooser */
  title = g_strdup_printf (_("Select an Icon for \"%s\""), fmb_file_get_display_name (file));
  chooser = blxo_icon_chooser_dialog_new (title, GTK_WINDOW (dialog),
                                         GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                         GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
                                         NULL);
  gtk_dialog_set_alternative_button_order (GTK_DIALOG (chooser), GTK_RESPONSE_ACCEPT, GTK_RESPONSE_CANCEL, -1);
  gtk_dialog_set_default_response (GTK_DIALOG (chooser), GTK_RESPONSE_ACCEPT);
  g_free (title);

  /* use the custom_icon of the file as default */
  custom_icon = fmb_file_get_custom_icon (file);
  if (G_LIKELY (custom_icon != NULL && *custom_icon != '\0'))
    blxo_icon_chooser_dialog_set_icon (BLXO_ICON_CHOOSER_DIALOG (chooser), custom_icon);

  /* run the icon chooser dialog and make sure the dialog still has a file */
  if (gtk_dialog_run (GTK_DIALOG (chooser)) == GTK_RESPONSE_ACCEPT && file != NULL)
    {
      /* determine the selected icon and use it for the file */
      icon = blxo_icon_chooser_dialog_get_icon (BLXO_ICON_CHOOSER_DIALOG (chooser));
      if (!fmb_file_set_custom_icon (file, icon, &err))
        {
          /* hide the icon chooser dialog first */
          gtk_widget_hide (chooser);

          /* tell the user that we failed to change the icon of the .desktop file */
          fmb_dialogs_show_error (GTK_WIDGET (dialog), err,
                                     _("Failed to change icon of \"%s\""),
                                     fmb_file_get_display_name (file));
          g_error_free (err);
        }
      g_free (icon);
    }

  /* destroy the chooser */
  gtk_widget_destroy (chooser);
}



static void
fmb_properties_dialog_update_providers (FmbPropertiesDialog *dialog)
{
  GtkWidget *label_widget;
  GList     *providers;
  GList     *pages = NULL;
  GList     *tmp;
  GList     *lp;

  /* load the property page providers from the provider factory */
  providers = fmbx_provider_factory_list_providers (dialog->provider_factory, FMBX_TYPE_PROPERTY_PAGE_PROVIDER);
  if (G_LIKELY (providers != NULL))
    {
      /* load the pages offered by the menu providers */
      for (lp = providers; lp != NULL; lp = lp->next)
        {
          tmp = fmbx_property_page_provider_get_pages (lp->data, dialog->files);
          pages = g_list_concat (pages, tmp);
          g_object_unref (G_OBJECT (lp->data));
        }
      g_list_free (providers);
    }

  /* destroy any previous set pages */
  for (lp = dialog->provider_pages; lp != NULL; lp = lp->next)
    {
      gtk_widget_destroy (GTK_WIDGET (lp->data));
      g_object_unref (G_OBJECT (lp->data));
    }
  g_list_free (dialog->provider_pages);

  /* apply the new set of pages */
  dialog->provider_pages = pages;
  for (lp = pages; lp != NULL; lp = lp->next)
    {
      label_widget = fmbx_property_page_get_label_widget (FMBX_PROPERTY_PAGE (lp->data));
      gtk_notebook_append_page (GTK_NOTEBOOK (dialog->notebook), GTK_WIDGET (lp->data), label_widget);
      g_object_ref (G_OBJECT (lp->data));
      gtk_widget_show (lp->data);
    }
}



static void
fmb_properties_dialog_update_single (FmbPropertiesDialog *dialog)
{
  FmbIconFactory *icon_factory;
  FmbDateStyle    date_style;
  GtkIconTheme      *icon_theme;
  const gchar       *content_type;
  const gchar       *name;
  const gchar       *path;
  GVolume           *volume;
  GIcon             *gicon;
  glong              offset;
  gchar             *date;
  gchar             *display_name;
  gchar             *fs_string;
  gchar             *str;
  gchar             *volume_name;
  FmbFile        *file;
  FmbFile        *parent_file;
  gboolean           show_chooser;
  guint64            fs_free;
  guint64            fs_size;
  gdouble            fs_fraction = 0.0;

  _fmb_return_if_fail (FMB_IS_PROPERTIES_DIALOG (dialog));
  _fmb_return_if_fail (g_list_length (dialog->files) == 1);
  _fmb_return_if_fail (FMB_IS_FILE (dialog->files->data));

  /* whether the dialog shows a single file or a group of files */
  file = FMB_FILE (dialog->files->data);

  /* hide the permissions chooser for trashed files */
  gtk_widget_set_visible (dialog->permissions_chooser, !fmb_file_is_trashed (file));

  /* queue a new thumbnail request */
  fmb_thumbnailer_queue_file (dialog->thumbnailer, file,
                                 &dialog->thumbnail_request);

  icon_theme = gtk_icon_theme_get_for_screen (gtk_widget_get_screen (GTK_WIDGET (dialog)));
  icon_factory = fmb_icon_factory_get_for_icon_theme (icon_theme);

  /* determine the style used to format dates */
  g_object_get (G_OBJECT (dialog->preferences), "misc-date-style", &date_style, NULL);

  /* update the properties dialog title */
  str = g_strdup_printf (_("%s - Properties"), fmb_file_get_display_name (file));
  gtk_window_set_title (GTK_WINDOW (dialog), str);
  g_free (str);

  /* update the preview image */
  fmb_image_set_file (FMB_IMAGE (dialog->icon_image), file);

  /* check if the icon may be changed (only for writable .desktop files) */
  g_object_ref (G_OBJECT (dialog->icon_image));
  gtk_container_remove (GTK_CONTAINER (gtk_widget_get_parent (dialog->icon_image)), dialog->icon_image);
  if (fmb_file_is_writable (file)
      && fmb_file_is_desktop_file (file, NULL))
    {
      gtk_container_add (GTK_CONTAINER (dialog->icon_button), dialog->icon_image);
      gtk_widget_show (dialog->icon_button);
    }
  else
    {
      gtk_box_pack_start (GTK_BOX (gtk_widget_get_parent (dialog->icon_button)), dialog->icon_image, FALSE, TRUE, 0);
      gtk_widget_hide (dialog->icon_button);
    }
  g_object_unref (G_OBJECT (dialog->icon_image));

  /* update the name (if it differs) */
  gtk_editable_set_editable (GTK_EDITABLE (dialog->name_entry), fmb_file_is_renameable (file));
  name = fmb_file_get_display_name (file);
  if (G_LIKELY (strcmp (name, gtk_entry_get_text (GTK_ENTRY (dialog->name_entry))) != 0))
    {
      gtk_entry_set_text (GTK_ENTRY (dialog->name_entry), name);

      /* grab the input focus to the name entry */
      gtk_widget_grab_focus (dialog->name_entry);

      /* select the pre-dot part of the name */
      str = fmb_util_str_get_extension (name);
      if (G_LIKELY (str != NULL))
        {
          /* calculate the offset */
          offset = g_utf8_pointer_to_offset (name, str);

          /* select the region */
          if (G_LIKELY (offset > 0))
            gtk_editable_select_region (GTK_EDITABLE (dialog->name_entry), 0, offset);
        }
    }

  /* update the content type */
  content_type = fmb_file_get_content_type (file);
  if (content_type != NULL)
    {
      if (G_UNLIKELY (g_content_type_equals (content_type, "inode/symlink")))
        str = g_strdup (_("broken link"));
      else if (G_UNLIKELY (fmb_file_is_symlink (file)))
        str = g_strdup_printf (_("link to %s"), fmb_file_get_symlink_target (file));
      else
        str = g_content_type_get_description (content_type);
      gtk_widget_set_tooltip_text (dialog->kind_ebox, content_type);
      gtk_label_set_text (GTK_LABEL (dialog->kind_label), str);
      g_free (str);
    }
  else
    {
      gtk_label_set_text (GTK_LABEL (dialog->kind_label), _("unknown"));
    }

  /* update the application chooser (shown only for non-executable regular files!) */
  show_chooser = fmb_file_is_regular (file) && !fmb_file_is_executable (file);
  gtk_widget_set_visible (dialog->openwith_chooser, show_chooser);
  if (show_chooser)
    fmb_chooser_button_set_file (FMB_CHOOSER_BUTTON (dialog->openwith_chooser), file);

  /* update the link target */
  path = fmb_file_is_symlink (file) ? fmb_file_get_symlink_target (file) : NULL;
  if (G_UNLIKELY (path != NULL))
    {
      display_name = g_filename_display_name (path);
      gtk_label_set_text (GTK_LABEL (dialog->link_label), display_name);
      gtk_widget_show (dialog->link_label);
      g_free (display_name);
    }
  else
    {
      gtk_widget_hide (dialog->link_label);
    }

  /* update the original path */
  path = fmb_file_get_original_path (file);
  if (G_UNLIKELY (path != NULL))
    {
      display_name = g_filename_display_name (path);
      gtk_label_set_text (GTK_LABEL (dialog->origin_label), display_name);
      gtk_widget_show (dialog->origin_label);
      g_free (display_name);
    }
  else
    {
      gtk_widget_hide (dialog->origin_label);
    }

  /* update the file or folder location (parent) */
  parent_file = fmb_file_get_parent (file, NULL);
  if (G_UNLIKELY (parent_file != NULL))
    {
      display_name = g_file_get_parse_name (fmb_file_get_file (parent_file));
      gtk_label_set_text (GTK_LABEL (dialog->location_label), display_name);
      gtk_widget_show (dialog->location_label);
      g_object_unref (G_OBJECT (parent_file));
      g_free (display_name);
    }
  else
    {
      gtk_widget_hide (dialog->location_label);
    }

  /* update the deleted time */
  date = fmb_file_get_deletion_date (file, date_style);
  if (G_LIKELY (date != NULL))
    {
      gtk_label_set_text (GTK_LABEL (dialog->deleted_label), date);
      gtk_widget_show (dialog->deleted_label);
      g_free (date);
    }
  else
    {
      gtk_widget_hide (dialog->deleted_label);
    }

  /* update the modified time */
  date = fmb_file_get_date_string (file, FMB_FILE_DATE_MODIFIED, date_style);
  if (G_LIKELY (date != NULL))
    {
      gtk_label_set_text (GTK_LABEL (dialog->modified_label), date);
      gtk_widget_show (dialog->modified_label);
      g_free (date);
    }
  else
    {
      gtk_widget_hide (dialog->modified_label);
    }

  /* update the accessed time */
  date = fmb_file_get_date_string (file, FMB_FILE_DATE_ACCESSED, date_style);
  if (G_LIKELY (date != NULL))
    {
      gtk_label_set_text (GTK_LABEL (dialog->accessed_label), date);
      gtk_widget_show (dialog->accessed_label);
      g_free (date);
    }
  else
    {
      gtk_widget_hide (dialog->accessed_label);
    }

  /* update the free space (only for folders) */
  if (fmb_file_is_directory (file))
    {
      fs_string = fmb_g_file_get_free_space_string (fmb_file_get_file (file),
                                                       dialog->file_size_binary);
      if (fmb_g_file_get_free_space (fmb_file_get_file (file), &fs_free, &fs_size)
          && fs_size > 0)
        {
          /* free disk space fraction */
          fs_fraction = ((fs_size - fs_free) * 100 / fs_size);
        }
      if (fs_string != NULL)
        {
          gtk_label_set_text (GTK_LABEL (dialog->freespace_label), fs_string);
          gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (dialog->freespace_bar), fs_fraction / 100);
          gtk_widget_show (dialog->freespace_vbox);
          g_free (fs_string);
        }
      else
        {
          gtk_widget_hide (dialog->freespace_vbox);
        }
    }
  else
    {
      gtk_widget_hide (dialog->freespace_vbox);
    }

  /* update the volume */
  volume = fmb_file_get_volume (file);
  if (G_LIKELY (volume != NULL))
    {
      gicon = g_volume_get_icon (volume);
      gtk_image_set_from_gicon (GTK_IMAGE (dialog->volume_image), gicon, GTK_ICON_SIZE_MENU);
      if (G_LIKELY (gicon != NULL))
        g_object_unref (gicon);

      volume_name = g_volume_get_name (volume);
      gtk_label_set_text (GTK_LABEL (dialog->volume_label), volume_name);
      gtk_widget_show (dialog->volume_label);
      g_free (volume_name);
    }
  else
    {
      gtk_widget_hide (dialog->volume_label);
    }

  /* cleanup */
  g_object_unref (G_OBJECT (icon_factory));
}



static void
fmb_properties_dialog_update_multiple (FmbPropertiesDialog *dialog)
{
  FmbFile  *file;
  GString     *names_string;
  gboolean     first_file = TRUE;
  GList       *lp;
  const gchar *content_type = NULL;
  const gchar *tmp;
  gchar       *str;
  GVolume     *volume = NULL;
  GVolume     *tmp_volume;
  GIcon       *gicon;
  gchar       *volume_name;
  gchar       *display_name;
  FmbFile  *parent_file = NULL;
  FmbFile  *tmp_parent;
  gboolean     has_trashed_files = FALSE;

  _fmb_return_if_fail (FMB_IS_PROPERTIES_DIALOG (dialog));
  _fmb_return_if_fail (g_list_length (dialog->files) > 1);

  /* update the properties dialog title */
  gtk_window_set_title (GTK_WINDOW (dialog), _("Properties"));

  /* widgets not used with > 1 file selected */
  gtk_widget_hide (dialog->deleted_label);
  gtk_widget_hide (dialog->modified_label);
  gtk_widget_hide (dialog->accessed_label);
  gtk_widget_hide (dialog->freespace_vbox);
  gtk_widget_hide (dialog->origin_label);
  gtk_widget_hide (dialog->openwith_chooser);
  gtk_widget_hide (dialog->link_label);

  names_string = g_string_new (NULL);

  /* collect data of the selected files */
  for (lp = dialog->files; lp != NULL; lp = lp->next)
    {
      _fmb_assert (FMB_IS_FILE (lp->data));
      file = FMB_FILE (lp->data);

      /* append the name */
      if (!first_file)
        g_string_append (names_string, ", ");
      g_string_append (names_string, fmb_file_get_display_name (file));

      /* update the content type */
      if (first_file)
        {
          content_type = fmb_file_get_content_type (file);
        }
      else if (content_type != NULL)
        {
          /* check the types match */
          tmp = fmb_file_get_content_type (file);
          if (tmp == NULL || !g_content_type_equals (content_type, tmp))
            content_type = NULL;
        }

      /* check if all selected files are on the same volume */
      tmp_volume = fmb_file_get_volume (file);
      if (first_file)
        {
          volume = tmp_volume;
        }
      else if (tmp_volume != NULL)
        {
          /* we only display information if the files are on the same volume */
          if (tmp_volume != volume)
            {
              if (volume != NULL)
                g_object_unref (G_OBJECT (volume));
              volume = NULL;
            }

          g_object_unref (G_OBJECT (tmp_volume));
        }

      /* check if all files have the same parent */
      tmp_parent = fmb_file_get_parent (file, NULL);
      if (first_file)
        {
          parent_file = tmp_parent;
        }
      else if (tmp_parent != NULL)
        {
          /* we only display the location if they are all equal */
          if (!g_file_equal (fmb_file_get_file (parent_file), fmb_file_get_file (tmp_parent)))
            {
              if (parent_file != NULL)
                g_object_unref (G_OBJECT (parent_file));
              parent_file = NULL;
            }

          g_object_unref (G_OBJECT (tmp_parent));
        }

      if (fmb_file_is_trashed (file))
        has_trashed_files = TRUE;

      first_file = FALSE;
    }

  /* set the labels string */
  gtk_label_set_text (GTK_LABEL (dialog->names_label), names_string->str);
  gtk_widget_set_tooltip_text (dialog->names_label, names_string->str);
  g_string_free (names_string, TRUE);

  /* hide the permissions chooser for trashed files */
  gtk_widget_set_visible (dialog->permissions_chooser, !has_trashed_files);

  /* update the content type */
  if (content_type != NULL
      && !g_content_type_equals (content_type, "inode/symlink"))
    {
      str = g_content_type_get_description (content_type);
      gtk_widget_set_tooltip_text (dialog->kind_ebox, content_type);
      gtk_label_set_text (GTK_LABEL (dialog->kind_label), str);
      g_free (str);
    }
  else
    {
      gtk_label_set_text (GTK_LABEL (dialog->kind_label), _("mixed"));
    }

  /* update the file or folder location (parent) */
  if (G_UNLIKELY (parent_file != NULL))
    {
      display_name = g_file_get_parse_name (fmb_file_get_file (parent_file));
      gtk_label_set_text (GTK_LABEL (dialog->location_label), display_name);
      gtk_widget_show (dialog->location_label);
      g_object_unref (G_OBJECT (parent_file));
      g_free (display_name);
    }
  else
    {
      gtk_widget_hide (dialog->location_label);
    }

  /* update the volume */
  if (G_LIKELY (volume != NULL))
    {
      gicon = g_volume_get_icon (volume);
      gtk_image_set_from_gicon (GTK_IMAGE (dialog->volume_image), gicon, GTK_ICON_SIZE_MENU);
      if (G_LIKELY (gicon != NULL))
        g_object_unref (gicon);

      volume_name = g_volume_get_name (volume);
      gtk_label_set_text (GTK_LABEL (dialog->volume_label), volume_name);
      gtk_widget_show (dialog->volume_label);
      g_free (volume_name);

      g_object_unref (G_OBJECT (volume));
    }
  else
    {
      gtk_widget_hide (dialog->volume_label);
    }
}



static void
fmb_properties_dialog_update (FmbPropertiesDialog *dialog)
{
  _fmb_return_if_fail (FMB_IS_PROPERTIES_DIALOG (dialog));
  _fmb_return_if_fail (dialog->files != NULL);

  /* cancel any pending thumbnail requests */
  if (dialog->thumbnail_request > 0)
    {
      fmb_thumbnailer_dequeue (dialog->thumbnailer, dialog->thumbnail_request);
      dialog->thumbnail_request = 0;
    }

  if (dialog->files->next == NULL)
    {
      /* show single file name box */
      gtk_widget_show (dialog->single_box);

      /* update the properties for a dialog showing 1 file */
      fmb_properties_dialog_update_single (dialog);
    }
  else
    {
      /* show multiple files box */
      gtk_widget_hide (dialog->single_box);

      /* update the properties for a dialog showing multiple files */
      fmb_properties_dialog_update_multiple (dialog);
    }
}



/**
 * fmb_properties_dialog_new:
 * @parent: transient window or NULL;
 *
 * Allocates a new #FmbPropertiesDialog instance,
 * that is not associated with any #FmbFile.
 *
 * Return value: the newly allocated #FmbPropertiesDialog
 *               instance.
 **/
GtkWidget*
fmb_properties_dialog_new (GtkWindow *parent)
{
  _fmb_return_val_if_fail (parent == NULL || GTK_IS_WINDOW (parent), NULL);
  return g_object_new (FMB_TYPE_PROPERTIES_DIALOG,
                       "transient-for", parent,
                       "destroy-with-parent", parent != NULL,
                       NULL);
}



/**
 * fmb_properties_dialog_get_files:
 * @dialog : a #FmbPropertiesDialog.
 *
 * Returns the #FmbFile currently being displayed
 * by @dialog or %NULL if @dialog doesn't display
 * any file right now.
 *
 * Return value: list of #FmbFile's displayed by @dialog
 *               or %NULL.
 **/
static GList*
fmb_properties_dialog_get_files (FmbPropertiesDialog *dialog)
{
  _fmb_return_val_if_fail (FMB_IS_PROPERTIES_DIALOG (dialog), NULL);
  return dialog->files;
}



/**
 * fmb_properties_dialog_set_files:
 * @dialog : a #FmbPropertiesDialog.
 * @files  : a GList of #FmbFile's or %NULL.
 *
 * Sets the #FmbFile that is displayed by @dialog
 * to @files.
 **/
void
fmb_properties_dialog_set_files (FmbPropertiesDialog *dialog,
                                    GList                  *files)
{
  GList      *lp;
  FmbFile *file;

  _fmb_return_if_fail (FMB_IS_PROPERTIES_DIALOG (dialog));

  /* check if the same lists are used (or null) */
  if (G_UNLIKELY (dialog->files == files))
    return;

  /* disconnect from any previously set files */
  for (lp = dialog->files; lp != NULL; lp = lp->next)
    {
      file = FMB_FILE (lp->data);

      /* unregister our file watch */
      fmb_file_unwatch (file);

      /* unregister handlers */
      g_signal_handlers_disconnect_by_func (G_OBJECT (file), fmb_properties_dialog_update, dialog);
      g_signal_handlers_disconnect_by_func (G_OBJECT (file), gtk_widget_destroy, dialog);

      g_object_unref (G_OBJECT (file));
    }
  g_list_free (dialog->files);

  /* activate the new list */
  dialog->files = g_list_copy (files);

  /* connect to the new files */
  for (lp = dialog->files; lp != NULL; lp = lp->next)
    {
      _fmb_assert (FMB_IS_FILE (lp->data));
      file = g_object_ref (G_OBJECT (lp->data));

      /* watch the file for changes */
      fmb_file_watch (file);

      /* install signal handlers */
      g_signal_connect_swapped (G_OBJECT (file), "changed", G_CALLBACK (fmb_properties_dialog_update), dialog);
      g_signal_connect_swapped (G_OBJECT (file), "destroy", G_CALLBACK (gtk_widget_destroy), dialog);
    }

  /* update the dialog contents */
  if (dialog->files != NULL)
    {
      /* update the UI for the new file */
      fmb_properties_dialog_update (dialog);

      /* update the provider property pages */
      fmb_properties_dialog_update_providers (dialog);
    }

  /* tell everybody that we have a new file here */
  g_object_notify (G_OBJECT (dialog), "files");
}



/**
 * fmb_properties_dialog_set_file:
 * @dialog : a #FmbPropertiesDialog.
 * @file   : a #FmbFile or %NULL.
 *
 * Sets the #FmbFile that is displayed by @dialog
 * to @file.
 **/
void
fmb_properties_dialog_set_file (FmbPropertiesDialog *dialog,
                                   FmbFile             *file)
{
  GList foo;

  _fmb_return_if_fail (FMB_IS_PROPERTIES_DIALOG (dialog));
  _fmb_return_if_fail (file == NULL || FMB_IS_FILE (file));

  if (file == NULL)
    {
      fmb_properties_dialog_set_files (dialog, NULL);
    }
  else
    {
      /* create a fake list */
      foo.next = NULL;
      foo.prev = NULL;
      foo.data = file;

      fmb_properties_dialog_set_files (dialog, &foo);
    }
}




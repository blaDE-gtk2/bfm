/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2006 Benedikt Meurer <benny@xfce.org>
 * Copyright (c) 2009 Jannis Pohlmann <jannis@xfce.org>
 * Copyright (c) 2012 Nick Schermer <nick@xfce.org>
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

#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif
#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif

#include <fmb/fmb-deep-count-job.h>
#include <fmb/fmb-gtk-extensions.h>
#include <fmb/fmb-preferences.h>
#include <fmb/fmb-private.h>
#include <fmb/fmb-size-label.h>



/* Property identifiers */
enum
{
  PROP_0,
  PROP_FILES,
  PROP_FILE_SIZE_BINARY
};



static void     fmb_size_label_finalize              (GObject              *object);
static void     fmb_size_label_get_property          (GObject              *object,
                                                         guint                 prop_id,
                                                         GValue               *value,
                                                         GParamSpec           *pspec);
static void     fmb_size_label_set_property          (GObject              *object,
                                                         guint                 prop_id,
                                                         const GValue         *value,
                                                         GParamSpec           *pspec);
static gboolean fmb_size_label_button_press_event    (GtkWidget            *ebox,
                                                         GdkEventButton       *event,
                                                         FmbSizeLabel      *size_label);
static void     fmb_size_label_files_changed         (FmbSizeLabel      *size_label);
static void     fmb_size_label_error                 (BlxoJob               *job,
                                                         const GError         *error,
                                                         FmbSizeLabel      *size_label);
static void     fmb_size_label_finished              (BlxoJob               *job,
                                                         FmbSizeLabel      *size_label);
static void     fmb_size_label_status_update         (FmbDeepCountJob   *job,
                                                         guint64               total_size,
                                                         guint                 file_count,
                                                         guint                 directory_count,
                                                         guint                 unreadable_directory_count,
                                                         FmbSizeLabel      *size_label);
static GList   *fmb_size_label_get_files             (FmbSizeLabel      *size_label);
static void     fmb_size_label_set_files             (FmbSizeLabel      *size_label,
                                                         GList                *files);



struct _FmbSizeLabelClass
{
  GtkHBoxClass __parent__;
};

struct _FmbSizeLabel
{
  GtkHBox             __parent__;

  FmbDeepCountJob *job;
  FmbPreferences  *preferences;

  GList              *files;
  gboolean            file_size_binary;

  GtkWidget          *label;
  GtkWidget          *spinner;
};



G_DEFINE_TYPE (FmbSizeLabel, fmb_size_label, GTK_TYPE_HBOX)



static void
fmb_size_label_class_init (FmbSizeLabelClass *klass)
{
  GObjectClass *gobject_class;

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = fmb_size_label_finalize;
  gobject_class->get_property = fmb_size_label_get_property;
  gobject_class->set_property = fmb_size_label_set_property;

  /**
   * FmbSizeLabel:file:
   *
   * The #FmbFile whose size should be displayed
   * by this #FmbSizeLabel.
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
}



static void
fmb_size_label_init (FmbSizeLabel *size_label)
{
  GtkWidget *ebox;

  /* binary file size */
  size_label->preferences = fmb_preferences_get ();
  blxo_binding_new (G_OBJECT (size_label->preferences), "misc-file-size-binary",
                   G_OBJECT (size_label), "file-size-binary");
  g_signal_connect_swapped (G_OBJECT (size_label->preferences), "notify::misc-file-size-binary",
                            G_CALLBACK (fmb_size_label_files_changed), size_label);
  gtk_widget_push_composite_child ();

  /* configure the box */
  gtk_box_set_spacing (GTK_BOX (size_label), 6);

  /* add an evenbox for the spinner */
  ebox = gtk_event_box_new ();
  gtk_event_box_set_visible_window (GTK_EVENT_BOX (ebox), FALSE);
  g_signal_connect (G_OBJECT (ebox), "button-press-event", G_CALLBACK (fmb_size_label_button_press_event), size_label);
  gtk_widget_set_tooltip_text (ebox, _("Click here to stop calculating the total size of the folder."));
  gtk_box_pack_start (GTK_BOX (size_label), ebox, FALSE, FALSE, 0);

  /* add the spinner widget */
  size_label->spinner = gtk_spinner_new ();
  blxo_binding_new (G_OBJECT (size_label->spinner), "visible", G_OBJECT (ebox), "visible");
  gtk_container_add (GTK_CONTAINER (ebox), size_label->spinner);
  gtk_widget_show (size_label->spinner);

  /* add the label widget */
  size_label->label = gtk_label_new (_("Calculating..."));
  gtk_misc_set_alignment (GTK_MISC (size_label->label), 0.0f, 0.5f);
  gtk_label_set_selectable (GTK_LABEL (size_label->label), TRUE);
  gtk_label_set_ellipsize (GTK_LABEL (size_label->label), PANGO_ELLIPSIZE_MIDDLE);
  gtk_box_pack_start (GTK_BOX (size_label), size_label->label, TRUE, TRUE, 0);
  gtk_widget_show (size_label->label);

  gtk_widget_pop_composite_child ();
}



static void
fmb_size_label_finalize (GObject *object)
{
  FmbSizeLabel *size_label = FMB_SIZE_LABEL (object);

  /* cancel the pending job (if any) */
  if (G_UNLIKELY (size_label->job != NULL))
    {
      g_signal_handlers_disconnect_matched (G_OBJECT (size_label->job), G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, size_label);
      blxo_job_cancel (BLXO_JOB (size_label->job));
      g_object_unref (size_label->job);
    }

  /* reset the file property */
  fmb_size_label_set_files (size_label, NULL);

  /* disconnect from the preferences */
  g_signal_handlers_disconnect_by_func (size_label->preferences, fmb_size_label_files_changed, size_label);
  g_object_unref (size_label->preferences);

  (*G_OBJECT_CLASS (fmb_size_label_parent_class)->finalize) (object);
}



static void
fmb_size_label_get_property (GObject    *object,
                                guint       prop_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
  FmbSizeLabel *size_label = FMB_SIZE_LABEL (object);

  switch (prop_id)
    {
    case PROP_FILES:
      g_value_set_boxed (value, fmb_size_label_get_files (size_label));
      break;

    case PROP_FILE_SIZE_BINARY:
      g_value_set_boolean (value, size_label->file_size_binary);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static void
fmb_size_label_set_property (GObject      *object,
                                guint         prop_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
  FmbSizeLabel *size_label = FMB_SIZE_LABEL (object);

  switch (prop_id)
    {
    case PROP_FILES:
      fmb_size_label_set_files (size_label, g_value_get_boxed (value));
      break;

    case PROP_FILE_SIZE_BINARY:
      size_label->file_size_binary = g_value_get_boolean (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static gboolean
fmb_size_label_button_press_event (GtkWidget       *ebox,
                                      GdkEventButton  *event,
                                      FmbSizeLabel *size_label)
{
  _fmb_return_val_if_fail (GTK_IS_EVENT_BOX (ebox), FALSE);
  _fmb_return_val_if_fail (FMB_IS_SIZE_LABEL (size_label), FALSE);

  /* left button press on the spinner cancels the calculation */
  if (G_LIKELY (event->button == 1))
    {
      /* cancel the pending job (if any) */
      if (G_UNLIKELY (size_label->job != NULL))
        {
          g_signal_handlers_disconnect_matched (size_label->job, G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, size_label);
          blxo_job_cancel (BLXO_JOB (size_label->job));
          g_object_unref (size_label->job);
          size_label->job = NULL;
        }

      /* be sure to stop and hide the spinner */
      gtk_spinner_stop (GTK_SPINNER (size_label->spinner));
      gtk_widget_hide (size_label->spinner);

      /* tell the user that the operation was canceled */
      gtk_label_set_text (GTK_LABEL (size_label->label), _("Calculation aborted"));

      /* we handled the event */
      return TRUE;
    }

  return FALSE;
}



static void
fmb_size_label_files_changed (FmbSizeLabel *size_label)
{
  gchar             *size_string;
  guint64            size;

  _fmb_return_if_fail (FMB_IS_SIZE_LABEL (size_label));
  _fmb_return_if_fail (size_label->files != NULL);
  _fmb_return_if_fail (FMB_IS_FILE (size_label->files->data));

  /* cancel the pending job (if any) */
  if (G_UNLIKELY (size_label->job != NULL))
    {
      g_signal_handlers_disconnect_matched (size_label->job, G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, size_label);
      blxo_job_cancel (BLXO_JOB (size_label->job));
      g_object_unref (size_label->job);
      size_label->job = NULL;
    }

  /* check if there are multiple files or the single file is a directory */
  if (size_label->files->next != NULL
      || fmb_file_is_directory (FMB_FILE (size_label->files->data)))
    {
      /* schedule a new job to determine the total size of the directory (not following symlinks) */
      size_label->job = fmb_deep_count_job_new (size_label->files, G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS);
      g_signal_connect (size_label->job, "error", G_CALLBACK (fmb_size_label_error), size_label);
      g_signal_connect (size_label->job, "finished", G_CALLBACK (fmb_size_label_finished), size_label);
      g_signal_connect (size_label->job, "status-update", G_CALLBACK (fmb_size_label_status_update), size_label);

      /* tell the user that we started calculation */
      gtk_label_set_text (GTK_LABEL (size_label->label), _("Calculating..."));
      gtk_spinner_start (GTK_SPINNER (size_label->spinner));
      gtk_widget_show (size_label->spinner);

      /* launch the job */
      blxo_job_launch (BLXO_JOB (size_label->job));
    }
  else
    {
      /* this is going to be quick, stop and hide the spinner */
      gtk_spinner_stop (GTK_SPINNER (size_label->spinner));
      gtk_widget_hide (size_label->spinner);

      /* determine the size of the file */
      size = fmb_file_get_size (FMB_FILE (size_label->files->data));

      /* setup the new label */
      size_string = g_format_size_full (size, size_label->file_size_binary ? G_FORMAT_SIZE_LONG_FORMAT | G_FORMAT_SIZE_IEC_UNITS : G_FORMAT_SIZE_LONG_FORMAT);
      gtk_label_set_text (GTK_LABEL (size_label->label), size_string);
      g_free (size_string);
    }
}



static void
fmb_size_label_error (BlxoJob          *job,
                         const GError    *error,
                         FmbSizeLabel *size_label)
{
  _fmb_return_if_fail (FMB_IS_JOB (job));
  _fmb_return_if_fail (FMB_IS_SIZE_LABEL (size_label));
  _fmb_return_if_fail (size_label->job == FMB_DEEP_COUNT_JOB (job));

  /* setup the error text as label */
  gtk_label_set_text (GTK_LABEL (size_label->label), error->message);
}



static void
fmb_size_label_finished (BlxoJob          *job,
                            FmbSizeLabel *size_label)
{
  _fmb_return_if_fail (FMB_IS_JOB (job));
  _fmb_return_if_fail (FMB_IS_SIZE_LABEL (size_label));
  _fmb_return_if_fail (size_label->job == FMB_DEEP_COUNT_JOB (job));

  /* stop and hide the spinner */
  gtk_spinner_stop (GTK_SPINNER (size_label->spinner));
  gtk_widget_hide (size_label->spinner);

  /* disconnect from the job */
  g_signal_handlers_disconnect_matched (size_label->job, G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, size_label);
  g_object_unref (size_label->job);
  size_label->job = NULL;
}



static void
fmb_size_label_status_update (FmbDeepCountJob *job,
                                 guint64             total_size,
                                 guint               file_count,
                                 guint               directory_count,
                                 guint               unreadable_directory_count,
                                 FmbSizeLabel    *size_label)
{
  gchar             *size_string;
  gchar             *text;
  guint              n;
  gchar             *unreable_text;

  _fmb_return_if_fail (FMB_IS_DEEP_COUNT_JOB (job));
  _fmb_return_if_fail (FMB_IS_SIZE_LABEL (size_label));
  _fmb_return_if_fail (size_label->job == job);

  /* determine the total number of items */
  n = file_count + directory_count + unreadable_directory_count;

  if (G_LIKELY (n > unreadable_directory_count))
    {
      /* update the label */
      size_string = g_format_size_full (total_size, size_label->file_size_binary ? G_FORMAT_SIZE_IEC_UNITS : G_FORMAT_SIZE_DEFAULT);
      text = g_strdup_printf (ngettext ("%u item, totalling %s", "%u items, totalling %s", n), n, size_string);
      g_free (size_string);
      
      if (unreadable_directory_count > 0)
        {
          /* TRANSLATORS: this is shows if during the deep count size
           * directories were not accessible */
          unreable_text = g_strconcat (text, "\n", _("(some contents unreadable)"), NULL);
          g_free (text);
          text = unreable_text;
        }
      
      gtk_label_set_text (GTK_LABEL (size_label->label), text);
      g_free (text);
    }
  else
    {
      /* nothing was readable, so permission was denied */
      gtk_label_set_text (GTK_LABEL (size_label->label), _("Permission denied"));
    }
}



/**
 * fmb_size_label_get_files:
 * @size_label : a #FmbSizeLabel.
 *
 * Get the files displayed by the @size_label.
 **/
static GList*
fmb_size_label_get_files (FmbSizeLabel *size_label)
{
  _fmb_return_val_if_fail (FMB_IS_SIZE_LABEL (size_label), NULL);
  return size_label->files;
}



/**
 * fmb_size_label_set_files:
 * @size_label : a #FmbSizeLabel.
 * @files      : a list of #FmbFile's or %NULL.
 *
 * Sets @file as the #FmbFile displayed by the @size_label.
 **/
static void
fmb_size_label_set_files (FmbSizeLabel *size_label,
                             GList           *files)
{
  GList *lp;

  _fmb_return_if_fail (FMB_IS_SIZE_LABEL (size_label));
  _fmb_return_if_fail (files == NULL || FMB_IS_FILE (files->data));

  /* disconnect from the previous files */
  for (lp = size_label->files; lp != NULL; lp = lp->next)
    {
      _fmb_assert (FMB_IS_FILE (lp->data));

      g_signal_handlers_disconnect_by_func (G_OBJECT (lp->data), fmb_size_label_files_changed, size_label);
      g_object_unref (G_OBJECT (lp->data));
    }
  g_list_free (size_label->files);

  size_label->files = g_list_copy (files);

  /* connect to the new file */
  for (lp = size_label->files; lp != NULL; lp = lp->next)
    {
      _fmb_assert (FMB_IS_FILE (lp->data));

      g_object_ref (G_OBJECT (lp->data));
      g_signal_connect_swapped (G_OBJECT (lp->data), "changed", G_CALLBACK (fmb_size_label_files_changed), size_label);
    }

  if (size_label->files != NULL)
    fmb_size_label_files_changed (size_label);

  /* notify listeners */
  g_object_notify (G_OBJECT (size_label), "files");
}



/**
 * fmb_size_label_new:
 *
 * Allocates a new #FmbSizeLabel instance.
 *
 * Return value: the newly allocated #FmbSizeLabel.
 **/
GtkWidget*
fmb_size_label_new (void)
{
  return g_object_new (FMB_TYPE_SIZE_LABEL, NULL);
}


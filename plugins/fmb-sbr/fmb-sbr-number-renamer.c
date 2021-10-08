/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2006 Benedikt Meurer <benny@xfce.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_MEMORY_H
#include <memory.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include <blxo/blxo.h>

#include <fmb-sbr/fmb-sbr-number-renamer.h>



/* Property identifiers */
enum
{
  PROP_0,
  PROP_MODE,
  PROP_START,
  PROP_TEXT,
  PROP_TEXT_MODE,
};



static void   fmb_sbr_number_renamer_finalize      (GObject                      *object);
static void   fmb_sbr_number_renamer_get_property  (GObject                      *object,
                                                       guint                         prop_id,
                                                       GValue                       *value,
                                                       GParamSpec                   *pspec);
static void   fmb_sbr_number_renamer_set_property  (GObject                      *object,
                                                       guint                         prop_id,
                                                       const GValue                 *value,
                                                       GParamSpec                   *pspec);
static void   fmb_sbr_number_renamer_realize       (GtkWidget                    *widget);
static gchar *fmb_sbr_number_renamer_process       (FmbxRenamer               *renamer,
                                                       FmbxFileInfo              *file,
                                                       const gchar                  *text,
                                                       guint                         idx);
static void   fmb_sbr_number_renamer_update        (FmbSbrNumberRenamer       *number_renamer);



struct _FmbSbrNumberRenamerClass
{
  FmbxRenamerClass __parent__;
};

struct _FmbSbrNumberRenamer
{
  FmbxRenamer      __parent__;

  GtkWidget          *start_entry;

  FmbSbrNumberMode mode;
  gchar              *start;
  gchar              *text;
  FmbSbrTextMode   text_mode;
};



FMBX_DEFINE_TYPE (FmbSbrNumberRenamer, fmb_sbr_number_renamer, FMBX_TYPE_RENAMER);



static void
fmb_sbr_number_renamer_class_init (FmbSbrNumberRenamerClass *klass)
{
  FmbxRenamerClass *fmbxrenamer_class;
  GtkWidgetClass      *gtkwidget_class;
  GObjectClass        *gobject_class;

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = fmb_sbr_number_renamer_finalize;
  gobject_class->get_property = fmb_sbr_number_renamer_get_property;
  gobject_class->set_property = fmb_sbr_number_renamer_set_property;

  gtkwidget_class = GTK_WIDGET_CLASS (klass);
  gtkwidget_class->realize = fmb_sbr_number_renamer_realize;

  fmbxrenamer_class = FMBX_RENAMER_CLASS (klass);
  fmbxrenamer_class->process = fmb_sbr_number_renamer_process;

  /**
   * FmbSbrNumberRenamer:mode:
   *
   * The #FmbSbrNumberMode to use.
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_MODE,
                                   g_param_spec_enum ("mode", "mode", "mode",
                                                      FMB_SBR_TYPE_NUMBER_MODE,
                                                      FMB_SBR_NUMBER_MODE_123,
                                                      G_PARAM_READWRITE));

  /**
   * FmbSbrNumberRenamer:start:
   *
   * The starting value according to the #FmbSbrNumberMode.
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_START,
                                   g_param_spec_string ("start",
                                                        "start",
                                                        "start",
                                                        "1",
                                                        G_PARAM_CONSTRUCT | G_PARAM_READWRITE));

  /**
   * FmbSbrNumberRenamer:text:
   *
   * The additional text, depending on the #FmbSbrTextMode.
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_TEXT,
                                   g_param_spec_string ("text",
                                                        "text",
                                                        "text",
                                                        ". ",
                                                        G_PARAM_CONSTRUCT | G_PARAM_READWRITE));

  /**
   * FmbSbrNumberRenamer:text-mode:
   *
   * The text mode for the renamer.
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_TEXT_MODE,
                                   g_param_spec_enum ("text-mode",
                                                      "text-mode",
                                                      "text-mode",
                                                      FMB_SBR_TYPE_TEXT_MODE,
                                                      FMB_SBR_TEXT_MODE_NTO,
                                                      G_PARAM_CONSTRUCT | G_PARAM_READWRITE));
}



static void
fmb_sbr_number_renamer_init (FmbSbrNumberRenamer *number_renamer)
{
  AtkRelationSet *relations;
  AtkRelation    *relation;
  GEnumClass     *klass;
  AtkObject      *object;
  GtkWidget      *combo;
  GtkWidget      *entry;
  GtkWidget      *label;
  GtkWidget      *hbox;
  guint           n;

  hbox = gtk_hbox_new (FALSE, 12);
  gtk_box_pack_start (GTK_BOX (number_renamer), hbox, FALSE, FALSE, 0);
  gtk_widget_show (hbox);

  label = gtk_label_new_with_mnemonic (_("_Number Format:"));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  combo = gtk_combo_box_new_text ();
  klass = g_type_class_ref (FMB_SBR_TYPE_NUMBER_MODE);
  for (n = 0; n < klass->n_values; ++n)
    gtk_combo_box_append_text (GTK_COMBO_BOX (combo), _(klass->values[n].value_nick));
  blxo_mutual_binding_new (G_OBJECT (number_renamer), "mode", G_OBJECT (combo), "active");
  gtk_box_pack_start (GTK_BOX (hbox), combo, FALSE, FALSE, 0);
  gtk_label_set_mnemonic_widget (GTK_LABEL (label), combo);
  g_type_class_unref (klass);
  gtk_widget_show (combo);

  /* set Atk label relation for the combo */
  object = gtk_widget_get_accessible (combo);
  relations = atk_object_ref_relation_set (gtk_widget_get_accessible (label));
  relation = atk_relation_new (&object, 1, ATK_RELATION_LABEL_FOR);
  atk_relation_set_add (relations, relation);
  g_object_unref (G_OBJECT (relation));

  number_renamer->start_entry = gtk_entry_new ();
  gtk_entry_set_max_length (GTK_ENTRY (number_renamer->start_entry), 4);
  gtk_entry_set_width_chars (GTK_ENTRY (number_renamer->start_entry), 3);
  gtk_entry_set_alignment (GTK_ENTRY (number_renamer->start_entry), 1.0f);
  gtk_entry_set_activates_default (GTK_ENTRY (number_renamer->start_entry), TRUE);
  blxo_mutual_binding_new (G_OBJECT (number_renamer->start_entry), "text", G_OBJECT (number_renamer), "start");
  gtk_box_pack_end (GTK_BOX (hbox), number_renamer->start_entry, TRUE, TRUE, 0);
  gtk_widget_show (number_renamer->start_entry);

  label = gtk_label_new_with_mnemonic (_("_Start With:"));
  gtk_box_pack_end (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  gtk_label_set_mnemonic_widget (GTK_LABEL (label), number_renamer->start_entry);
  gtk_widget_show (label);

  /* set Atk label relation for the entry */
  object = gtk_widget_get_accessible (number_renamer->start_entry);
  relations = atk_object_ref_relation_set (gtk_widget_get_accessible (label));
  relation = atk_relation_new (&object, 1, ATK_RELATION_LABEL_FOR);
  atk_relation_set_add (relations, relation);
  g_object_unref (G_OBJECT (relation));

  hbox = gtk_hbox_new (FALSE, 12);
  gtk_box_pack_end (GTK_BOX (number_renamer), hbox, FALSE, FALSE, 0);
  gtk_widget_show (hbox);

  label = gtk_label_new_with_mnemonic (_("Text _Format:"));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  combo = gtk_combo_box_new_text ();
  klass = g_type_class_ref (FMB_SBR_TYPE_TEXT_MODE);
  for (n = 0; n < klass->n_values; ++n)
    gtk_combo_box_append_text (GTK_COMBO_BOX (combo), _(klass->values[n].value_nick));
  blxo_mutual_binding_new (G_OBJECT (number_renamer), "text-mode", G_OBJECT (combo), "active");
  gtk_box_pack_start (GTK_BOX (hbox), combo, FALSE, FALSE, 0);
  gtk_label_set_mnemonic_widget (GTK_LABEL (label), combo);
  g_type_class_unref (klass);
  gtk_widget_show (combo);

  /* set Atk label relation for the combo */
  object = gtk_widget_get_accessible (combo);
  relations = atk_object_ref_relation_set (gtk_widget_get_accessible (label));
  relation = atk_relation_new (&object, 1, ATK_RELATION_LABEL_FOR);
  atk_relation_set_add (relations, relation);
  g_object_unref (G_OBJECT (relation));

  entry = gtk_entry_new ();
  gtk_entry_set_width_chars (GTK_ENTRY (entry), 4);
  gtk_entry_set_activates_default (GTK_ENTRY (entry), TRUE);
  blxo_mutual_binding_new (G_OBJECT (entry), "text", G_OBJECT (number_renamer), "text");
  gtk_box_pack_end (GTK_BOX (hbox), entry, TRUE, TRUE, 0);
  gtk_widget_show (entry);

  label = gtk_label_new_with_mnemonic (_("_Text:"));
  gtk_misc_set_alignment (GTK_MISC (label), 1.0f, 0.5f);
  gtk_box_pack_end (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  gtk_label_set_mnemonic_widget (GTK_LABEL (label), entry);
  gtk_widget_show (label);

  /* set Atk label relation for the entry */
  object = gtk_widget_get_accessible (entry);
  relations = atk_object_ref_relation_set (gtk_widget_get_accessible (label));
  relation = atk_relation_new (&object, 1, ATK_RELATION_LABEL_FOR);
  atk_relation_set_add (relations, relation);
  g_object_unref (G_OBJECT (relation));
}



static void
fmb_sbr_number_renamer_finalize (GObject *object)
{
  FmbSbrNumberRenamer *number_renamer = FMB_SBR_NUMBER_RENAMER (object);

  /* release renamer resources */
  g_free (number_renamer->start);
  g_free (number_renamer->text);

  (*G_OBJECT_CLASS (fmb_sbr_number_renamer_parent_class)->finalize) (object);
}



static void
fmb_sbr_number_renamer_get_property (GObject    *object,
                                        guint       prop_id,
                                        GValue     *value,
                                        GParamSpec *pspec)
{
  FmbSbrNumberRenamer *number_renamer = FMB_SBR_NUMBER_RENAMER (object);

  switch (prop_id)
    {
    case PROP_MODE:
      g_value_set_enum (value, fmb_sbr_number_renamer_get_mode (number_renamer));
      break;

    case PROP_START:
      g_value_set_string (value, fmb_sbr_number_renamer_get_start (number_renamer));
      break;

    case PROP_TEXT:
      g_value_set_string (value, fmb_sbr_number_renamer_get_text (number_renamer));
      break;

    case PROP_TEXT_MODE:
      g_value_set_enum (value, fmb_sbr_number_renamer_get_text_mode (number_renamer));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static void
fmb_sbr_number_renamer_set_property (GObject      *object,
                                        guint         prop_id,
                                        const GValue *value,
                                        GParamSpec   *pspec)
{
  FmbSbrNumberRenamer *number_renamer = FMB_SBR_NUMBER_RENAMER (object);

  switch (prop_id)
    {
    case PROP_MODE:
      fmb_sbr_number_renamer_set_mode (number_renamer, g_value_get_enum (value));
      break;

    case PROP_START:
      fmb_sbr_number_renamer_set_start (number_renamer, g_value_get_string (value));
      break;

    case PROP_TEXT:
      fmb_sbr_number_renamer_set_text (number_renamer, g_value_get_string (value));
      break;

    case PROP_TEXT_MODE:
      fmb_sbr_number_renamer_set_text_mode (number_renamer, g_value_get_enum (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static void
fmb_sbr_number_renamer_realize (GtkWidget *widget)
{
  /* realize the widget */
  (*GTK_WIDGET_CLASS (fmb_sbr_number_renamer_parent_class)->realize) (widget);

  /* update the state */
  fmb_sbr_number_renamer_update (FMB_SBR_NUMBER_RENAMER (widget));
}



static gchar*
fmb_sbr_number_renamer_process (FmbxRenamer  *renamer,
                                   FmbxFileInfo *file,
                                   const gchar     *text,
                                   guint            idx)
{
  FmbSbrNumberRenamer *number_renamer = FMB_SBR_NUMBER_RENAMER (renamer);
  gboolean                invalid = TRUE;
  gchar                  *endp;
  gchar                  *name;
  gchar                  *number = NULL;
  guint                   start = 0;

  /* check whether "start" is valid for the "mode" */
  if (number_renamer->mode < FMB_SBR_NUMBER_MODE_ABC)
    {
      /* "start" must be a positive number */
      start = strtoul (number_renamer->start, &endp, 10);
      invalid = (endp <= number_renamer->start || *endp != '\0');
    }
  else if (number_renamer->mode == FMB_SBR_NUMBER_MODE_ABC)
    {
      /* "start" property must be 'a', 'b', 'c', etc. */
      start = *number_renamer->start;
      invalid = (strlen (number_renamer->start) != 1
              || g_ascii_tolower (start) < 'a'
              || g_ascii_tolower (start) > 'z');
    }

  /* check if we have invalid settings */
  if (G_UNLIKELY (invalid))
    return g_strdup (text);

  /* format the number */
  switch (number_renamer->mode)
    {
    case FMB_SBR_NUMBER_MODE_123:
      number = g_strdup_printf ("%u", start + idx);
      break;

    case FMB_SBR_NUMBER_MODE_010203:
      number = g_strdup_printf ("%02u", start + idx);
      break;

    case FMB_SBR_NUMBER_MODE_001002003:
      number = g_strdup_printf ("%03u", start + idx);
      break;

    case FMB_SBR_NUMBER_MODE_000100020003:
      number = g_strdup_printf ("%04u", start + idx);
      break;

    case FMB_SBR_NUMBER_MODE_ABC:
      if (start >= 'a' && start <= 'z')
        number = g_strdup_printf ("%c", (gchar) (MIN (start + idx, 'z')));
      else if (start >= 'A' && start <= 'Z')
        number = g_strdup_printf ("%c", (gchar) (MIN (start + idx, 'Z')));
      else
        g_assert_not_reached ();
      break;

    default:
      g_assert_not_reached ();
      break;
    }

  /* format the text */
  switch (number_renamer->text_mode)
    {
    case FMB_SBR_TEXT_MODE_OTN:
      name = g_strconcat (text, number_renamer->text, number, NULL);
      break;

    case FMB_SBR_TEXT_MODE_NTO:
      name = g_strconcat (number, number_renamer->text, text, NULL);
      break;

    case FMB_SBR_TEXT_MODE_TN:
      name = g_strconcat (number_renamer->text, number, NULL);
      break;

    case FMB_SBR_TEXT_MODE_NT:
      name = g_strconcat (number, number_renamer->text, NULL);
      break;

    default:
      g_assert_not_reached ();
      break;
    }

  /* release the number */
  g_free (number);

  /* return the new name */
  return name;
}



static void
fmb_sbr_number_renamer_update (FmbSbrNumberRenamer *number_renamer)
{
  gboolean invalid = TRUE;
  GdkColor back;
  GdkColor text;
  gchar   *endp;

  /* check whether "start" is valid for the "mode" */
  if (number_renamer->mode < FMB_SBR_NUMBER_MODE_ABC)
    {
      /* "start" must be a positive number */
      strtoul (number_renamer->start, &endp, 10);
      invalid = (endp <= number_renamer->start || *endp != '\0');
    }
  else if (number_renamer->mode == FMB_SBR_NUMBER_MODE_ABC)
    {
      /* "start" property must be 'a', 'b', 'c', etc. */
      invalid = (strlen (number_renamer->start) != 1
              || g_ascii_tolower (*number_renamer->start) < 'a'
              || g_ascii_tolower (*number_renamer->start) > 'z');
    }

  /* check if the start entry is realized */
  if (gtk_widget_get_realized (number_renamer->start_entry))
    {
      /* check if the "start" value is valid */
      if (G_UNLIKELY (invalid))
        {
          /* if GTK+ wouldn't be that stupid with style properties and 
           * type plugins, this would be themable, but unfortunately
           * GTK+ is totally broken, and so it's hardcoded.
           */
          gdk_color_parse ("#ff6666", &back);
          gdk_color_parse ("White", &text);

          /* setup a red background/text color to indicate the error */
          gtk_widget_modify_base (number_renamer->start_entry, GTK_STATE_NORMAL, &back);
          gtk_widget_modify_text (number_renamer->start_entry, GTK_STATE_NORMAL, &text);
        }
      else
        {
          /* reset background/text color */
          gtk_widget_modify_base (number_renamer->start_entry, GTK_STATE_NORMAL, NULL);
          gtk_widget_modify_text (number_renamer->start_entry, GTK_STATE_NORMAL, NULL);
        }
    }

  /* notify everybody that we have a new state */
  fmbx_renamer_changed (FMBX_RENAMER (number_renamer));
}



/**
 * fmb_sbr_number_renamer_new:
 *
 * Allocates a new #FmbSbrNumberRenamer instance.
 *
 * Return value: the newly allocated #FmbSbrNumberRenamer.
 **/
FmbSbrNumberRenamer*
fmb_sbr_number_renamer_new (void)
{
  return g_object_new (FMB_SBR_TYPE_NUMBER_RENAMER,
                       "name", _("Numbering"),
                       NULL);
}



/**
 * fmb_sbr_number_renamer_get_mode:
 * @number_renamer : a #FmbSbrNumberRenamer.
 *
 * Returns the mode of the @number_renamer.
 *
 * Return value: the mode of @number_renamer.
 **/
FmbSbrNumberMode
fmb_sbr_number_renamer_get_mode (FmbSbrNumberRenamer *number_renamer)
{
  g_return_val_if_fail (FMB_SBR_IS_NUMBER_RENAMER (number_renamer), FMB_SBR_NUMBER_MODE_123);
  return number_renamer->mode;
}



/**
 * fmb_sbr_number_renamer_set_mode:
 * @number_renamer : a #FmbSbrNumberRenamer.
 * @mode           : the new #FmbSbrNumberMode for @number_renamer.
 *
 * Sets the mode of @number_renamer to @mode.
 **/
void
fmb_sbr_number_renamer_set_mode (FmbSbrNumberRenamer *number_renamer,
                                    FmbSbrNumberMode     mode)
{
  g_return_if_fail (FMB_SBR_IS_NUMBER_RENAMER (number_renamer));

  /* check if we have a new mode */
  if (G_LIKELY (number_renamer->mode != mode))
    {
      /* apply the new mode */
      number_renamer->mode = mode;

      /* update the renamer */
      fmb_sbr_number_renamer_update (number_renamer);

      /* notify listeners */
      g_object_notify (G_OBJECT (number_renamer), "mode");
    }
}



/**
 * fmb_sbr_number_renamer_get_start:
 * @number_renamer : a #FmbSbrNumberRenamer.
 *
 * Returns the start for the @number_renamer.
 *
 * Return value: the start for @number_renamer.
 **/
const gchar*
fmb_sbr_number_renamer_get_start (FmbSbrNumberRenamer *number_renamer)
{
  g_return_val_if_fail (FMB_SBR_IS_NUMBER_RENAMER (number_renamer), NULL);
  return number_renamer->start;
}



/**
 * fmb_sbr_number_renamer_set_start:
 * @number_renamer : a #FmbSbrNumberRenamer.
 * @start          : the new start for @number_renamer.
 *
 * Sets the start for @number_renamer to @start.
 **/
void
fmb_sbr_number_renamer_set_start (FmbSbrNumberRenamer *number_renamer,
                                     const gchar            *start)
{
  g_return_if_fail (FMB_SBR_IS_NUMBER_RENAMER (number_renamer));

  /* check if we have a new start */
  if (!blxo_str_is_equal (number_renamer->start, start))
    {
      /* apply the new start */
      g_free (number_renamer->start);
      number_renamer->start = g_strdup (start);

      /* update the renamer */
      fmb_sbr_number_renamer_update (number_renamer);

      /* notify listeners */
      g_object_notify (G_OBJECT (number_renamer), "start");
    }
}



/**
 * fmb_sbr_number_renamer_get_text:
 * @number_renamer : a #FmbSbrNumberRenamer.
 *
 * Returns the text for the @number_renamer.
 *
 * Return value: the text for @number_renamer.
 **/
const gchar*
fmb_sbr_number_renamer_get_text (FmbSbrNumberRenamer *number_renamer)
{
  g_return_val_if_fail (FMB_SBR_IS_NUMBER_RENAMER (number_renamer), NULL);
  return number_renamer->text;
}



/**
 * fmb_sbr_number_renamer_set_text:
 * @number_renamer : a #FmbSbrNumberRenamer.
 * @text           : the new text for @number_renamer.
 *
 * Sets the text for @number_renamer to @text.
 **/
void
fmb_sbr_number_renamer_set_text (FmbSbrNumberRenamer *number_renamer,
                                    const gchar            *text)
{
  g_return_if_fail (FMB_SBR_IS_NUMBER_RENAMER (number_renamer));

  /* check if we have a new text */
  if (G_LIKELY (!blxo_str_is_equal (number_renamer->text, text)))
    {
      /* apply the new text */
      g_free (number_renamer->text);
      number_renamer->text = g_strdup (text);

      /* update the renamer */
      fmb_sbr_number_renamer_update (number_renamer);

      /* notify listeners */
      g_object_notify (G_OBJECT (number_renamer), "text");
    }
}



/**
 * fmb_sbr_number_renamer_get_text_mode:
 * @number_renamer : a #FmbSbrNumberRenamer.
 *
 * Returns the text mode for the @number_renamer.
 *
 * Return value: the text mode for @number_renamer.
 **/
FmbSbrTextMode
fmb_sbr_number_renamer_get_text_mode (FmbSbrNumberRenamer *number_renamer)
{
  g_return_val_if_fail (FMB_SBR_IS_NUMBER_RENAMER (number_renamer), FMB_SBR_TEXT_MODE_NTO);
  return number_renamer->text_mode;
}



/**
 * fmb_sbr_number_renamer_set_offset_mode:
 * @number_renamer : a #FmbSbrNumberRenamer.
 * @text_mode      : the new text mode for @number_renamer.
 *
 * Sets the text mode for @number_renamer to @text_mode.
 **/
void
fmb_sbr_number_renamer_set_text_mode (FmbSbrNumberRenamer *number_renamer,
                                         FmbSbrTextMode       text_mode)
{
  g_return_if_fail (FMB_SBR_IS_NUMBER_RENAMER (number_renamer));

  /* check if we have a new setting */
  if (G_LIKELY (number_renamer->text_mode != text_mode))
    {
      /* apply the new setting */
      number_renamer->text_mode = text_mode;

      /* update the renamer */
      fmb_sbr_number_renamer_update (number_renamer);

      /* notify listeners */
      g_object_notify (G_OBJECT (number_renamer), "text-mode");
    }
}





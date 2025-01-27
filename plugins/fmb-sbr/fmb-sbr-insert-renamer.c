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

#include <blxo/blxo.h>

#include <fmb-sbr/fmb-sbr-insert-renamer.h>



/* Property identifiers */
enum
{
  PROP_0,
  PROP_MODE,
  PROP_OFFSET,
  PROP_OFFSET_MODE,
  PROP_TEXT,
};



static void   fmb_sbr_insert_renamer_finalize      (GObject                      *object);
static void   fmb_sbr_insert_renamer_get_property  (GObject                      *object,
                                                       guint                         prop_id,
                                                       GValue                       *value,
                                                       GParamSpec                   *pspec);
static void   fmb_sbr_insert_renamer_set_property  (GObject                      *object,
                                                       guint                         prop_id,
                                                       const GValue                 *value,
                                                       GParamSpec                   *pspec);
static gchar *fmb_sbr_insert_renamer_process       (FmbxRenamer               *renamer,
                                                       FmbxFileInfo              *file,
                                                       const gchar                  *text,
                                                       guint                         idx);



struct _FmbSbrInsertRenamerClass
{
  FmbxRenamerClass __parent__;
};

struct _FmbSbrInsertRenamer
{
  FmbxRenamer      __parent__;
  FmbSbrInsertMode mode;
  guint               offset;
  FmbSbrOffsetMode offset_mode;
  gchar              *text;
};



FMBX_DEFINE_TYPE (FmbSbrInsertRenamer, fmb_sbr_insert_renamer, FMBX_TYPE_RENAMER);



static void
fmb_sbr_insert_renamer_class_init (FmbSbrInsertRenamerClass *klass)
{
  FmbxRenamerClass *fmbxrenamer_class;
  GObjectClass        *gobject_class;

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = fmb_sbr_insert_renamer_finalize;
  gobject_class->get_property = fmb_sbr_insert_renamer_get_property;
  gobject_class->set_property = fmb_sbr_insert_renamer_set_property;

  fmbxrenamer_class = FMBX_RENAMER_CLASS (klass);
  fmbxrenamer_class->process = fmb_sbr_insert_renamer_process;

  /**
   * FmbSbrInsertRenamer:mode:
   *
   * The #FmbSbrInsertMode to use.
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_MODE,
                                   g_param_spec_enum ("mode", "mode", "mode",
                                                      FMB_SBR_TYPE_INSERT_MODE,
                                                      FMB_SBR_INSERT_MODE_INSERT,
                                                      G_PARAM_READWRITE));

  /**
   * FmbSbrInsertRenamer:offset:
   *
   * The starting offset at which to insert/overwrite.
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_OFFSET,
                                   g_param_spec_uint ("offset",
                                                      "offset",
                                                      "offset",
                                                      0, G_MAXUINT, 1,
                                                      G_PARAM_READWRITE));

  /**
   * FmbSbrInsertRenamer:offset-mode:
   *
   * The offset mode for the renamer.
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_OFFSET_MODE,
                                   g_param_spec_enum ("offset-mode",
                                                      "offset-mode",
                                                      "offset-mode",
                                                      FMB_SBR_TYPE_OFFSET_MODE,
                                                      FMB_SBR_OFFSET_MODE_LEFT,
                                                      G_PARAM_READWRITE));

  /**
   * FmbSbrInsertRenamer:text:
   *
   * The text to insert/overwrite.
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_TEXT,
                                   g_param_spec_string ("text",
                                                        "text",
                                                        "text",
                                                        NULL,
                                                        G_PARAM_READWRITE));
}



static void
fmb_sbr_insert_renamer_init (FmbSbrInsertRenamer *insert_renamer)
{
  AtkRelationSet *relations;
  GtkAdjustment  *adjustment;
  AtkRelation    *relation;
  GEnumClass     *klass;
  AtkObject      *object;
  GtkWidget      *spinner;
  GtkWidget      *combo;
  GtkWidget      *entry;
  GtkWidget      *label;
  GtkWidget      *table;
  GtkWidget      *hbox;
  guint           n;

  table = gtk_table_new (2, 3, FALSE);
  gtk_table_set_row_spacings (GTK_TABLE (table), 6);
  gtk_table_set_col_spacings (GTK_TABLE (table), 12);
  gtk_box_pack_start (GTK_BOX (insert_renamer), table, TRUE, TRUE, 0);
  gtk_widget_show (table);

  combo = gtk_combo_box_new_text ();
  klass = g_type_class_ref (FMB_SBR_TYPE_INSERT_MODE);
  for (n = 0; n < klass->n_values; ++n)
    gtk_combo_box_append_text (GTK_COMBO_BOX (combo), _(klass->values[n].value_nick));
  blxo_mutual_binding_new (G_OBJECT (insert_renamer), "mode", G_OBJECT (combo), "active");
  gtk_table_attach (GTK_TABLE (table), combo, 0, 1, 0, 1, GTK_FILL, 0, 0, 0);
  g_type_class_unref (klass);
  gtk_widget_show (combo);

  label = gtk_label_new_with_mnemonic (_("_Text:"));
  gtk_table_attach (GTK_TABLE (table), label, 1, 2, 0, 1, GTK_FILL, 0, 0, 0);
  gtk_widget_show (label);

  entry = gtk_entry_new ();
  gtk_entry_set_activates_default (GTK_ENTRY (entry), TRUE);
  blxo_mutual_binding_new (G_OBJECT (entry), "text", G_OBJECT (insert_renamer), "text");
  gtk_table_attach (GTK_TABLE (table), entry, 2, 3, 0, 1, GTK_EXPAND | GTK_FILL, 0, 0, 0);
  gtk_label_set_mnemonic_widget (GTK_LABEL (label), entry);
  gtk_widget_show (entry);

  /* set Atk label relation for the entry */
  object = gtk_widget_get_accessible (entry);
  relations = atk_object_ref_relation_set (gtk_widget_get_accessible (label));
  relation = atk_relation_new (&object, 1, ATK_RELATION_LABEL_FOR);
  atk_relation_set_add (relations, relation);
  g_object_unref (G_OBJECT (relation));

  label = gtk_label_new_with_mnemonic (_("_At position:"));
  gtk_misc_set_alignment (GTK_MISC (label), 1.0f, 0.5f);
  gtk_table_attach (GTK_TABLE (table), label, 0, 2, 1, 2, GTK_FILL, 0, 0, 0);
  gtk_widget_show (label);

  hbox = gtk_hbox_new (FALSE, 12);
  gtk_table_attach (GTK_TABLE (table), hbox, 2, 3, 1, 2, GTK_FILL, 0, 0, 0);
  gtk_widget_show (hbox);

  spinner = gtk_spin_button_new_with_range (0u, G_MAXUINT, 1u);
  gtk_entry_set_width_chars (GTK_ENTRY (spinner), 4);
  gtk_entry_set_alignment (GTK_ENTRY (spinner), 1.0f);
  gtk_entry_set_activates_default (GTK_ENTRY (spinner), TRUE);
  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (spinner), 0);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (spinner), TRUE);
  gtk_spin_button_set_snap_to_ticks (GTK_SPIN_BUTTON (spinner), TRUE);
  gtk_box_pack_start (GTK_BOX (hbox), spinner, FALSE, FALSE, 0);
  gtk_label_set_mnemonic_widget (GTK_LABEL (label), spinner);
  gtk_widget_show (spinner);

  adjustment = gtk_spin_button_get_adjustment (GTK_SPIN_BUTTON (spinner));
  blxo_mutual_binding_new (G_OBJECT (insert_renamer), "offset", G_OBJECT (adjustment), "value");

  /* set Atk label relation for the entry */
  object = gtk_widget_get_accessible (spinner);
  relations = atk_object_ref_relation_set (gtk_widget_get_accessible (label));
  relation = atk_relation_new (&object, 1, ATK_RELATION_LABEL_FOR);
  atk_relation_set_add (relations, relation);
  g_object_unref (G_OBJECT (relation));

  combo = gtk_combo_box_new_text ();
  klass = g_type_class_ref (FMB_SBR_TYPE_OFFSET_MODE);
  for (n = 0; n < klass->n_values; ++n)
    gtk_combo_box_append_text (GTK_COMBO_BOX (combo), _(klass->values[n].value_nick));
  blxo_mutual_binding_new (G_OBJECT (insert_renamer), "offset-mode", G_OBJECT (combo), "active");
  gtk_box_pack_start (GTK_BOX (hbox), combo, FALSE, FALSE, 0);
  g_type_class_unref (klass);
  gtk_widget_show (combo);
}



static void
fmb_sbr_insert_renamer_finalize (GObject *object)
{
  FmbSbrInsertRenamer *insert_renamer = FMB_SBR_INSERT_RENAMER (object);

  /* release the text */
  g_free (insert_renamer->text);

  (*G_OBJECT_CLASS (fmb_sbr_insert_renamer_parent_class)->finalize) (object);
}



static void
fmb_sbr_insert_renamer_get_property (GObject    *object,
                                        guint       prop_id,
                                        GValue     *value,
                                        GParamSpec *pspec)
{
  FmbSbrInsertRenamer *insert_renamer = FMB_SBR_INSERT_RENAMER (object);

  switch (prop_id)
    {
    case PROP_MODE:
      g_value_set_enum (value, fmb_sbr_insert_renamer_get_mode (insert_renamer));
      break;

    case PROP_OFFSET:
      g_value_set_uint (value, fmb_sbr_insert_renamer_get_offset (insert_renamer));
      break;

    case PROP_OFFSET_MODE:
      g_value_set_enum (value, fmb_sbr_insert_renamer_get_offset_mode (insert_renamer));
      break;

    case PROP_TEXT:
      g_value_set_string (value, fmb_sbr_insert_renamer_get_text (insert_renamer));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static void
fmb_sbr_insert_renamer_set_property (GObject      *object,
                                        guint         prop_id,
                                        const GValue *value,
                                        GParamSpec   *pspec)
{
  FmbSbrInsertRenamer *insert_renamer = FMB_SBR_INSERT_RENAMER (object);

  switch (prop_id)
    {
    case PROP_MODE:
      fmb_sbr_insert_renamer_set_mode (insert_renamer, g_value_get_enum (value));
      break;

    case PROP_OFFSET:
      fmb_sbr_insert_renamer_set_offset (insert_renamer, g_value_get_uint (value));
      break;

    case PROP_OFFSET_MODE:
      fmb_sbr_insert_renamer_set_offset_mode (insert_renamer, g_value_get_enum (value));
      break;

    case PROP_TEXT:
      fmb_sbr_insert_renamer_set_text (insert_renamer, g_value_get_string (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static gchar*
fmb_sbr_insert_renamer_process (FmbxRenamer  *renamer,
                                   FmbxFileInfo *file,
                                   const gchar     *text,
                                   guint            idx)
{
  FmbSbrInsertRenamer *insert_renamer = FMB_SBR_INSERT_RENAMER (renamer);
  const gchar            *t;
  const gchar            *s;
  GString                *result;
  guint                   text_length;
  guint                   offset;

  /* check if we have any text to insert/overwrite */
  if (G_UNLIKELY (insert_renamer->text == NULL || *insert_renamer->text == '\0'))
    return g_strdup (text);

  /* determine the input text length */
  text_length = g_utf8_strlen (text, -1);

  /* determine the real offset and check if it's valid */
  offset = (insert_renamer->offset_mode == FMB_SBR_OFFSET_MODE_LEFT) ? insert_renamer->offset : (text_length - insert_renamer->offset);
  if (G_UNLIKELY (offset > text_length))
    return g_strdup (text);

  /* allocate space for the result */
  result = g_string_sized_new (2 * text_length);

  /* determine the text pointer for the offset */
  s = g_utf8_offset_to_pointer (text, offset);

  /* add the text before the insert/overwrite offset */
  g_string_append_len (result, text, s - text);

  /* add the text to insert */
  g_string_append (result, insert_renamer->text);

  /* skip over the input text if overwriting */
  if (insert_renamer->mode == FMB_SBR_INSERT_MODE_OVERWRITE)
    {
      /* skip over the number of characters in the overwrite text */
      for (t = insert_renamer->text; *s != '\0' && *t != '\0'; s = g_utf8_next_char (s), t = g_utf8_next_char (t))
        ;
    }

  /* append the remaining text */
  g_string_append (result, s);

  /* return the result */
  return g_string_free (result, FALSE);
}



/**
 * fmb_sbr_insert_renamer_new:
 *
 * Allocates a new #FmbSbrInsertRenamer instance.
 *
 * Return value: the newly allocated #FmbSbrInsertRenamer.
 **/
FmbSbrInsertRenamer*
fmb_sbr_insert_renamer_new (void)
{
  return g_object_new (FMB_SBR_TYPE_INSERT_RENAMER,
                       "name", _("Insert / Overwrite"),
                       NULL);
}



/**
 * fmb_sbr_insert_renamer_get_mode:
 * @insert_renamer : a #FmbSbrInsertRenamer.
 *
 * Returns the mode of the @insert_renamer.
 *
 * Return value: the mode of @insert_renamer.
 **/
FmbSbrInsertMode
fmb_sbr_insert_renamer_get_mode (FmbSbrInsertRenamer *insert_renamer)
{
  g_return_val_if_fail (FMB_SBR_IS_INSERT_RENAMER (insert_renamer), FMB_SBR_INSERT_MODE_INSERT);
  return insert_renamer->mode;
}



/**
 * fmb_sbr_insert_renamer_set_mode:
 * @insert_renamer : a #FmbSbrInsertRenamer.
 * @mode           : the new #FmbSbrInsertMode for @insert_renamer.
 *
 * Sets the mode of @insert_renamer to @mode.
 **/
void
fmb_sbr_insert_renamer_set_mode (FmbSbrInsertRenamer *insert_renamer,
                                    FmbSbrInsertMode     mode)
{
  g_return_if_fail (FMB_SBR_IS_INSERT_RENAMER (insert_renamer));

  /* check if we have a new mode */
  if (G_LIKELY (insert_renamer->mode != mode))
    {
      /* apply the new mode */
      insert_renamer->mode = mode;

      /* update the renamer */
      fmbx_renamer_changed (FMBX_RENAMER (insert_renamer));

      /* notify listeners */
      g_object_notify (G_OBJECT (insert_renamer), "mode");
    }
}



/**
 * fmb_sbr_insert_renamer_get_offset:
 * @insert_renamer : a #FmbSbrInsertRenamer.
 *
 * Returns the offset for the @insert_renamer.
 *
 * Return value: the offset for @insert_renamer.
 **/
guint
fmb_sbr_insert_renamer_get_offset (FmbSbrInsertRenamer *insert_renamer)
{
  g_return_val_if_fail (FMB_SBR_IS_INSERT_RENAMER (insert_renamer), 0);
  return insert_renamer->offset;
}



/**
 * fmb_sbr_insert_renamer_set_offset:
 * @insert_renamer : a #FmbSbrInsertRenamer.
 * @offset         : the new offset for @insert_renamer.
 *
 * Sets the offset for @insert_renamer to @offset.
 **/
void
fmb_sbr_insert_renamer_set_offset (FmbSbrInsertRenamer *insert_renamer,
                                      guint                   offset)
{
  g_return_if_fail (FMB_SBR_IS_INSERT_RENAMER (insert_renamer));

  /* check if we have a new offset */
  if (G_LIKELY (insert_renamer->offset != offset))
    {
      /* apply the new offset */
      insert_renamer->offset = offset;

      /* update the renamer */
      fmbx_renamer_changed (FMBX_RENAMER (insert_renamer));

      /* notify listeners */
      g_object_notify (G_OBJECT (insert_renamer), "offset");
    }
}



/**
 * fmb_sbr_insert_renamer_get_offset_mode:
 * @insert_renamer : a #FmbSbrInsertRenamer.
 *
 * Returns the offset mode for the @insert_renamer.
 *
 * Return value: the offset mode for @insert_renamer.
 **/
FmbSbrOffsetMode
fmb_sbr_insert_renamer_get_offset_mode (FmbSbrInsertRenamer *insert_renamer)
{
  g_return_val_if_fail (FMB_SBR_IS_INSERT_RENAMER (insert_renamer), FMB_SBR_OFFSET_MODE_LEFT);
  return insert_renamer->offset_mode;
}



/**
 * fmb_sbr_insert_renamer_set_offset_mode:
 * @insert_renamer : a #FmbSbrInsertRenamer.
 * @offset_mode    : the new offset mode for @insert_renamer.
 *
 * Sets the offset mode for @insert_renamer to @offset_mode.
 **/
void
fmb_sbr_insert_renamer_set_offset_mode (FmbSbrInsertRenamer *insert_renamer,
                                           FmbSbrOffsetMode     offset_mode)
{
  g_return_if_fail (FMB_SBR_IS_INSERT_RENAMER (insert_renamer));

  /* check if we have a new setting */
  if (G_LIKELY (insert_renamer->offset_mode != offset_mode))
    {
      /* apply the new setting */
      insert_renamer->offset_mode = offset_mode;

      /* update the renamer */
      fmbx_renamer_changed (FMBX_RENAMER (insert_renamer));

      /* notify listeners */
      g_object_notify (G_OBJECT (insert_renamer), "offset-mode");
    }
}



/**
 * fmb_sbr_insert_renamer_get_text:
 * @insert_renamer : a #FmbSbrInsertRenamer.
 *
 * Returns the text for the @insert_renamer.
 *
 * Return value: the text for @insert_renamer.
 **/
const gchar*
fmb_sbr_insert_renamer_get_text (FmbSbrInsertRenamer *insert_renamer)
{
  g_return_val_if_fail (FMB_SBR_IS_INSERT_RENAMER (insert_renamer), NULL);
  return insert_renamer->text;
}



/**
 * fmb_sbr_insert_renamer_set_text:
 * @insert_renamer : a #FmbSbrInsertRenamer.
 * @text           : the new text for @insert_renamer.
 *
 * Sets the text for @insert_renamer to @text.
 **/
void
fmb_sbr_insert_renamer_set_text (FmbSbrInsertRenamer *insert_renamer,
                                    const gchar            *text)
{
  g_return_if_fail (FMB_SBR_IS_INSERT_RENAMER (insert_renamer));

  /* check if we have a new text */
  if (G_LIKELY (!blxo_str_is_equal (insert_renamer->text, text)))
    {
      /* apply the new text */
      g_free (insert_renamer->text);
      insert_renamer->text = g_strdup (text);

      /* update the renamer */
      fmbx_renamer_changed (FMBX_RENAMER (insert_renamer));

      /* notify listeners */
      g_object_notify (G_OBJECT (insert_renamer), "text");
    }
}



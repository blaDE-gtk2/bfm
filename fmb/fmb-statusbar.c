/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2005-2006 Benedikt Meurer <benny@xfce.org>
 * Copyright (c) 2011 Jannis Pohlmann <jannis@xfce.org>
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

#include <blxo/blxo.h>

#include <fmb/fmb-gobject-extensions.h>
#include <fmb/fmb-private.h>
#include <fmb/fmb-statusbar.h>



enum
{
  PROP_0,
  PROP_TEXT,
};



static void        fmb_statusbar_set_property          (GObject              *object,
                                                           guint                 prop_id,
                                                           const GValue         *value,
                                                           GParamSpec           *pspec);
static void        fmb_statusbar_set_text              (FmbStatusbar      *statusbar,
                                                           const gchar          *text);


struct _FmbStatusbarClass
{
  GtkStatusbarClass __parent__;
};

struct _FmbStatusbar
{
  GtkStatusbar __parent__;
  guint        context_id;
};



G_DEFINE_TYPE (FmbStatusbar, fmb_statusbar, GTK_TYPE_STATUSBAR)



static void
fmb_statusbar_class_init (FmbStatusbarClass *klass)
{
  static gboolean style_initialized = FALSE;

  GObjectClass *gobject_class;

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->set_property = fmb_statusbar_set_property;

  /**
   * FmbStatusbar:text:
   *
   * The main text to be displayed in the statusbar. This property
   * can only be written.
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_TEXT,
                                   g_param_spec_string ("text",
                                                        "text",
                                                        "text",
                                                        NULL,
                                                        BLXO_PARAM_WRITABLE));

  if (!style_initialized)
    {
      gtk_rc_parse_string ("style \"fmb-statusbar-internal\" {\n"
                           "  GtkStatusbar::shadow-type = GTK_SHADOW_NONE\n"
                           "}\n"
                           "class \"FmbStatusbar\" "
                           "style \"fmb-statusbar-internal\"\n");
    }
}



static void
fmb_statusbar_init (FmbStatusbar *statusbar)
{
  statusbar->context_id = gtk_statusbar_get_context_id (GTK_STATUSBAR (statusbar), "Main text");
  gtk_statusbar_set_has_resize_grip (GTK_STATUSBAR (statusbar), TRUE);
}



static void
fmb_statusbar_set_property (GObject      *object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  FmbStatusbar *statusbar = FMB_STATUSBAR (object);

  switch (prop_id)
    {
    case PROP_TEXT:
      fmb_statusbar_set_text (statusbar, g_value_get_string (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



/**
 * fmb_statusbar_set_text:
 * @statusbar : a #FmbStatusbar instance.
 * @text      : the main text to be displayed in @statusbar.
 *
 * Sets up a new main text for @statusbar.
 **/
static void
fmb_statusbar_set_text (FmbStatusbar *statusbar,
                           const gchar     *text)
{
  _fmb_return_if_fail (FMB_IS_STATUSBAR (statusbar));
  _fmb_return_if_fail (text != NULL);

  gtk_statusbar_pop (GTK_STATUSBAR (statusbar), statusbar->context_id);
  gtk_statusbar_push (GTK_STATUSBAR (statusbar), statusbar->context_id, text);
}



/**
 * fmb_statusbar_new:
 *
 * Allocates a new #FmbStatusbar instance with no
 * text set.
 *
 * Return value: the newly allocated #FmbStatusbar instance.
 **/
GtkWidget*
fmb_statusbar_new (void)
{
  return g_object_new (FMB_TYPE_STATUSBAR, NULL);
}

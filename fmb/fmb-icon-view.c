/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2005-2006 Benedikt Meurer <benny@xfce.org>
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

#include <fmb/fmb-icon-view.h>
#include <fmb/fmb-private.h>



/* Property identifiers */
enum
{
  PROP_0,
  PROP_TEXT_BESIDE_ICONS,
};



static void         fmb_icon_view_set_property           (GObject             *object,
                                                             guint                prop_id,
                                                             const GValue        *value,
                                                             GParamSpec          *pspec);
static AtkObject   *fmb_icon_view_get_accessible         (GtkWidget           *widget);
static void         fmb_icon_view_zoom_level_changed     (FmbStandardView  *standard_view);



struct _FmbIconViewClass
{
  FmbAbstractIconViewClass __parent__;
};

struct _FmbIconView
{
  FmbAbstractIconView __parent__;
};



G_DEFINE_TYPE (FmbIconView, fmb_icon_view, FMB_TYPE_ABSTRACT_ICON_VIEW)



static void
fmb_icon_view_class_init (FmbIconViewClass *klass)
{
  FmbStandardViewClass *fmbstandard_view_class;
  GtkWidgetClass          *gtkwidget_class;
  GObjectClass            *gobject_class;

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->set_property = fmb_icon_view_set_property;

  gtkwidget_class = GTK_WIDGET_CLASS (klass);
  gtkwidget_class->get_accessible = fmb_icon_view_get_accessible;

  fmbstandard_view_class = FMB_STANDARD_VIEW_CLASS (klass);
  fmbstandard_view_class->zoom_level_property_name = "last-icon-view-zoom-level";

  /**
   * FmbIconView::text-beside-icons:
   *
   * Write-only property to specify whether text should be
   * display besides the icon rather than below.
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_TEXT_BESIDE_ICONS,
                                   g_param_spec_boolean ("text-beside-icons",
                                                         "text-beside-icons",
                                                         "text-beside-icons",
                                                         FALSE,
                                                         BLXO_PARAM_WRITABLE));
}



static void
fmb_icon_view_init (FmbIconView *icon_view)
{
  /* setup the icon renderer */
  g_object_set (G_OBJECT (FMB_STANDARD_VIEW (icon_view)->icon_renderer),
                "ypad", 3u,
                NULL);

  /* setup the name renderer */
  g_object_set (G_OBJECT (FMB_STANDARD_VIEW (icon_view)->name_renderer),
                "wrap-mode", PANGO_WRAP_WORD_CHAR,
                NULL);

  /* synchronize the "text-beside-icons" property with the global preference */
  blxo_binding_new (G_OBJECT (FMB_STANDARD_VIEW (icon_view)->preferences), "misc-text-beside-icons", G_OBJECT (icon_view), "text-beside-icons");
}



static void
fmb_icon_view_set_property (GObject      *object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  FmbStandardView *standard_view = FMB_STANDARD_VIEW (object);

  switch (prop_id)
    {
    case PROP_TEXT_BESIDE_ICONS:
      if (G_UNLIKELY (g_value_get_boolean (value)))
        {
          blxo_icon_view_set_orientation (BLXO_ICON_VIEW (GTK_BIN (standard_view)->child), GTK_ORIENTATION_HORIZONTAL);
          g_object_set (G_OBJECT (standard_view->name_renderer), "wrap-width", 128, "yalign", 0.5f, "alignment", PANGO_ALIGN_LEFT, NULL);

          /* disconnect the "zoom-level" signal handler, since we're using a fixed wrap-width here */
          g_signal_handlers_disconnect_by_func (object, fmb_icon_view_zoom_level_changed, NULL);
        }
      else
        {
          blxo_icon_view_set_orientation (BLXO_ICON_VIEW (GTK_BIN (standard_view)->child), GTK_ORIENTATION_VERTICAL);
          g_object_set (G_OBJECT (standard_view->name_renderer), "yalign", 0.0f, "alignment", PANGO_ALIGN_CENTER, NULL);

          /* connect the "zoom-level" signal handler as the wrap-width is now synced with the "zoom-level" */
          g_signal_connect (object, "notify::zoom-level", G_CALLBACK (fmb_icon_view_zoom_level_changed), NULL);
          fmb_icon_view_zoom_level_changed (standard_view);
        }
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static AtkObject*
fmb_icon_view_get_accessible (GtkWidget *widget)
{
  AtkObject *object;

  /* query the atk object for the icon view class */
  object = (*GTK_WIDGET_CLASS (fmb_icon_view_parent_class)->get_accessible) (widget);

  /* set custom Atk properties for the icon view */
  if (G_LIKELY (object != NULL))
    {
      atk_object_set_description (object, _("Icon based directory listing"));
      atk_object_set_name (object, _("Icon view"));
      atk_object_set_role (object, ATK_ROLE_DIRECTORY_PANE);
    }

  return object;
}



static void
fmb_icon_view_zoom_level_changed (FmbStandardView *standard_view)
{
  gint wrap_width;

  _fmb_return_if_fail (FMB_IS_STANDARD_VIEW (standard_view));

  /* determine the "wrap-width" depending on the "zoom-level" */
  switch (fmb_view_get_zoom_level (FMB_VIEW (standard_view)))
    {
    case FMB_ZOOM_LEVEL_SMALLEST:
      wrap_width = 48;
      break;

    case FMB_ZOOM_LEVEL_SMALLER:
      wrap_width = 64;
      break;

    case FMB_ZOOM_LEVEL_SMALL:
      wrap_width = 72;
      break;

    case FMB_ZOOM_LEVEL_NORMAL:
      wrap_width = 112;
      break;

    default:
      wrap_width = 128;
      break;
    }

  /* set the new "wrap-width" for the text renderer */
  g_object_set (G_OBJECT (standard_view->name_renderer), "wrap-width", wrap_width, NULL);
}





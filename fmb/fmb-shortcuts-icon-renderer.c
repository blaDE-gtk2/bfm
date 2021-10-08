/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2005-2006 Benedikt Meurer <benny@xfce.org>
 * Copyright (c) 2009 Jannis Pohlmann <jannis@xfce.org>
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

#include <gio/gio.h>

#include <fmb/fmb-gio-extensions.h>
#include <fmb/fmb-gobject-extensions.h>
#include <fmb/fmb-gdk-extensions.h>
#include <fmb/fmb-icon-factory.h>
#include <fmb/fmb-shortcuts-icon-renderer.h>
#include <fmb/fmb-device.h>



/* Property identifiers */
enum
{
  PROP_0,
  PROP_DEVICE,
  PROP_GICON,
};



static void fmb_shortcuts_icon_renderer_finalize     (GObject                          *object);
static void fmb_shortcuts_icon_renderer_get_property (GObject                          *object,
                                                         guint                             prop_id,
                                                         GValue                           *value,
                                                         GParamSpec                       *pspec);
static void fmb_shortcuts_icon_renderer_set_property (GObject                          *object,
                                                         guint                             prop_id,
                                                         const GValue                     *value,
                                                         GParamSpec                       *pspec);
static void fmb_shortcuts_icon_renderer_render       (GtkCellRenderer                  *renderer,
                                                         GdkWindow                        *window,
                                                         GtkWidget                        *widget,
                                                         GdkRectangle                     *background_area,
                                                         GdkRectangle                     *cell_area,
                                                         GdkRectangle                     *expose_area,
                                                         GtkCellRendererState              flags);



struct _FmbShortcutsIconRendererClass
{
  FmbIconRendererClass __parent__;
};

struct _FmbShortcutsIconRenderer
{
  FmbIconRenderer __parent__;

  FmbDevice      *device;
  GIcon             *gicon;
};



G_DEFINE_TYPE (FmbShortcutsIconRenderer, fmb_shortcuts_icon_renderer, FMB_TYPE_ICON_RENDERER)



static void
fmb_shortcuts_icon_renderer_class_init (FmbShortcutsIconRendererClass *klass)
{
  GtkCellRendererClass *gtkcell_renderer_class;
  GObjectClass         *gobject_class;

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = fmb_shortcuts_icon_renderer_finalize;
  gobject_class->get_property = fmb_shortcuts_icon_renderer_get_property;
  gobject_class->set_property = fmb_shortcuts_icon_renderer_set_property;

  gtkcell_renderer_class = GTK_CELL_RENDERER_CLASS (klass);
  gtkcell_renderer_class->render = fmb_shortcuts_icon_renderer_render;

  /**
   * FmbShortcutsIconRenderer:device:
   *
   * The #FmbDevice for which to render an icon or %NULL to fallback
   * to the default icon renderering (see #FmbIconRenderer).
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_DEVICE,
                                   g_param_spec_object ("device", "device", "device",
                                                        FMB_TYPE_DEVICE,
                                                        BLXO_PARAM_READWRITE));

  /**
   * FmbIconRenderer:gicon:
   *
   * The GIcon to render, this property has preference over the the icon returned
   * by the FmbFile property.
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_GICON,
                                   g_param_spec_object ("gicon", "gicon", "gicon",
                                                        G_TYPE_ICON,
                                                        BLXO_PARAM_READWRITE));
}



static void
fmb_shortcuts_icon_renderer_init (FmbShortcutsIconRenderer *shortcuts_icon_renderer)
{
  /* no padding please */
  GTK_CELL_RENDERER (shortcuts_icon_renderer)->xpad = 0;
  GTK_CELL_RENDERER (shortcuts_icon_renderer)->ypad = 0;
}



static void
fmb_shortcuts_icon_renderer_finalize (GObject *object)
{
  FmbShortcutsIconRenderer *renderer = FMB_SHORTCUTS_ICON_RENDERER (object);

  if (G_UNLIKELY (renderer->device != NULL))
    g_object_unref (renderer->device);

  if (G_UNLIKELY (renderer->gicon != NULL))
    g_object_unref (renderer->gicon);

  (*G_OBJECT_CLASS (fmb_shortcuts_icon_renderer_parent_class)->finalize) (object);
}



static void
fmb_shortcuts_icon_renderer_get_property (GObject    *object,
                                             guint       prop_id,
                                             GValue     *value,
                                             GParamSpec *pspec)
{
  FmbShortcutsIconRenderer *renderer = FMB_SHORTCUTS_ICON_RENDERER (object);

  switch (prop_id)
    {
    case PROP_DEVICE:
      g_value_set_object (value, renderer->device);
      break;

    case PROP_GICON:
      g_value_set_object (value, renderer->gicon);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static void
fmb_shortcuts_icon_renderer_set_property (GObject      *object,
                                             guint         prop_id,
                                             const GValue *value,
                                             GParamSpec   *pspec)
{
  FmbShortcutsIconRenderer *renderer = FMB_SHORTCUTS_ICON_RENDERER (object);

  switch (prop_id)
    {
    case PROP_DEVICE:
      if (G_UNLIKELY (renderer->device != NULL))
        g_object_unref (renderer->device);
      renderer->device = g_value_dup_object (value);
      break;

    case PROP_GICON:
      if (G_UNLIKELY (renderer->gicon != NULL))
        g_object_unref (renderer->gicon);
      renderer->gicon = g_value_dup_object (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static void
fmb_shortcuts_icon_renderer_render (GtkCellRenderer     *renderer,
                                       GdkWindow           *window,
                                       GtkWidget           *widget,
                                       GdkRectangle        *background_area,
                                       GdkRectangle        *cell_area,
                                       GdkRectangle        *expose_area,
                                       GtkCellRendererState flags)
{
  FmbShortcutsIconRenderer *shortcuts_icon_renderer = FMB_SHORTCUTS_ICON_RENDERER (renderer);
  GtkIconTheme                *icon_theme;
  GdkRectangle                 draw_area;
  GdkRectangle                 icon_area;
  GtkIconInfo                 *icon_info;
  GdkPixbuf                   *icon = NULL;
  GdkPixbuf                   *temp;
  GIcon                       *gicon;
  cairo_t                     *cr;
  gdouble                      alpha;

  /* check if we have a volume set */
  if (G_UNLIKELY (shortcuts_icon_renderer->gicon != NULL
      ||  shortcuts_icon_renderer->device != NULL))
    {
      /* load the volume icon */
      icon_theme = gtk_icon_theme_get_for_screen (gdk_drawable_get_screen (window));

      /* look up the icon info */
      if (shortcuts_icon_renderer->gicon != NULL)
        gicon = g_object_ref (shortcuts_icon_renderer->gicon);
      else
        gicon = fmb_device_get_icon (shortcuts_icon_renderer->device);

      icon_info = gtk_icon_theme_lookup_by_gicon (icon_theme, gicon, cell_area->width, 
                                                  GTK_ICON_LOOKUP_USE_BUILTIN);
      g_object_unref (gicon);

      /* try to load the icon */
      if (G_LIKELY (icon_info != NULL))
        {
          icon = gtk_icon_info_load_icon (icon_info, NULL);
          gtk_icon_info_free (icon_info);
        }

      /* render the icon (if any) */
      if (G_LIKELY (icon != NULL))
        {
          /* determine the real icon size */
          icon_area.width = gdk_pixbuf_get_width (icon);
          icon_area.height = gdk_pixbuf_get_height (icon);

          /* scale down the icon on-demand */
          if (G_UNLIKELY (icon_area.width > cell_area->width || icon_area.height > cell_area->height))
            {
              /* scale down to fit */
              temp = blxo_gdk_pixbuf_scale_down (icon, TRUE, MAX (1, cell_area->width), MAX (1, cell_area->height));
              g_object_unref (G_OBJECT (icon));
              icon = temp;

              /* determine the icon dimensions again */
              icon_area.width = gdk_pixbuf_get_width (icon);
              icon_area.height = gdk_pixbuf_get_height (icon);
            }

          /* 50% translucent for unmounted volumes */
          if (shortcuts_icon_renderer->device != NULL
              && !fmb_device_is_mounted (shortcuts_icon_renderer->device))
            alpha = 0.50;
          else
            alpha = 1.00;

          icon_area.x = cell_area->x + (cell_area->width - icon_area.width) / 2;
          icon_area.y = cell_area->y + (cell_area->height - icon_area.height) / 2;

          /* check whether the icon is affected by the expose event */
          if (gdk_rectangle_intersect (expose_area, &icon_area, &draw_area))
            {
              /* render the invalid parts of the icon */
              cr = gdk_cairo_create (window);
              fmb_gdk_cairo_set_source_pixbuf (cr, icon, icon_area.x, icon_area.y);
              gdk_cairo_rectangle (cr, &draw_area);
              cairo_paint_with_alpha (cr, alpha);
              cairo_destroy (cr);
            }

          /* cleanup */
          g_object_unref (G_OBJECT (icon));
        }
    }
  else
    {
      /* fallback to the default icon renderering */
      (*GTK_CELL_RENDERER_CLASS (fmb_shortcuts_icon_renderer_parent_class)->render) (renderer, window, widget, background_area,
                                                                                        cell_area, expose_area, flags);
    }
}



/**
 * fmb_shortcuts_icon_renderer_new:
 *
 * Allocates a new #FmbShortcutsIconRenderer instance.
 *
 * Return value: the newly allocated #FmbShortcutsIconRenderer.
 **/
GtkCellRenderer*
fmb_shortcuts_icon_renderer_new (void)
{
  return g_object_new (FMB_TYPE_SHORTCUTS_ICON_RENDERER, NULL);
}


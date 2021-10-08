/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2009 Jannis Pohlmann <jannis@xfce.org>
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

#include <glib.h>
#include <glib-object.h>

#include <fmb/fmb-application.h>
#include <fmb/fmb-file-monitor.h>
#include <fmb/fmb-image.h>
#include <fmb/fmb-icon-factory.h>
#include <fmb/fmb-private.h>



#define FMB_IMAGE_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), FMB_TYPE_IMAGE, FmbImagePrivate))



/* Property identifiers */
enum
{
  PROP_0,
  PROP_FILE,
};



static void fmb_image_finalize             (GObject           *object);
static void fmb_image_get_property         (GObject           *object,
                                               guint              prop_id,
                                               GValue            *value,
                                               GParamSpec        *pspec);
static void fmb_image_set_property         (GObject           *object,
                                               guint              prop_id,
                                               const GValue      *value,
                                               GParamSpec        *pspec);
static void fmb_image_file_changed         (FmbFileMonitor *monitor,
                                               FmbFile        *file,
                                               FmbImage       *image);



struct _FmbImageClass
{
  GtkImageClass __parent__;
};

struct _FmbImage
{
  GtkImage __parent__;

  FmbImagePrivate *priv;
};

struct _FmbImagePrivate
{
  FmbFileMonitor *monitor;
  FmbFile        *file;
};



G_DEFINE_TYPE (FmbImage, fmb_image, GTK_TYPE_IMAGE);



static void
fmb_image_class_init (FmbImageClass *klass)
{
  GObjectClass *gobject_class;

  g_type_class_add_private (klass, sizeof (FmbImagePrivate));

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = fmb_image_finalize; 
  gobject_class->get_property = fmb_image_get_property;
  gobject_class->set_property = fmb_image_set_property;

  g_object_class_install_property (gobject_class, PROP_FILE,
                                   g_param_spec_object ("file",
                                                        "file",
                                                        "file",
                                                        FMB_TYPE_FILE,
                                                        G_PARAM_READWRITE));
}



static void
fmb_image_init (FmbImage *image)
{
  image->priv = FMB_IMAGE_GET_PRIVATE (image);
  image->priv->file = NULL;

  image->priv->monitor = fmb_file_monitor_get_default ();
  g_signal_connect (image->priv->monitor, "file-changed", 
                    G_CALLBACK (fmb_image_file_changed), image);
}



static void
fmb_image_finalize (GObject *object)
{
  FmbImage *image = FMB_IMAGE (object);

  g_signal_handlers_disconnect_by_func (image->priv->monitor, 
                                        fmb_image_file_changed, image);
  g_object_unref (image->priv->monitor);

  fmb_image_set_file (image, NULL);

  (*G_OBJECT_CLASS (fmb_image_parent_class)->finalize) (object);
}



static void
fmb_image_get_property (GObject    *object,
                           guint       prop_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
  FmbImage *image = FMB_IMAGE (object);

  switch (prop_id)
    {
    case PROP_FILE:
      g_value_set_object (value, image->priv->file);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static void
fmb_image_set_property (GObject      *object,
                           guint         prop_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
  FmbImage *image = FMB_IMAGE (object);

  switch (prop_id)
    {
    case PROP_FILE:
      fmb_image_set_file (image, g_value_get_object (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static void
fmb_image_update (FmbImage *image)
{
  FmbIconFactory *icon_factory;
  GtkIconTheme      *icon_theme;
  GdkPixbuf         *icon;
  GdkScreen         *screen;

  _fmb_return_if_fail (FMB_IS_IMAGE (image));
          
  if (FMB_IS_FILE (image->priv->file))
    {
      screen = gtk_widget_get_screen (GTK_WIDGET (image));
      icon_theme = gtk_icon_theme_get_for_screen (screen);
      icon_factory = fmb_icon_factory_get_for_icon_theme (icon_theme);

      icon = fmb_icon_factory_load_file_icon (icon_factory, image->priv->file,
                                                 FMB_FILE_ICON_STATE_DEFAULT, 48);

      gtk_image_set_from_pixbuf (GTK_IMAGE (image), icon);

      g_object_unref (icon_factory);
    }
}



static void
fmb_image_file_changed (FmbFileMonitor *monitor,
                           FmbFile        *file,
                           FmbImage       *image)
{
  _fmb_return_if_fail (FMB_IS_FILE_MONITOR (monitor));
  _fmb_return_if_fail (FMB_IS_FILE (file));
  _fmb_return_if_fail (FMB_IS_IMAGE (image));

  if (file == image->priv->file)
    fmb_image_update (image);
}



GtkWidget *
fmb_image_new (void)
{
  return g_object_new (FMB_TYPE_IMAGE, NULL);
}



void
fmb_image_set_file (FmbImage *image,
                       FmbFile  *file)
{
  _fmb_return_if_fail (FMB_IS_IMAGE (image));

  if (image->priv->file != NULL)
    {
      if (image->priv->file == file)
        return;

      g_object_unref (image->priv->file);
    }

  if (file != NULL)
    image->priv->file = g_object_ref (file);
  else
    image->priv->file = NULL;

  fmb_image_update (image);

  g_object_notify (G_OBJECT (image), "file");
}

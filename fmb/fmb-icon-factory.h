/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2005 Benedikt Meurer <benny@xfce.org>
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

#ifndef __FMB_ICON_FACTORY_H__
#define __FMB_ICON_FACTORY_H__

#include <fmb/fmb-file.h>

G_BEGIN_DECLS;

typedef struct _FmbIconFactoryClass FmbIconFactoryClass;
typedef struct _FmbIconFactory      FmbIconFactory;

#define FMB_TYPE_ICON_FACTORY            (fmb_icon_factory_get_type ())
#define FMB_ICON_FACTORY(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), FMB_TYPE_ICON_FACTORY, FmbIconFactory))
#define FMB_ICON_FACTORY_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), FMB_TYPE_ICON_FACTORY, FmbIconFactoryClass))
#define FMB_IS_ICON_FACTORY(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FMB_TYPE_ICON_FACTORY))
#define FMB_IS_ICON_FACTORY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), FMB_TYPE_ICON_FACTORY))
#define FMB_ICON_FACTORY_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), FMB_TYPE_ICON_FACTORY, FmbIconFactoryClass))

/**
 * FMB_THUMBNAIL_SIZE:
 * The icon size which is used for loading and storing
 * thumbnails in Fmb.
 **/
#define FMB_THUMBNAIL_SIZE (128)



GType                  fmb_icon_factory_get_type           (void) G_GNUC_CONST;

FmbIconFactory     *fmb_icon_factory_get_default        (void);
FmbIconFactory     *fmb_icon_factory_get_for_icon_theme (GtkIconTheme             *icon_theme);

gboolean               fmb_icon_factory_get_show_thumbnail (const FmbIconFactory  *factory,
                                                               const FmbFile         *file);

GdkPixbuf             *fmb_icon_factory_load_icon          (FmbIconFactory        *factory,
                                                               const gchar              *name,
                                                               gint                      size,
                                                               gboolean                  wants_default);

GdkPixbuf             *fmb_icon_factory_load_file_icon     (FmbIconFactory        *factory,
                                                               FmbFile               *file,
                                                               FmbFileIconState       icon_state,
                                                               gint                      icon_size);

void                   fmb_icon_factory_clear_pixmap_cache (FmbFile               *file);

G_END_DECLS;

#endif /* !__FMB_ICON_FACTORY_H__ */

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

#ifndef __FMB_IMAGE_H__
#define __FMB_IMAGE_H__

#include <fmb/fmb-file.h>

G_BEGIN_DECLS;

#define FMB_TYPE_IMAGE            (fmb_image_get_type ())
#define FMB_IMAGE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), FMB_TYPE_IMAGE, FmbImage))
#define FMB_IMAGE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), FMB_TYPE_IMAGE, FmbImageClass))
#define FMB_IS_IMAGE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FMB_TYPE_IMAGE))
#define FMB_IS_IMAGE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), FMB_TYPE_IMAGE)
#define FMB_IMAGE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), FMB_TYPE_IMAGE, FmbImageClass))

typedef struct _FmbImagePrivate FmbImagePrivate;
typedef struct _FmbImageClass   FmbImageClass;
typedef struct _FmbImage        FmbImage;

GType      fmb_image_get_type (void) G_GNUC_CONST;

GtkWidget *fmb_image_new      (void) G_GNUC_MALLOC;
void       fmb_image_set_file (FmbImage *image,
                                  FmbFile  *file);

G_END_DECLS;

#endif /* !__FMB_IMAGE_H__ */

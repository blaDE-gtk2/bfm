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

#ifndef __FMB_LOCATION_BUTTON_H__
#define __FMB_LOCATION_BUTTON_H__

#include <fmb/fmb-file.h>

G_BEGIN_DECLS;

typedef struct _FmbLocationButtonClass FmbLocationButtonClass;
typedef struct _FmbLocationButton      FmbLocationButton;

#define FMB_TYPE_LOCATION_BUTTON             (fmb_location_button_get_type ())
#define FMB_LOCATION_BUTTON(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), FMB_TYPE_LOCATION_BUTTON, FmbLocationButton))
#define FMB_LOCATION_BUTTON_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), FMB_TYPE_LOCATION_BUTTON, FmbLocationButtonClass))
#define FMB_IS_LOCATION_BUTTON(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FMB_TYPE_LOCATION_BUTTON))
#define FMB_IS_LOCATION_BUTTON_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), FMB_TYPE_LOCATION_BUTTON))
#define FMB_LOCATION_BUTTON_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), FMB_TYPE_LOCATION_BUTTON, FmbLocationButtonClass))

GType       fmb_location_button_get_type   (void) G_GNUC_CONST;

GtkWidget  *fmb_location_button_new        (void) G_GNUC_MALLOC;

gboolean    fmb_location_button_get_active (FmbLocationButton *location_button);
void        fmb_location_button_set_active (FmbLocationButton *location_button,
                                               gboolean              active);

FmbFile *fmb_location_button_get_file   (FmbLocationButton *location_button);
void        fmb_location_button_set_file   (FmbLocationButton *location_button,
                                               FmbFile           *file);

void        fmb_location_button_clicked    (FmbLocationButton *location_button);

G_END_DECLS;

#endif /* !__FMB_LOCATION_BUTTON_H__ */

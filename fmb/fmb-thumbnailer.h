/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2009-2011 Jannis Pohlmann <jannis@xfce.org>
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

#ifndef __FMB_THUMBNAILER_H__
#define __FMB_THUMBNAILER_H__

#include <fmb/fmb-file.h>

G_BEGIN_DECLS

typedef struct _FmbThumbnailerClass FmbThumbnailerClass;
typedef struct _FmbThumbnailer      FmbThumbnailer;

#define FMB_TYPE_THUMBNAILER            (fmb_thumbnailer_get_type ())
#define FMB_THUMBNAILER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), FMB_TYPE_THUMBNAILER, FmbThumbnailer))
#define FMB_THUMBNAILER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), FMB_TYPE_THUMBNAILER, FmbThumbnailerClass))
#define FMB_IS_THUMBNAILER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FMB_TYPE_THUMBNAILER))
#define FMB_IS_THUMBNAILER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), FMB_TYPE_THUMBNAILER))
#define FMB_THUMBNAILER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), FMB_TYPE_THUMBNAILER, FmbThumbnailerClass))

GType              fmb_thumbnailer_get_type        (void) G_GNUC_CONST;

FmbThumbnailer *fmb_thumbnailer_get             (void) G_GNUC_MALLOC;

gboolean           fmb_thumbnailer_queue_file      (FmbThumbnailer        *thumbnailer,
                                                       FmbFile               *file,
                                                       guint                    *request);
gboolean           fmb_thumbnailer_queue_files     (FmbThumbnailer        *thumbnailer,
                                                       gboolean                  lazy_checks,
                                                       GList                    *files,
                                                       guint                    *request);
void               fmb_thumbnailer_dequeue         (FmbThumbnailer        *thumbnailer,
                                                       guint                     request);

G_END_DECLS

#endif /* !__FMB_THUMBNAILER_H__ */

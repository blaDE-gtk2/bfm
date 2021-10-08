/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
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

#ifndef __FMB_THUMBNAIL_CACHE_H__
#define __FMB_THUMBNAIL_CACHE_H__

#include <gio/gio.h>

G_BEGIN_DECLS

#define FMB_TYPE_THUMBNAIL_CACHE            (fmb_thumbnail_cache_get_type ())
#define FMB_THUMBNAIL_CACHE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), FMB_TYPE_THUMBNAIL_CACHE, FmbThumbnailCache))
#define FMB_THUMBNAIL_CACHE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), FMB_TYPE_THUMBNAIL_CACHE, FmbThumbnailCacheClass))
#define FMB_IS_THUMBNAIL_CACHE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FMB_TYPE_THUMBNAIL_CACHE))
#define FMB_IS_THUMBNAIL_CACHE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), FMB_TYPE_THUMBNAIL_CACHE)
#define FMB_THUMBNAIL_CACHE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), FMB_TYPE_THUMBNAIL_CACHE, FmbThumbnailCacheClass))

typedef struct _FmbThumbnailCachePrivate FmbThumbnailCachePrivate;
typedef struct _FmbThumbnailCacheClass   FmbThumbnailCacheClass;
typedef struct _FmbThumbnailCache        FmbThumbnailCache;

GType                 fmb_thumbnail_cache_get_type     (void) G_GNUC_CONST;

FmbThumbnailCache *fmb_thumbnail_cache_new          (void) G_GNUC_MALLOC G_GNUC_WARN_UNUSED_RESULT;

void                  fmb_thumbnail_cache_move_file    (FmbThumbnailCache *cache,
                                                           GFile                *source_file,
                                                           GFile                *target_file);
void                  fmb_thumbnail_cache_copy_file    (FmbThumbnailCache *cache,
                                                           GFile                *source_file,
                                                           GFile                *target_file);
void                  fmb_thumbnail_cache_delete_file  (FmbThumbnailCache *cache,
                                                           GFile                *file);
void                  fmb_thumbnail_cache_cleanup_file (FmbThumbnailCache *cache,
                                                           GFile                *file);

G_END_DECLS

#endif /* !__FMB_THUMBNAIL_CACHE_H__ */

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

#ifndef __FMB_SBR_INSERT_RENAMER_H__
#define __FMB_SBR_INSERT_RENAMER_H__

#include <fmb-sbr/fmb-sbr-enum-types.h>

G_BEGIN_DECLS;

typedef struct _FmbSbrInsertRenamerClass FmbSbrInsertRenamerClass;
typedef struct _FmbSbrInsertRenamer      FmbSbrInsertRenamer;

#define FMB_SBR_TYPE_INSERT_RENAMER            (fmb_sbr_insert_renamer_get_type ())
#define FMB_SBR_INSERT_RENAMER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), FMB_SBR_TYPE_INSERT_RENAMER, FmbSbrInsertRenamer))
#define FMB_SBR_INSERT_RENAMER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), FMB_SBR_TYPE_INSERT_RENAMER, FmbSbrInsertRenamerClass))
#define FMB_SBR_IS_INSERT_RENAMER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FMB_SBR_TYPE_INSERT_RENAMER))
#define FMB_SBR_IS_INSERT_RENAMER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), FMB_SBR_TYPE_INSERT_RENAMER))
#define FMB_SBR_INSERT_RENAMER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), FMB_SBR_TYPE_INSERT_RENAMER, FmbSbrInsertRenamerClass))

GType                   fmb_sbr_insert_renamer_get_type        (void) G_GNUC_CONST;
void                    fmb_sbr_insert_renamer_register_type   (FmbxProviderPlugin  *plugin);

FmbSbrInsertRenamer *fmb_sbr_insert_renamer_new             (void) G_GNUC_MALLOC;

FmbSbrInsertMode     fmb_sbr_insert_renamer_get_mode        (FmbSbrInsertRenamer *insert_renamer);
void                    fmb_sbr_insert_renamer_set_mode        (FmbSbrInsertRenamer *insert_renamer,
                                                                   FmbSbrInsertMode     mode);

guint                   fmb_sbr_insert_renamer_get_offset      (FmbSbrInsertRenamer *insert_renamer);
void                    fmb_sbr_insert_renamer_set_offset      (FmbSbrInsertRenamer *insert_renamer,
                                                                   guint                   offset);

FmbSbrOffsetMode     fmb_sbr_insert_renamer_get_offset_mode (FmbSbrInsertRenamer *insert_renamer);
void                    fmb_sbr_insert_renamer_set_offset_mode (FmbSbrInsertRenamer *insert_renamer,
                                                                   FmbSbrOffsetMode     offset_mode);

const gchar            *fmb_sbr_insert_renamer_get_text        (FmbSbrInsertRenamer *insert_renamer);
void                    fmb_sbr_insert_renamer_set_text        (FmbSbrInsertRenamer *insert_renamer,
                                                                   const gchar            *text);

G_END_DECLS;

#endif /* !__FMB_SBR_INSERT_RENAMER_H__ */

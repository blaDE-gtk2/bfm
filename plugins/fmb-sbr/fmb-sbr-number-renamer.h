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

#ifndef __FMB_SBR_NUMBER_RENAMER_H__
#define __FMB_SBR_NUMBER_RENAMER_H__

#include <fmb-sbr/fmb-sbr-enum-types.h>

G_BEGIN_DECLS;

typedef struct _FmbSbrNumberRenamerClass FmbSbrNumberRenamerClass;
typedef struct _FmbSbrNumberRenamer      FmbSbrNumberRenamer;

#define FMB_SBR_TYPE_NUMBER_RENAMER            (fmb_sbr_number_renamer_get_type ())
#define FMB_SBR_NUMBER_RENAMER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), FMB_SBR_TYPE_NUMBER_RENAMER, FmbSbrNumberRenamer))
#define FMB_SBR_NUMBER_RENAMER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), FMB_SBR_TYPE_NUMBER_RENAMER, FmbSbrNumberRenamerClass))
#define FMB_SBR_IS_NUMBER_RENAMER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FMB_SBR_TYPE_NUMBER_RENAMER))
#define FMB_SBR_IS_NUMBER_RENAMER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), FMB_SBR_TYPE_NUMBER_RENAMER))
#define FMB_SBR_NUMBER_RENAMER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), FMB_SBR_TYPE_NUMBER_RENAMER, FmbSbrNumberRenamerClass))

GType                   fmb_sbr_number_renamer_get_type        (void) G_GNUC_CONST;
void                    fmb_sbr_number_renamer_register_type   (FmbxProviderPlugin  *plugin);

FmbSbrNumberRenamer *fmb_sbr_number_renamer_new             (void) G_GNUC_MALLOC;

FmbSbrNumberMode     fmb_sbr_number_renamer_get_mode        (FmbSbrNumberRenamer *number_renamer);
void                    fmb_sbr_number_renamer_set_mode        (FmbSbrNumberRenamer *number_renamer,
                                                                   FmbSbrNumberMode     mode);

const gchar            *fmb_sbr_number_renamer_get_start        (FmbSbrNumberRenamer *number_renamer);
void                    fmb_sbr_number_renamer_set_start        (FmbSbrNumberRenamer *number_renamer,
                                                                   const gchar            *start);

const gchar            *fmb_sbr_number_renamer_get_text        (FmbSbrNumberRenamer *number_renamer);
void                    fmb_sbr_number_renamer_set_text        (FmbSbrNumberRenamer *number_renamer,
                                                                   const gchar            *text);

FmbSbrTextMode       fmb_sbr_number_renamer_get_text_mode   (FmbSbrNumberRenamer *number_renamer);
void                    fmb_sbr_number_renamer_set_text_mode   (FmbSbrNumberRenamer *number_renamer,
                                                                   FmbSbrTextMode       text_mode);

G_END_DECLS;

#endif /* !__FMB_SBR_NUMBER_RENAMER_H__ */

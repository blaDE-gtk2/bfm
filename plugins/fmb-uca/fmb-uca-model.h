/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2005 Benedikt Meurer <benny@xfce.org>
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

#ifndef __FMB_UCA_MODEL_H__
#define __FMB_UCA_MODEL_H__

#include <fmbx/fmbx.h>

G_BEGIN_DECLS;

typedef struct _FmbUcaModelClass FmbUcaModelClass;
typedef struct _FmbUcaModel      FmbUcaModel;

#define FMB_UCA_TYPE_MODEL             (fmb_uca_model_get_type ())
#define FMB_UCA_MODEL(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), FMB_UCA_TYPE_MODEL, FmbUcaModel))
#define FMB_UCA_MODEL_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), FMB_UCA_TYPE_MODEL, FmbUcaModelClass))
#define FMB_UCA_IS_MODEL(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FMB_UCA_TYPE_MODEL))
#define FMB_UCA_IS_MODEL_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), FMB_UCA_TYPE_MODEL))
#define FMB_UCA_MODEL_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), FMB_UCA_TYPE_MODEL, FmbUcaModelClass))

typedef enum
{
  FMB_UCA_MODEL_COLUMN_NAME,
  FMB_UCA_MODEL_COLUMN_DESCRIPTION,
  FMB_UCA_MODEL_COLUMN_GICON,
  FMB_UCA_MODEL_COLUMN_ICON_NAME,
  FMB_UCA_MODEL_COLUMN_UNIQUE_ID,
  FMB_UCA_MODEL_COLUMN_COMMAND,
  FMB_UCA_MODEL_COLUMN_STARTUP_NOTIFY,
  FMB_UCA_MODEL_COLUMN_PATTERNS,
  FMB_UCA_MODEL_COLUMN_TYPES,
  FMB_UCA_MODEL_COLUMN_STOCK_LABEL,
  FMB_UCA_MODEL_N_COLUMNS,
} FmbUcaModelColumn;

/**
 * FmbUcaTypes:
 * @FMB_UCA_TYPE_DIRECTORIES : directories.
 * @FMB_UCA_TYPE_AUDIO_FILES : audio files.
 * @FMB_UCA_TYPE_IMAGE_FILES : image files.
 * @FMB_UCA_TYPE_OTHER_FILES : other files.
 * @FMB_UCA_TYPE_TEXT_FILES  : text files.
 * @FMB_UCA_TYPE_VIDEO_FILES : video files.
 **/
typedef enum /*< flags >*/
{
  FMB_UCA_TYPE_DIRECTORIES = 1 << 0,
  FMB_UCA_TYPE_AUDIO_FILES = 1 << 1,
  FMB_UCA_TYPE_IMAGE_FILES = 1 << 2,
  FMB_UCA_TYPE_OTHER_FILES = 1 << 3,
  FMB_UCA_TYPE_TEXT_FILES  = 1 << 4,
  FMB_UCA_TYPE_VIDEO_FILES = 1 << 5,
} FmbUcaTypes;

GType           fmb_uca_model_get_type       (void) G_GNUC_CONST;
void            fmb_uca_model_register_type  (FmbxProviderPlugin  *plugin);

FmbUcaModel *fmb_uca_model_get_default    (void);

GList          *fmb_uca_model_match          (FmbUcaModel         *uca_model,
                                                 GList                  *file_infos);

void            fmb_uca_model_append         (FmbUcaModel         *uca_model,
                                                 GtkTreeIter            *iter);

void            fmb_uca_model_exchange       (FmbUcaModel         *uca_model,
                                                 GtkTreeIter            *iter_a,
                                                 GtkTreeIter            *iter_b);

void            fmb_uca_model_remove         (FmbUcaModel         *uca_model,
                                                 GtkTreeIter            *iter);

void            fmb_uca_model_update         (FmbUcaModel         *uca_model,
                                                 GtkTreeIter            *iter,
                                                 const gchar            *name,
                                                 const gchar            *unique_id,
                                                 const gchar            *description,
                                                 const gchar            *icon,
                                                 const gchar            *command,
                                                 gboolean                startup_notify,
                                                 const gchar            *patterns,
                                                 FmbUcaTypes          types);

gboolean        fmb_uca_model_save           (FmbUcaModel         *uca_model,
                                                 GError                **error);

gboolean        fmb_uca_model_parse_argv     (FmbUcaModel         *uca_model,
                                                 GtkTreeIter            *iter,
                                                 GList                  *file_infos,
                                                 gint                   *argcp,
                                                 gchar                ***argvp,
                                                 GError                **error);

G_END_DECLS;

#endif /* !__FMB_UCA_MODEL_H__ */

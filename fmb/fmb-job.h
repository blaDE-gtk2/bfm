/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2005-2007 Benedikt Meurer <benny@xfce.org>
 * Copyright (c) 2009 Jannis Pohlmann <jannis@xfce.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef __FMB_JOB_H__
#define __FMB_JOB_H__

#include <gio/gio.h>

#include <blxo/blxo.h>

#include <fmb/fmb-enum-types.h>
#include <fmb/fmb-file.h>

G_BEGIN_DECLS

typedef struct _FmbJobPrivate FmbJobPrivate;
typedef struct _FmbJobClass   FmbJobClass;
typedef struct _FmbJob        FmbJob;

#define FMB_TYPE_JOB            (fmb_job_get_type ())
#define FMB_JOB(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), FMB_TYPE_JOB, FmbJob))
#define FMB_JOB_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), FMB_TYPE_JOB, FmbJobClass))
#define FMB_IS_JOB(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FMB_TYPE_JOB))
#define FMB_IS_JOB_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), FMB_TYPE_JOB))
#define FMB_JOB_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), FMB_TYPE_JOB, FmbJobClass))

struct _FmbJobClass
{
  /*< private >*/
  BlxoJobClass __parent__;

  /*< public >*/

  /* signals */
  FmbJobResponse (*ask)         (FmbJob        *job,
                                    const gchar      *message,
                                    FmbJobResponse choices);
  FmbJobResponse (*ask_replace) (FmbJob        *job,
                                    FmbFile       *source_file,
                                    FmbFile       *target_file);
};

struct _FmbJob
{
  /*< private >*/
  BlxoJob            __parent__;
  FmbJobPrivate *priv;
};

GType             fmb_job_get_type               (void) G_GNUC_CONST;
void              fmb_job_set_total_files        (FmbJob       *job,
                                                     GList           *total_files);
void              fmb_job_processing_file        (FmbJob       *job,
                                                     GList           *current_file);

FmbJobResponse fmb_job_ask_create             (FmbJob       *job,
                                                     const gchar     *format,
                                                     ...);
FmbJobResponse fmb_job_ask_overwrite          (FmbJob       *job,
                                                     const gchar     *format,
                                                     ...);
FmbJobResponse fmb_job_ask_replace            (FmbJob       *job,
                                                     GFile           *source_path,
                                                     GFile           *target_path,
                                                     GError         **error);
FmbJobResponse fmb_job_ask_skip               (FmbJob       *job,
                                                     const gchar     *format,
                                                     ...);
gboolean          fmb_job_ask_no_size            (FmbJob       *job,
                                                     const gchar     *format,
                                                     ...);
gboolean          fmb_job_files_ready            (FmbJob       *job,
                                                     GList           *file_list);
void              fmb_job_new_files              (FmbJob       *job,
                                                     const GList     *file_list);

G_END_DECLS

#endif /* !__FMB_JOB_H__ */

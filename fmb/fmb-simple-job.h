/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
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

#ifndef __FMB_SIMPLE_JOB_H__
#define __FMB_SIMPLE_JOB_H__

#include <fmb/fmb-job.h>

G_BEGIN_DECLS

/**
 * FmbSimpleJobFunc:
 * @job            : a #FmbJob.
 * @param_values   : a #GArray of the #GValue<!---->s passed to 
 *                   fmb_simple_job_launch().
 * @error          : return location for errors.
 *
 * Used by the #FmbSimpleJob to process the @job. See fmb_simple_job_launch()
 * for further details.
 *
 * Return value: %TRUE on success, %FALSE in case of an error.
 **/
typedef gboolean (*FmbSimpleJobFunc) (FmbJob  *job,
                                         GArray     *param_values,
                                         GError    **error);


typedef struct _FmbSimpleJobClass FmbSimpleJobClass;
typedef struct _FmbSimpleJob      FmbSimpleJob;

#define FMB_TYPE_SIMPLE_JOB            (fmb_simple_job_get_type ())
#define FMB_SIMPLE_JOB(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), FMB_TYPE_SIMPLE_JOB, FmbSimpleJob))
#define FMB_SIMPLE_JOB_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), FMB_TYPE_SIMPLE_JOB, FmbSimpleJobClass))
#define FMB_IS_SIMPLE_JOB(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FMB_TYPE_SIMPLE_JOB))
#define FMB_IS_SIMPLE_JOB_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), FMB_TYPE_SIMPLE_JOB))
#define FMB_SIMPLE_JOB_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), FMB_TYPE_SIMPLE_JOB, FmbSimpleJobClass))

GType      fmb_simple_job_get_type           (void) G_GNUC_CONST;

FmbJob *fmb_simple_job_launch             (FmbSimpleJobFunc func,
                                                 guint               n_param_values,
                                                 ...) G_GNUC_MALLOC G_GNUC_WARN_UNUSED_RESULT;
GArray    *fmb_simple_job_get_param_values   (FmbSimpleJob    *job);

G_END_DECLS

#endif /* !__FMB_SIMPLE_JOB_H__ */

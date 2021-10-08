/* vi:set sw=2 sts=2 ts=2 et ai: */
/*-
 * Copyright (c) 2009 Jannis Pohlmann <jannis@xfce.org>.
 *
 * This program is free software; you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation; either version 2 of the License, or (at 
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program; if not, write to the Free Software 
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
 * MA  02111-1307  USA
 */

#ifndef __FMB_TRANSFER_JOB_H__
#define __FMB_TRANSFER_JOB_H__

#include <glib-object.h>

G_BEGIN_DECLS

/**
 * FmbTransferJobFlags:
 *
 * Flags to control the behavior of the transfer job.
 **/
typedef enum /*< enum >*/
{
  FMB_TRANSFER_JOB_COPY,
  FMB_TRANSFER_JOB_LINK,
  FMB_TRANSFER_JOB_MOVE,
  FMB_TRANSFER_JOB_TRASH,
} FmbTransferJobType;

typedef struct _FmbTransferJobPrivate FmbTransferJobPrivate;
typedef struct _FmbTransferJobClass   FmbTransferJobClass;
typedef struct _FmbTransferJob        FmbTransferJob;

#define FMB_TYPE_TRANSFER_JOB            (fmb_transfer_job_get_type ())
#define FMB_TRANSFER_JOB(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), FMB_TYPE_TRANSFER_JOB, FmbTransferJob))
#define FMB_TRANSFER_JOB_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), FMB_TYPE_TRANSFER_JOB, FmbTransferJobClass))
#define FMB_IS_TRANSFER_JOB(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FMB_TYPE_TRANSFER_JOB))
#define FMB_IS_TRANSFER_JOB_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), FMB_TYPE_TRANSFER_JOB)
#define FMB_TRANSFER_JOB_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), FMB_TYPE_TRANSFER_JOB, FmbTransferJobClass))

GType      fmb_transfer_job_get_type (void) G_GNUC_CONST;

FmbJob *fmb_transfer_job_new        (GList                *source_file_list,
                                           GList                *target_file_list,
                                           FmbTransferJobType type) G_GNUC_MALLOC G_GNUC_WARN_UNUSED_RESULT;

gchar     *fmb_transfer_job_get_status (FmbTransferJob    *job);

G_END_DECLS

#endif /* !__FMB_TRANSFER_JOB_H__ */

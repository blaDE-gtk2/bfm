/* vi:set sw=2 sts=2 ts=2 et ai: */
/*-
 * Copyright (c) 2009 Jannis Pohlmann <jannis@xfce.org>
 * Copyright (c) 2012 Nick Schermer <nick@xfce.org>
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>

#include <fmb/fmb-deep-count-job.h>
#include <fmb/fmb-job.h>
#include <fmb/fmb-marshal.h>
#include <fmb/fmb-util.h>
#include <fmb/fmb-private.h>



/* Signal identifiers */
enum
{
  STATUS_UPDATE,
  LAST_SIGNAL,
};


#define DEEP_COUNT_FILE_INFO_NAMESPACE \
  G_FILE_ATTRIBUTE_STANDARD_TYPE "," \
  G_FILE_ATTRIBUTE_STANDARD_SIZE "," \
  G_FILE_ATTRIBUTE_ID_FILESYSTEM

static void     fmb_deep_count_job_finalize   (GObject                 *object);
static gboolean fmb_deep_count_job_execute    (BlxoJob                  *job,
                                                  GError                 **error);



struct _FmbDeepCountJobClass
{
  FmbJobClass __parent__;

  /* signals */
  void (*status_update) (FmbJob *job,
                         guint64    total_size,
                         guint      file_count,
                         guint      directory_count,
                         guint      unreadable_directory_count);
};

struct _FmbDeepCountJob
{
  FmbJob __parent__;

  GList              *files;
  GFileQueryInfoFlags query_flags;

  /* the time of the last "status-update" emission */
  gint64              last_time;

  /* status information */
  guint64             total_size;
  guint               file_count;
  guint               directory_count;
  guint               unreadable_directory_count;
};



static guint deep_count_signals[LAST_SIGNAL];



G_DEFINE_TYPE (FmbDeepCountJob, fmb_deep_count_job, FMB_TYPE_JOB)



static void
fmb_deep_count_job_class_init (FmbDeepCountJobClass *klass)
{
  BlxoJobClass  *job_class;
  GObjectClass *gobject_class;

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = fmb_deep_count_job_finalize;

  job_class = BLXO_JOB_CLASS (klass);
  job_class->execute = fmb_deep_count_job_execute;

  /**
   * FmbDeepCountJob::status-update:
   * @job                        : a #FmbJob.
   * @total_size                 : the total size in bytes.
   * @file_count                 : the number of files.
   * @directory_count            : the number of directories.
   * @unreadable_directory_count : the number of unreadable directories.
   *
   * Emitted by the @job to inform listeners about the number of files,
   * directories and bytes counted so far.
   **/
  deep_count_signals[STATUS_UPDATE] =
    g_signal_new ("status-update",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (FmbDeepCountJobClass, status_update),
                  NULL, NULL,
                  _fmb_marshal_VOID__UINT64_UINT_UINT_UINT,
                  G_TYPE_NONE, 4,
                  G_TYPE_UINT64,
                  G_TYPE_UINT,
                  G_TYPE_UINT,
                  G_TYPE_UINT);
}



static void
fmb_deep_count_job_init (FmbDeepCountJob *job)
{
  job->query_flags = G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS;
}



static void
fmb_deep_count_job_finalize (GObject *object)
{
  FmbDeepCountJob *job = FMB_DEEP_COUNT_JOB (object);

  g_list_free_full (job->files, g_object_unref);

  (*G_OBJECT_CLASS (fmb_deep_count_job_parent_class)->finalize) (object);
}



static void
fmb_deep_count_job_status_update (FmbDeepCountJob *job)
{
  _fmb_return_if_fail (FMB_IS_DEEP_COUNT_JOB (job));

  blxo_job_emit (BLXO_JOB (job),
                deep_count_signals[STATUS_UPDATE],
                0,
                job->total_size,
                job->file_count,
                job->directory_count,
                job->unreadable_directory_count);
}



static gboolean
fmb_deep_count_job_process (BlxoJob       *job,
                               GFile        *file,
                               GFileInfo    *file_info,
                               const gchar  *toplevel_fs_id,
                               GError      **error)
{
  FmbDeepCountJob *count_job = FMB_DEEP_COUNT_JOB (job);
  GFileEnumerator    *enumerator;
  GFileInfo          *child_info;
  GFileInfo          *info;
  gboolean            success = TRUE;
  GFile              *child;
  gint64              real_time;
  const gchar        *fs_id;
  gboolean            toplevel_file = (toplevel_fs_id == NULL);

  _fmb_return_val_if_fail (FMB_IS_JOB (job), FALSE);
  _fmb_return_val_if_fail (G_IS_FILE (file), FALSE);
  _fmb_return_val_if_fail (file_info == NULL || G_IS_FILE_INFO (file_info), FALSE);
  _fmb_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  /* abort if job was already cancelled */
  if (blxo_job_is_cancelled (job))
    return FALSE;

  if (file_info != NULL)
    {
      /* use the enumerator info */
      info = g_object_ref (file_info);
    }
  else
    {
      /* query size and type of the current file */
      info = g_file_query_info (file,
                                DEEP_COUNT_FILE_INFO_NAMESPACE,
                                count_job->query_flags,
                                blxo_job_get_cancellable (job),
                                error);
    }

  /* abort on invalid info or cancellation */
  if (info == NULL)
    return FALSE;

  /* abort on cancellation */
  if (blxo_job_is_cancelled (job))
    {
      g_object_unref (info);
      return FALSE;
    }

  /* only check files on the same filesystem so no remote mounts or
   * dummy filesystems are counted */
  fs_id = g_file_info_get_attribute_string (info, G_FILE_ATTRIBUTE_ID_FILESYSTEM);
  if (fs_id == NULL)
    fs_id = "";

  if (toplevel_fs_id == NULL)
    {
      /* first toplevel, so use this id */
      toplevel_fs_id = fs_id;
    }
  else if (strcmp (fs_id, toplevel_fs_id) != 0)
    {
      /* release the file info */
      g_object_unref (info);

      /* other filesystem, continue */
      return TRUE;
    }

  /* recurse if we have a directory */
  if (g_file_info_get_file_type (info) == G_FILE_TYPE_DIRECTORY)
    {
      /* try to read from the directory */
      enumerator = g_file_enumerate_children (file,
                                              DEEP_COUNT_FILE_INFO_NAMESPACE ","
                                              G_FILE_ATTRIBUTE_STANDARD_NAME,
                                              count_job->query_flags,
                                              blxo_job_get_cancellable (job),
                                              error);

      if (!blxo_job_is_cancelled (job))
        {
          if (enumerator == NULL)
            {
              /* directory was unreadable */
              count_job->unreadable_directory_count++;

              if (toplevel_file
                  && g_list_length (count_job->files) < 2)
                {
                  /* we only bail out if the job file is unreadable */
                  success = FALSE;
                }
              else
                {
                  /* ignore errors from files other than the job file */
                  g_clear_error (error);
                }
            }
          else
            {
              /* directory was readable */
              count_job->directory_count++;

              while (!blxo_job_is_cancelled (job))
                {
                  /* query next child info */
                  child_info = g_file_enumerator_next_file (enumerator,
                                                            blxo_job_get_cancellable (job),
                                                            error);

                  /* abort on invalid child info (iteration ends) or cancellation */
                  if (child_info == NULL)
                    break;

                  if (!blxo_job_is_cancelled (job))
                    {
                      /* generate a GFile for the child */
                      child = g_file_resolve_relative_path (file, g_file_info_get_name (child_info));

                      /* recurse unless the job was cancelled before */
                      fmb_deep_count_job_process (job, child, child_info, toplevel_fs_id, error);

                      /* free resources */
                      g_object_unref (child);
                    }

                  g_object_unref (child_info);
                }
            }
        }

      /* destroy the enumerator */
      if (enumerator != NULL)
        g_object_unref (enumerator);

      /* emit status update whenever we've finished a directory,
       * but not more than four times per second */
      real_time = g_get_real_time ();
      if (real_time >= count_job->last_time)
        {
          if (count_job->last_time != 0)
            fmb_deep_count_job_status_update (count_job);
          count_job->last_time = real_time + (G_USEC_PER_SEC / 4);
        }
    }
  else
    {
      /* we have a regular file or at least not a directory */
      count_job->file_count++;

      /* add size of the file to the total size */
      count_job->total_size += g_file_info_get_size (info);
    }

  /* destroy the file info */
  g_object_unref (info);

  /* we've succeeded if there was no error when loading information
   * about the job file itself and the job was not cancelled */
  return !blxo_job_is_cancelled (job) && success;
}



static gboolean
fmb_deep_count_job_execute (BlxoJob  *job,
                               GError **error)
{
  FmbDeepCountJob *count_job = FMB_DEEP_COUNT_JOB (job);
  gboolean            success = TRUE;
  GError             *err = NULL;
  GList              *lp;
  GFile              *gfile;

  _fmb_return_val_if_fail (FMB_IS_JOB (job), FALSE);
  _fmb_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  /* don't start the job if it was already cancelled */
  if (blxo_job_set_error_if_cancelled (job, error))
    return FALSE;

  /* reset counters */
  count_job->total_size = 0;
  count_job->file_count = 0;
  count_job->directory_count = 0;
  count_job->unreadable_directory_count = 0;
  count_job->last_time = 0;

  /* count files, directories and compute size of the job files */
  for (lp = count_job->files; lp != NULL; lp = lp->next)
    {
      gfile = fmb_file_get_file (FMB_FILE (lp->data));
      success = fmb_deep_count_job_process (job, gfile, NULL, NULL, &err);
      if (G_UNLIKELY (!success))
        break;
    }

  if (!success)
    {
      g_assert (err != NULL || blxo_job_is_cancelled (job));

      /* set error if the job was cancelled. otherwise just propagate
       * the results of the processing function */
      if (blxo_job_set_error_if_cancelled (job, error))
        {
          if (err != NULL)
            g_error_free (err);
        }
      else
        {
          if (err != NULL)
            g_propagate_error (error, err);
        }
    }
  else if (!blxo_job_is_cancelled (job))
    {
      /* emit final status update at the very end of the computation */
      fmb_deep_count_job_status_update (count_job);
    }

  return success;
}



FmbDeepCountJob *
fmb_deep_count_job_new (GList               *files,
                           GFileQueryInfoFlags  flags)
{
  FmbDeepCountJob *job;

  _fmb_return_val_if_fail (files != NULL, NULL);

  job = g_object_new (FMB_TYPE_DEEP_COUNT_JOB, NULL);
  job->files = g_list_copy (files);
  job->query_flags = flags;

  g_list_foreach (job->files, (GFunc) g_object_ref, NULL);

  return job;
}

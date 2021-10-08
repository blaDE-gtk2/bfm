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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_MEMORY_H
#include <memory.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include <blxo/blxo.h>

#include <fmb/fmb-enum-types.h>
#include <fmb/fmb-job.h>
#include <fmb/fmb-marshal.h>
#include <fmb/fmb-private.h>



#define FMB_JOB_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), FMB_TYPE_JOB, FmbJobPrivate))



/* Signal identifiers */
enum
{
  ASK,
  ASK_REPLACE,
  FILES_READY,
  NEW_FILES,
  LAST_SIGNAL,
};



static void              fmb_job_finalize            (GObject            *object);
static FmbJobResponse fmb_job_real_ask            (FmbJob          *job,
                                                         const gchar        *message,
                                                         FmbJobResponse   choices);
static FmbJobResponse fmb_job_real_ask_replace    (FmbJob          *job,
                                                         FmbFile         *source_file,
                                                         FmbFile         *target_file);



struct _FmbJobPrivate
{
  FmbJobResponse earlier_ask_create_response;
  FmbJobResponse earlier_ask_overwrite_response;
  FmbJobResponse earlier_ask_skip_response;
  GList            *total_files;
};



static guint job_signals[LAST_SIGNAL];



G_DEFINE_ABSTRACT_TYPE (FmbJob, fmb_job, BLXO_TYPE_JOB)



static gboolean
_fmb_job_ask_accumulator (GSignalInvocationHint *ihint,
                             GValue                *return_accu,
                             const GValue          *handler_return,
                             gpointer               data)
{
  g_value_copy (handler_return, return_accu);
  return FALSE;
}



static void
fmb_job_class_init (FmbJobClass *klass)
{
  GObjectClass *gobject_class;

  /* add our private data for this class */
  g_type_class_add_private (klass, sizeof (FmbJobPrivate));

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = fmb_job_finalize;

  klass->ask = fmb_job_real_ask;
  klass->ask_replace = fmb_job_real_ask_replace;

  /**
   * FmbJob::ask:
   * @job     : a #FmbJob.
   * @message : question to display to the user.
   * @choices : a combination of #FmbJobResponse<!---->s.
   *
   * The @message is garantied to contain valid UTF-8.
   *
   * Return value: the selected choice.
   **/
  job_signals[ASK] =
    g_signal_new (I_("ask"),
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_NO_HOOKS | G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (FmbJobClass, ask),
                  _fmb_job_ask_accumulator, NULL,
                  _fmb_marshal_FLAGS__STRING_FLAGS,
                  FMB_TYPE_JOB_RESPONSE,
                  2, G_TYPE_STRING,
                  FMB_TYPE_JOB_RESPONSE);

  /**
   * FmbJob::ask-replace:
   * @job      : a #FmbJob.
   * @src_file : the #FmbFile of the source file.
   * @dst_file : the #FmbFile of the destination file, that
   *             may be replaced with the source file.
   *
   * Emitted to ask the user whether the destination file should
   * be replaced by the source file.
   *
   * Return value: the selected choice.
   **/
  job_signals[ASK_REPLACE] =
    g_signal_new (I_("ask-replace"),
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_NO_HOOKS | G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (FmbJobClass, ask_replace),
                  _fmb_job_ask_accumulator, NULL,
                  _fmb_marshal_FLAGS__OBJECT_OBJECT,
                  FMB_TYPE_JOB_RESPONSE, 
                  2, FMB_TYPE_FILE, FMB_TYPE_FILE);

  /**
   * FmbJob::files-ready:
   * @job       : a #FmbJob.
   * @file_list : a list of #FmbFile<!---->s.
   *
   * This signal is used by #FmbJob<!---->s returned by
   * the fmb_io_jobs_list_directory() function whenever 
   * there's a bunch of #FmbFile<!---->s ready. This signal 
   * is garantied to be never emitted with an @file_list 
   * parameter of %NULL.
   *
   * To allow some further optimizations on the handler-side,
   * the handler is allowed to take over ownership of the
   * @file_list, i.e. it can reuse the @infos list and just replace
   * the data elements with it's own objects based on the
   * #FmbFile<!---->s contained within the @file_list (and
   * of course properly unreffing the previously contained infos).
   * If a handler takes over ownership of @file_list it must return
   * %TRUE here, and no further handlers will be run. Else, if
   * the handler doesn't want to take over ownership of @infos,
   * it must return %FALSE, and other handlers will be run. Use
   * this feature with care, and only if you can be sure that
   * you are the only handler connected to this signal for a
   * given job!
   *
   * Return value: %TRUE if the handler took over ownership of
   *               @file_list, else %FALSE.
   **/
  job_signals[FILES_READY] =
    g_signal_new (I_("files-ready"),
                  G_TYPE_FROM_CLASS (klass), G_SIGNAL_NO_HOOKS,
                  0, g_signal_accumulator_true_handled, NULL,
                  _fmb_marshal_BOOLEAN__POINTER,
                  G_TYPE_BOOLEAN, 1, G_TYPE_POINTER);

  /**
   * FmbJob::new-files:
   * @job       : a #FmbJob.
   * @file_list : a list of #GFile<!---->s that were created by @job.
   *
   * This signal is emitted by the @job right before the @job is terminated
   * and informs the application about the list of created files in @file_list.
   * @file_list contains only the toplevel file items, that were specified by
   * the application on creation of the @job.
   **/
  job_signals[NEW_FILES] =
    g_signal_new (I_("new-files"),
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_NO_HOOKS, 0, NULL, NULL,
                  g_cclosure_marshal_VOID__POINTER,
                  G_TYPE_NONE, 1, G_TYPE_POINTER);
}



static void
fmb_job_init (FmbJob *job)
{
  job->priv = FMB_JOB_GET_PRIVATE (job);
  job->priv->earlier_ask_create_response = 0;
  job->priv->earlier_ask_overwrite_response = 0;
  job->priv->earlier_ask_skip_response = 0;
}



static void
fmb_job_finalize (GObject *object)
{
  (*G_OBJECT_CLASS (fmb_job_parent_class)->finalize) (object);
}



static FmbJobResponse 
fmb_job_real_ask (FmbJob        *job,
                     const gchar      *message,
                     FmbJobResponse choices)
{
  FmbJobResponse response;

  _fmb_return_val_if_fail (FMB_IS_JOB (job), FMB_JOB_RESPONSE_CANCEL);
  g_signal_emit (job, job_signals[ASK], 0, message, choices, &response);
  return response;
}



static FmbJobResponse 
fmb_job_real_ask_replace (FmbJob  *job,
                             FmbFile *source_file,
                             FmbFile *target_file)
{
  FmbJobResponse response;
  gchar            *message;

  _fmb_return_val_if_fail (FMB_IS_JOB (job), FMB_JOB_RESPONSE_CANCEL);
  _fmb_return_val_if_fail (FMB_IS_FILE (source_file), FMB_JOB_RESPONSE_CANCEL);
  _fmb_return_val_if_fail (FMB_IS_FILE (target_file), FMB_JOB_RESPONSE_CANCEL);

  message = g_strdup_printf (_("The file \"%s\" already exists. Would you like to replace it?\n\n"
                               "If you replace an existing file, its contents will be overwritten."),
                             fmb_file_get_display_name (source_file));

  g_signal_emit (job, job_signals[ASK], 0, message,
                 FMB_JOB_RESPONSE_YES
                 | FMB_JOB_RESPONSE_YES_ALL
                 | FMB_JOB_RESPONSE_NO
                 | FMB_JOB_RESPONSE_NO_ALL
                 | FMB_JOB_RESPONSE_CANCEL,
                 &response);

  /* clean up */
  g_free (message);

  return response;
}



static FmbJobResponse
_fmb_job_ask_valist (FmbJob        *job,
                        const gchar      *format,
                        va_list           var_args,
                        const gchar      *question,
                        FmbJobResponse choices)
{
  FmbJobResponse response;
  gchar            *text;
  gchar            *message;

  _fmb_return_val_if_fail (FMB_IS_JOB (job), FMB_JOB_RESPONSE_CANCEL);
  _fmb_return_val_if_fail (g_utf8_validate (format, -1, NULL), FMB_JOB_RESPONSE_CANCEL);
  
  /* generate the dialog message */
  text = g_strdup_vprintf (format, var_args);
  message = (question != NULL) 
            ? g_strconcat (text, ".\n\n", question, NULL) 
            : g_strconcat (text, ".", NULL);
  g_free (text);  

  /* send the question and wait for the answer */
  blxo_job_emit (BLXO_JOB (job), job_signals[ASK], 0, message, choices, &response);
  g_free (message);

  /* cancel the job as per users request */
  if (G_UNLIKELY (response == FMB_JOB_RESPONSE_CANCEL))
    blxo_job_cancel (BLXO_JOB (job));

  return response;
}



FmbJobResponse
fmb_job_ask_overwrite (FmbJob   *job,
                          const gchar *format,
                          ...)
{
  FmbJobResponse response;
  va_list           var_args;

  _fmb_return_val_if_fail (FMB_IS_JOB (job), FMB_JOB_RESPONSE_CANCEL);
  _fmb_return_val_if_fail (format != NULL, FMB_JOB_RESPONSE_CANCEL);

  /* check if the user already cancelled the job */
  if (G_UNLIKELY (blxo_job_is_cancelled (BLXO_JOB (job))))
    return FMB_JOB_RESPONSE_CANCEL;

  /* check if the user said "Overwrite All" earlier */
  if (G_UNLIKELY (job->priv->earlier_ask_overwrite_response == FMB_JOB_RESPONSE_YES_ALL))
    return FMB_JOB_RESPONSE_YES;

  /* check if the user said "Overwrite None" earlier */
  if (G_UNLIKELY (job->priv->earlier_ask_overwrite_response == FMB_JOB_RESPONSE_NO_ALL))
    return FMB_JOB_RESPONSE_NO;

  /* ask the user what he wants to do */
  va_start (var_args, format);
  response = _fmb_job_ask_valist (job, format, var_args,
                                     _("Do you want to overwrite it?"),
                                     FMB_JOB_RESPONSE_YES 
                                     | FMB_JOB_RESPONSE_YES_ALL
                                     | FMB_JOB_RESPONSE_NO
                                     | FMB_JOB_RESPONSE_NO_ALL
                                     | FMB_JOB_RESPONSE_CANCEL);
  va_end (var_args);

  /* remember response for later */
  job->priv->earlier_ask_overwrite_response = response;

  /* translate response */
  switch (response)
    {
    case FMB_JOB_RESPONSE_YES_ALL:
      response = FMB_JOB_RESPONSE_YES;
      break;

    case FMB_JOB_RESPONSE_NO_ALL:
      response = FMB_JOB_RESPONSE_NO;
      break;

    default:
      break;
    }

  return response;
}



FmbJobResponse
fmb_job_ask_create (FmbJob   *job,
                       const gchar *format,
                       ...)
{
  FmbJobResponse response;
  va_list           var_args;

  _fmb_return_val_if_fail (FMB_IS_JOB (job), FMB_JOB_RESPONSE_CANCEL);

  if (G_UNLIKELY (blxo_job_is_cancelled (BLXO_JOB (job))))
    return FMB_JOB_RESPONSE_CANCEL;

  /* check if the user said "Create All" earlier */
  if (G_UNLIKELY (job->priv->earlier_ask_create_response == FMB_JOB_RESPONSE_YES_ALL))
    return FMB_JOB_RESPONSE_YES;

  /* check if the user said "Create None" earlier */
  if (G_UNLIKELY (job->priv->earlier_ask_create_response == FMB_JOB_RESPONSE_NO_ALL))
    return FMB_JOB_RESPONSE_NO;

  va_start (var_args, format);
  response = _fmb_job_ask_valist (job, format, var_args,
                                     _("Do you want to create it?"),
                                     FMB_JOB_RESPONSE_YES 
                                     | FMB_JOB_RESPONSE_CANCEL);
  va_end (var_args);

  job->priv->earlier_ask_create_response = response;

  /* translate the response */
  if (response == FMB_JOB_RESPONSE_YES_ALL)
    response = FMB_JOB_RESPONSE_YES;
  else if (response == FMB_JOB_RESPONSE_NO_ALL)
    response = FMB_JOB_RESPONSE_NO;
  else if (response == FMB_JOB_RESPONSE_CANCEL)
    blxo_job_cancel (BLXO_JOB (job));

  return response;
}



FmbJobResponse 
fmb_job_ask_replace (FmbJob *job,
                        GFile     *source_path,
                        GFile     *target_path,
                        GError   **error)
{
  FmbJobResponse response;
  FmbFile       *source_file;
  FmbFile       *target_file;

  _fmb_return_val_if_fail (FMB_IS_JOB (job), FMB_JOB_RESPONSE_CANCEL);
  _fmb_return_val_if_fail (G_IS_FILE (source_path), FMB_JOB_RESPONSE_CANCEL);
  _fmb_return_val_if_fail (G_IS_FILE (target_path), FMB_JOB_RESPONSE_CANCEL);

  if (G_UNLIKELY (blxo_job_set_error_if_cancelled (BLXO_JOB (job), error)))
    return FMB_JOB_RESPONSE_CANCEL;

  /* check if the user said "Overwrite All" earlier */
  if (G_UNLIKELY (job->priv->earlier_ask_overwrite_response == FMB_JOB_RESPONSE_YES_ALL))
    return FMB_JOB_RESPONSE_YES;

  /* check if the user said "Overwrite None" earlier */
  if (G_UNLIKELY (job->priv->earlier_ask_overwrite_response == FMB_JOB_RESPONSE_NO_ALL))
    return FMB_JOB_RESPONSE_NO;

  source_file = fmb_file_get (source_path, error);

  if (G_UNLIKELY (source_file == NULL))
    return FMB_JOB_RESPONSE_NO;

  target_file = fmb_file_get (target_path, error);

  if (G_UNLIKELY (target_file == NULL))
    {
      g_object_unref (source_file);
      return FMB_JOB_RESPONSE_NO;
    }

  blxo_job_emit (BLXO_JOB (job), job_signals[ASK_REPLACE], 0, 
                source_file, target_file, &response);

  g_object_unref (source_file);
  g_object_unref (target_file);

  /* remember the response for later */
  job->priv->earlier_ask_overwrite_response = response;

  /* translate the response */
  if (response == FMB_JOB_RESPONSE_YES_ALL)
    response = FMB_JOB_RESPONSE_YES;
  else if (response == FMB_JOB_RESPONSE_NO_ALL)
    response = FMB_JOB_RESPONSE_NO;
  else if (response == FMB_JOB_RESPONSE_CANCEL)
    blxo_job_cancel (BLXO_JOB (job));

  return response;
}



FmbJobResponse 
fmb_job_ask_skip (FmbJob   *job,
                     const gchar *format,
                     ...)
{
  FmbJobResponse response;
  va_list           var_args;
  
  _fmb_return_val_if_fail (FMB_IS_JOB (job), FMB_JOB_RESPONSE_CANCEL);
  _fmb_return_val_if_fail (format != NULL, FMB_JOB_RESPONSE_CANCEL);

  /* check if the user already cancelled the job */
  if (G_UNLIKELY (blxo_job_is_cancelled (BLXO_JOB (job))))
    return FMB_JOB_RESPONSE_CANCEL;

  /* check if the user said "Skip All" earlier */
  if (G_UNLIKELY (job->priv->earlier_ask_skip_response == FMB_JOB_RESPONSE_YES_ALL))
    return FMB_JOB_RESPONSE_YES;

  /* ask the user what he wants to do */
  va_start (var_args, format);
  response = _fmb_job_ask_valist (job, format, var_args,
                                     _("Do you want to skip it?"),
                                     FMB_JOB_RESPONSE_YES 
                                     | FMB_JOB_RESPONSE_YES_ALL
                                     | FMB_JOB_RESPONSE_CANCEL
                                     | FMB_JOB_RESPONSE_RETRY);
  va_end (var_args);

  /* remember the response */
  job->priv->earlier_ask_skip_response = response;

  /* translate the response */
  switch (response)
    {
    case FMB_JOB_RESPONSE_YES_ALL:
      response = FMB_JOB_RESPONSE_YES;
      break;

    case FMB_JOB_RESPONSE_YES:
    case FMB_JOB_RESPONSE_CANCEL:
    case FMB_JOB_RESPONSE_RETRY:
      break;

    default:
      _fmb_assert_not_reached ();
    }

  return response;
}



gboolean
fmb_job_ask_no_size (FmbJob   *job,
                        const gchar *format,
                        ...)
{
  FmbJobResponse response;
  va_list           var_args;

  _fmb_return_val_if_fail (FMB_IS_JOB (job), FMB_JOB_RESPONSE_CANCEL);
  _fmb_return_val_if_fail (format != NULL, FMB_JOB_RESPONSE_CANCEL);

  /* check if the user already cancelled the job */
  if (G_UNLIKELY (blxo_job_is_cancelled (BLXO_JOB (job))))
    return FMB_JOB_RESPONSE_CANCEL;

  /* ask the user what he wants to do */
  va_start (var_args, format);
  response = _fmb_job_ask_valist (job, format, var_args,
                                     _("There is not enough space on the destination. Try to remove files to make space."),
                                     FMB_JOB_RESPONSE_FORCE
                                     | FMB_JOB_RESPONSE_CANCEL);
  va_end (var_args);

  return (response == FMB_JOB_RESPONSE_FORCE);
}



gboolean
fmb_job_files_ready (FmbJob *job,
                        GList     *file_list)
{
  gboolean handled = FALSE;

  _fmb_return_val_if_fail (FMB_IS_JOB (job), FALSE);

  blxo_job_emit (BLXO_JOB (job), job_signals[FILES_READY], 0, file_list, &handled);
  return handled;
}



void
fmb_job_new_files (FmbJob   *job,
                      const GList *file_list)
{
  FmbFile  *file;
  const GList *lp;

  _fmb_return_if_fail (FMB_IS_JOB (job));

  /* check if we have any files */
  if (G_LIKELY (file_list != NULL))
    {
      /* schedule a reload of cached files when idle */
      for (lp = file_list; lp != NULL; lp = lp->next)
        {
          file = fmb_file_cache_lookup (lp->data);
          if (file != NULL)
            {
              fmb_file_reload_idle_unref (file);
            }
        }

      /* emit the "new-files" signal */
      blxo_job_emit (BLXO_JOB (job), job_signals[NEW_FILES], 0, file_list);
    }
}



void
fmb_job_set_total_files (FmbJob *job,
                            GList     *total_files)
{
  _fmb_return_if_fail (FMB_IS_JOB (job));
  _fmb_return_if_fail (job->priv->total_files == NULL);
  _fmb_return_if_fail (total_files != NULL);

  job->priv->total_files = total_files;
}



void
fmb_job_processing_file (FmbJob *job,
                            GList     *current_file)
{
  GList *lp;
  gchar *base_name;
  gchar *display_name;
  guint  n_processed;
  guint  n_total;

  _fmb_return_if_fail (FMB_IS_JOB (job));
  _fmb_return_if_fail (current_file != NULL);

  base_name = g_file_get_basename (current_file->data);
  display_name = g_filename_display_name (base_name);
  g_free (base_name);

  blxo_job_info_message (BLXO_JOB (job), "%s", display_name);
  g_free (display_name);

  /* verify that we have total files set */
  if (G_LIKELY (job->priv->total_files != NULL))
    {
      /* determine the number of files processed so far */
      for (lp = job->priv->total_files, n_processed = 0;
           lp != current_file;
           lp = lp->next);

      /* emit only if n_processed is a multiple of 8 */
      if ((n_processed % 8) == 0)
        {
          /* determine the total_number of files */
          n_total = g_list_length (job->priv->total_files);

          blxo_job_percent (BLXO_JOB (job), (n_processed * 100.0) / n_total);
        }
    }
}

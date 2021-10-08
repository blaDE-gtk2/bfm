/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2006 Benedikt Meurer <benny@xfce.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <fmb/fmb-file-monitor.h>
#include <fmb/fmb-private.h>



/* Signal identifiers */
enum
{
  FILE_CHANGED,
  FILE_DESTROYED,
  LAST_SIGNAL,
};



struct _FmbFileMonitorClass
{
  GObjectClass __parent__;
};

struct _FmbFileMonitor
{
  GObject __parent__;
};



static FmbFileMonitor *file_monitor_default;
static guint              file_monitor_signals[LAST_SIGNAL];



G_DEFINE_TYPE (FmbFileMonitor, fmb_file_monitor, G_TYPE_OBJECT)



static void
fmb_file_monitor_class_init (FmbFileMonitorClass *klass)
{
  /**
   * FmbFileMonitor::file-changed:
   * @file_monitor : the default #FmbFileMonitor.
   * @file         : the #FmbFile that changed.
   *
   * This signal is emitted on @file_monitor whenever any of the currently
   * existing #FmbFile instances changes. @file identifies the instance
   * that changed.
   **/
  file_monitor_signals[FILE_CHANGED] =
    g_signal_new (I_("file-changed"),
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_NO_HOOKS,
                  0, NULL, NULL,
                  g_cclosure_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1, FMB_TYPE_FILE);

  /**
   * FmbFileMonitor::file-destroyed:
   * @file_monitor : the default #FmbFileMonitor.
   * @file         : the #FmbFile that is about to be destroyed.
   *
   * This signal is emitted on @file_monitor whenever any of the currently
   * existing #FmbFile instances is about to be destroyed. @file identifies
   * the instance that is about to be destroyed.
   *
   * Note that this signal is only emitted if @file is explicitly destroyed,
   * i.e. because Fmb noticed that it was removed from disk, it is not
   * emitted when the last reference on @file is released.
   **/
  file_monitor_signals[FILE_DESTROYED] =
    g_signal_new (I_("file-destroyed"),
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_NO_HOOKS,
                  0, NULL, NULL,
                  g_cclosure_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1, FMB_TYPE_FILE);
}



static void
fmb_file_monitor_init (FmbFileMonitor *monitor)
{
}



/**
 * fmb_file_monitor_get_default:
 *
 * Returns a reference to the default #FmbFileMonitor
 * instance. The #FmbFileMonitor default instance can
 * be used to monitor the lifecycle of all currently existing
 * #FmbFile instances. The ::file-changed and ::file-destroyed
 * signals will be emitted whenever any of the currently
 * existing #FmbFile<!---->s is changed or destroyed.
 *
 * The caller is responsible to free the returned instance
 * using g_object_unref() when no longer needed.
 *
 * Return value: the default #FmbFileMonitor instance.
 **/
FmbFileMonitor*
fmb_file_monitor_get_default (void)
{
  if (G_UNLIKELY (file_monitor_default == NULL))
    {
      /* allocate the default monitor */
      file_monitor_default = g_object_new (FMB_TYPE_FILE_MONITOR, NULL);
      g_object_add_weak_pointer (G_OBJECT (file_monitor_default), 
                                 (gpointer) &file_monitor_default);
    }
  else
    {
      /* take a reference for the caller */
      g_object_ref (G_OBJECT (file_monitor_default));
    }

  return file_monitor_default;
}



/**
 * fmb_file_monitor_file_changed:
 * @file : a #FmbFile.
 *
 * Emits the ::file-changed signal on the default
 * #FmbFileMonitor (if any). This method should
 * only be used by #FmbFile.
 **/
void
fmb_file_monitor_file_changed (FmbFile *file)
{
  _fmb_return_if_fail (FMB_IS_FILE (file));

  if (G_LIKELY (file_monitor_default != NULL))
    g_signal_emit (G_OBJECT (file_monitor_default), file_monitor_signals[FILE_CHANGED], 0, file);
}



/**
 * fmb_file_monitor_file_destroyed.
 * @file : a #FmbFile.
 *
 * Emits the ::file-destroyed signal on the default
 * #FmbFileMonitor (if any). This method should
 * only be used by #FmbFile.
 **/
void
fmb_file_monitor_file_destroyed (FmbFile *file)
{
  _fmb_return_if_fail (FMB_IS_FILE (file));

  if (G_LIKELY (file_monitor_default != NULL))
    g_signal_emit (G_OBJECT (file_monitor_default), file_monitor_signals[FILE_DESTROYED], 0, file);
}



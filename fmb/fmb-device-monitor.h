/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2012 Nick Schermer <nick@xfce.org>
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

#ifndef __FMB_DEVICE_MONITOR_H__
#define __FMB_DEVICE_MONITOR_H__

#include <fmb/fmb-device.h>

G_BEGIN_DECLS

typedef struct _FmbDeviceMonitorClass FmbDeviceMonitorClass;
typedef struct _FmbDeviceMonitor      FmbDeviceMonitor;

#define FMB_TYPE_DEVICE_MONITOR             (fmb_device_monitor_get_type ())
#define FMB_DEVICE_MONITOR(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), FMB_TYPE_DEVICE_MONITOR, FmbDeviceMonitor))
#define FMB_DEVICE_MONITOR_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), FMB_TYPE_DEVICE_MONITOR, FmbDeviceMonitorClass))
#define FMB_IS_DEVICE_MONITOR(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FMB_TYPE_DEVICE_MONITOR))
#define FMB_IS_DEVICE_MONITOR_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((obj), FMB_TYPE_DEVICE_MONITOR))
#define FMB_DEVICE_MONITOR_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), FMB_TYPE_DEVICE_MONITOR, FmbDeviceMonitorClass))

GType                fmb_device_monitor_get_type    (void) G_GNUC_CONST;

FmbDeviceMonitor *fmb_device_monitor_get         (void);

GList               *fmb_device_monitor_get_devices (FmbDeviceMonitor *monitor);

void                 fmb_device_monitor_set_hidden  (FmbDeviceMonitor *monitor,
                                                        FmbDevice        *device,
                                                        gboolean             hidden);

G_END_DECLS

#endif /* !__FMB_DEVICE_MONITOR_H__ */

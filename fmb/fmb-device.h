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

#ifndef __FMB_DEVICE_H__
#define __FMB_DEVICE_H__

#include <fmb/fmb-file.h>

G_BEGIN_DECLS

typedef struct _FmbDeviceClass FmbDeviceClass;
typedef struct _FmbDevice      FmbDevice;
typedef enum   _FmbDeviceKind  FmbDeviceKind;

typedef void   (*FmbDeviceCallback) (FmbDevice *device,
                                        const GError *error,
                                        gpointer      user_data);

enum _FmbDeviceKind
{
  FMB_DEVICE_KIND_VOLUME,
  FMB_DEVICE_KIND_MOUNT_LOCAL,
  FMB_DEVICE_KIND_MOUNT_REMOTE
};

#define FMB_TYPE_DEVICE             (fmb_device_get_type ())
#define FMB_DEVICE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), FMB_TYPE_DEVICE, FmbDevice))
#define FMB_DEVICE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), FMB_TYPE_DEVICE, FmbDeviceClass))
#define FMB_IS_DEVICE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FMB_TYPE_DEVICE))
#define FMB_IS_DEVICE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((obj), FMB_TYPE_DEVICE))
#define FMB_DEVICE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), FMB_TYPE_DEVICE, FmbDeviceClass))

GType                fmb_device_get_type         (void) G_GNUC_CONST;

gchar               *fmb_device_get_name         (const FmbDevice   *device) G_GNUC_MALLOC;

GIcon               *fmb_device_get_icon         (const FmbDevice   *device);

FmbDeviceKind     fmb_device_get_kind         (const FmbDevice   *device) G_GNUC_PURE;

gchar               *fmb_device_get_identifier   (const FmbDevice   *device) G_GNUC_MALLOC;

gboolean             fmb_device_get_hidden       (const FmbDevice   *device);

gboolean             fmb_device_can_eject        (const FmbDevice   *device);

gboolean             fmb_device_can_mount        (const FmbDevice   *device);

gboolean             fmb_device_can_unmount      (const FmbDevice   *device);

gboolean             fmb_device_is_mounted       (const FmbDevice   *device);

GFile               *fmb_device_get_root         (const FmbDevice   *device);

gint                 fmb_device_sort             (const FmbDevice   *device1,
                                                     const FmbDevice   *device2);

void                 fmb_device_mount            (FmbDevice         *device,
                                                     GMountOperation      *mount_operation,
                                                     GCancellable         *cancellable,
                                                     FmbDeviceCallback  callback,
                                                     gpointer              user_data);

void                 fmb_device_unmount          (FmbDevice         *device,
                                                     GMountOperation      *mount_operation,
                                                     GCancellable         *cancellable,
                                                     FmbDeviceCallback  callback,
                                                     gpointer              user_data);

void                 fmb_device_eject            (FmbDevice         *device,
                                                     GMountOperation      *mount_operation,
                                                     GCancellable         *cancellable,
                                                     FmbDeviceCallback  callback,
                                                     gpointer              user_data);

void                 fmb_device_reload_file      (FmbDevice         *device);

G_END_DECLS

#endif /* !__FMB_DEVICE_H__ */

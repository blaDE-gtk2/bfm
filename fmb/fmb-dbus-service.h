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

#ifndef __FMB_DBUS_SERVICE_H__
#define __FMB_DBUS_SERVICE_H__

#include <blxo/blxo.h>

G_BEGIN_DECLS;

typedef struct _FmbDBusServiceClass FmbDBusServiceClass;
typedef struct _FmbDBusService      FmbDBusService;

#define FMB_TYPE_DBUS_SERVICE             (fmb_dbus_service_get_type ())
#define FMB_DBUS_SERVICE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), FMB_TYPE_DBUS_SERVICE, FmbDBusService))
#define FMB_DBUS_SERVICE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), FMB_TYPE_DBUS_SERVICE, FmbDBusServiceClass))
#define FMB_IS_DBUS_SERVICE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FMB_TYPE_DBUS_SERVICE))
#define FMB_IS_DBUS_SERVICE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), FMB_TYPE_DBUS_BRIGDE))
#define FMB_DBUS_SERVICE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), FMB_TYPE_DBUS_SERVICE, FmbDBusServicetClass))

GType    fmb_dbus_service_get_type       (void) G_GNUC_CONST;

gboolean fmb_dbus_service_has_connection (FmbDBusService *dbus_service);

G_END_DECLS;

#endif /* !__FMB_DBUS_SERVICE_H__ */

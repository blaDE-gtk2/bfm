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

#ifndef __FMB_SESSION_CLIENT_H__
#define __FMB_SESSION_CLIENT_H__

#include <glib-object.h>

G_BEGIN_DECLS;

typedef struct _FmbSessionClientClass FmbSessionClientClass;
typedef struct _FmbSessionClient      FmbSessionClient;

#define FMB_TYPE_SESSION_CLIENT            (fmb_session_client_get_type ())
#define FMB_SESSION_CLIENT(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), FMB_TYPE_SESSION_CLIENT, FmbSessionClient))
#define FMB_SESSION_CLIENT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), FMB_TYPE_SESSION_CLIENT, FmbSessionClientClass))
#define FMB_IS_SESSION_CLIENT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FMB_TYPE_SESSION_CLIENT))
#define FMB_IS_SESSION_CLIENT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), FMB_TYPE_SESSION_CLIENT))
#define FMB_SESSION_CLIENT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), FMB_TYPE_SESSION_CLIENT, FmbSessionClientClass))

GType                fmb_session_client_get_type (void) G_GNUC_CONST;
FmbSessionClient *fmb_session_client_new      (const gchar *session_id) G_GNUC_MALLOC;

G_END_DECLS;

#endif /* !__FMB_SESSION_CLIENT_H__ */

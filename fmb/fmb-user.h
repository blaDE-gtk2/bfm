/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2005-2006 Benedikt Meurer <benny@xfce.org>
 * Copyright (c) 2009 Jannis Pohlmann <jannis@xfce.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __FMB_USER_H__
#define __FMB_USER_H__

#include <glib-object.h>

G_BEGIN_DECLS;

typedef struct _FmbGroupClass FmbGroupClass;
typedef struct _FmbGroup      FmbGroup;

#define FMB_TYPE_GROUP            (fmb_group_get_type ())
#define FMB_GROUP(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), FMB_TYPE_GROUP, FmbGroup))
#define FMB_GROUP_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), FMB_TYPE_GROUP, FmbGroupClass))
#define FMB_IS_GROUP(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FMB_TYPE_GROUP))
#define FMB_IS_GROUP_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), FMB_TYPE_GROUP))
#define FMB_GROUP_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), FMB_TYPE_GROUP, FmbGroupClass))

GType         fmb_group_get_type  (void) G_GNUC_CONST;

guint32       fmb_group_get_id    (FmbGroup *group);
const gchar  *fmb_group_get_name  (FmbGroup *group);


typedef struct _FmbUserClass FmbUserClass;
typedef struct _FmbUser      FmbUser;

#define FMB_TYPE_USER            (fmb_user_get_type ())
#define FMB_USER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), FMB_TYPE_USER, FmbUser))
#define FMB_USER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), FMB_TYPE_USER, FmbUserClass))
#define FMB_IS_USER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FMB_TYPE_USER))
#define FMB_IS_USER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), FMB_TYPE_USER))
#define FMB_USER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), FMB_TYPE_USER, FmbUserClass))

GType         fmb_user_get_type          (void) G_GNUC_CONST;

GList        *fmb_user_get_groups        (FmbUser *user);
const gchar  *fmb_user_get_name          (FmbUser *user);
const gchar  *fmb_user_get_real_name     (FmbUser *user);
gboolean      fmb_user_is_me             (FmbUser *user);


typedef struct _FmbUserManagerClass FmbUserManagerClass;
typedef struct _FmbUserManager      FmbUserManager;

#define FMB_TYPE_USER_MANAGER            (fmb_user_manager_get_type ())
#define FMB_USER_MANAGER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), FMB_TYPE_USER_MANAGER, FmbUserManager))
#define FMB_USER_MANAGER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), FMB_TYPE_USER_MANAGER, FmbUserManagerClass))
#define FMB_IS_USER_MANAGER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FMB_TYPE_USER_MANAGER))
#define FMB_IS_USER_MANAGER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), FMB_TYPE_USER_MANAGER))
#define FMB_USER_MANAGER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), FMB_TYPE_USER_MANAGER, FmbUserManagerClass))

GType              fmb_user_manager_get_type        (void) G_GNUC_CONST;

FmbUserManager *fmb_user_manager_get_default     (void) G_GNUC_WARN_UNUSED_RESULT;

FmbGroup       *fmb_user_manager_get_group_by_id (FmbUserManager *manager,
                                                        guint32            id) G_GNUC_WARN_UNUSED_RESULT;
FmbUser        *fmb_user_manager_get_user_by_id  (FmbUserManager *manager,
                                                        guint32            id) G_GNUC_WARN_UNUSED_RESULT;

GList             *fmb_user_manager_get_all_groups  (FmbUserManager *manager) G_GNUC_MALLOC G_GNUC_WARN_UNUSED_RESULT;

G_END_DECLS;

#endif /* !__FMB_USER_H__ */

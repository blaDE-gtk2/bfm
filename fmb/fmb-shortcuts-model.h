/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2005-2006 Benedikt Meurer <benny@xfce.org>
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

#ifndef __FMB_SHORTCUTS_MODEL_H__
#define __FMB_SHORTCUTS_MODEL_H__

#include <fmb/fmb-file.h>
#include <fmb/fmb-device.h>

G_BEGIN_DECLS;

typedef struct _FmbShortcutsModelClass FmbShortcutsModelClass;
typedef struct _FmbShortcutsModel      FmbShortcutsModel;
typedef enum   _FmbShortcutGroup       FmbShortcutGroup;

#define FMB_TYPE_SHORTCUTS_MODEL            (fmb_shortcuts_model_get_type ())
#define FMB_SHORTCUTS_MODEL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), FMB_TYPE_SHORTCUTS_MODEL, FmbShortcutsModel))
#define FMB_SHORTCUTS_MODEL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), FMB_TYPE_SHORTCUTS_MODEL, FmbShortcutsModelClass))
#define FMB_IS_SHORTCUTS_MODEL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FMB_TYPE_SHORTCUTS_MODEL))
#define FMB_IS_SHORTCUTS_MODEL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), FMB_TYPE_SHORTCUTS_MODEL))
#define FMB_SHORTCUTS_MODEL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), FMB_TYPE_MODEL_SHORTCUTS_MODEL, FmbShortcutsModelClass))

typedef enum
{
  FMB_SHORTCUTS_MODEL_COLUMN_IS_HEADER,
  FMB_SHORTCUTS_MODEL_COLUMN_IS_ITEM,
  FMB_SHORTCUTS_MODEL_COLUMN_VISIBLE,
  FMB_SHORTCUTS_MODEL_COLUMN_NAME,
  FMB_SHORTCUTS_MODEL_COLUMN_TOOLTIP,
  FMB_SHORTCUTS_MODEL_COLUMN_FILE,
  FMB_SHORTCUTS_MODEL_COLUMN_LOCATION,
  FMB_SHORTCUTS_MODEL_COLUMN_GICON,
  FMB_SHORTCUTS_MODEL_COLUMN_DEVICE,
  FMB_SHORTCUTS_MODEL_COLUMN_MUTABLE,
  FMB_SHORTCUTS_MODEL_COLUMN_CAN_EJECT,
  FMB_SHORTCUTS_MODEL_COLUMN_GROUP,
  FMB_SHORTCUTS_MODEL_COLUMN_BUSY,
  FMB_SHORTCUTS_MODEL_COLUMN_BUSY_PULSE,
  FMB_SHORTCUTS_MODEL_N_COLUMNS,
} FmbShortcutsModelColumn;

#define FMB_SHORTCUT_GROUP_DEVICES (FMB_SHORTCUT_GROUP_DEVICES_HEADER \
                                       | FMB_SHORTCUT_GROUP_DEVICES_FILESYSTEM \
                                       | FMB_SHORTCUT_GROUP_DEVICES_VOLUMES \
                                       | FMB_SHORTCUT_GROUP_DEVICES_MOUNTS)
#define FMB_SHORTCUT_GROUP_PLACES  (FMB_SHORTCUT_GROUP_PLACES_HEADER \
                                       | FMB_SHORTCUT_GROUP_PLACES_DEFAULT \
                                       | FMB_SHORTCUT_GROUP_PLACES_TRASH \
                                       | FMB_SHORTCUT_GROUP_PLACES_BOOKMARKS)
#define FMB_SHORTCUT_GROUP_NETWORK (FMB_SHORTCUT_GROUP_NETWORK_HEADER \
                                       | FMB_SHORTCUT_GROUP_NETWORK_DEFAULT \
                                       | FMB_SHORTCUT_GROUP_NETWORK_MOUNTS)
#define FMB_SHORTCUT_GROUP_HEADER  (FMB_SHORTCUT_GROUP_DEVICES_HEADER \
                                       | FMB_SHORTCUT_GROUP_PLACES_HEADER \
                                       | FMB_SHORTCUT_GROUP_NETWORK_HEADER)

enum _FmbShortcutGroup
{
  /* FMB_SHORTCUT_GROUP_DEVICES */
  FMB_SHORTCUT_GROUP_DEVICES_HEADER     = (1 << 0),  /* devices header */
  FMB_SHORTCUT_GROUP_DEVICES_FILESYSTEM = (1 << 1),  /* local filesystem */
  FMB_SHORTCUT_GROUP_DEVICES_VOLUMES    = (1 << 2),  /* local FmbDevices */
  FMB_SHORTCUT_GROUP_DEVICES_MOUNTS     = (1 << 3),  /* local mounts, like cameras and archives */

  /* FMB_SHORTCUT_GROUP_PLACES */
  FMB_SHORTCUT_GROUP_PLACES_HEADER      = (1 << 4),  /* places header */
  FMB_SHORTCUT_GROUP_PLACES_DEFAULT     = (1 << 5),  /* home and desktop */
  FMB_SHORTCUT_GROUP_PLACES_TRASH       = (1 << 6),  /* trash */
  FMB_SHORTCUT_GROUP_PLACES_BOOKMARKS   = (1 << 7),  /* gtk-bookmarks */

  /* FMB_SHORTCUT_GROUP_NETWORK */
  FMB_SHORTCUT_GROUP_NETWORK_HEADER     = (1 << 8),  /* network header */
  FMB_SHORTCUT_GROUP_NETWORK_DEFAULT    = (1 << 9),  /* browse network */
  FMB_SHORTCUT_GROUP_NETWORK_MOUNTS     = (1 << 10), /* remote FmbDevices */
};



GType                  fmb_shortcuts_model_get_type      (void) G_GNUC_CONST;

FmbShortcutsModel  *fmb_shortcuts_model_get_default   (void);

gboolean               fmb_shortcuts_model_has_bookmark  (FmbShortcutsModel *model,
                                                             GFile                *file);

gboolean               fmb_shortcuts_model_iter_for_file (FmbShortcutsModel *model,
                                                             FmbFile           *file,
                                                             GtkTreeIter          *iter);

gboolean               fmb_shortcuts_model_drop_possible (FmbShortcutsModel *model,
                                                             GtkTreePath          *path);

void                   fmb_shortcuts_model_add           (FmbShortcutsModel *model,
                                                             GtkTreePath          *dst_path,
                                                             gpointer              file);
void                   fmb_shortcuts_model_move          (FmbShortcutsModel *model,
                                                             GtkTreePath          *src_path,
                                                             GtkTreePath          *dst_path);
void                   fmb_shortcuts_model_remove        (FmbShortcutsModel *model,
                                                             GtkTreePath          *path);
void                   fmb_shortcuts_model_rename        (FmbShortcutsModel *model,
                                                             GtkTreeIter          *iter,
                                                             const gchar          *name);
void                   fmb_shortcuts_model_set_busy      (FmbShortcutsModel *model,
                                                             FmbDevice         *device,
                                                             gboolean              busy);
void                   fmb_shortcuts_model_set_hidden    (FmbShortcutsModel *model,
                                                             GtkTreePath          *path,
                                                             gboolean              hidden);

G_END_DECLS;

#endif /* !__FMB_SHORTCUTS_MODEL_H__ */

/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2005 Benedikt Meurer <benny@xfce.org>
 * Copyright (c) 2009 Jannis Pohlmann <jannis@xfce.org>
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

#ifndef __FMB_CLIPBOARD_MANAGER_H__
#define __FMB_CLIPBOARD_MANAGER_H__

#include <fmb/fmb-file.h>

G_BEGIN_DECLS;

typedef struct _FmbClipboardManagerClass FmbClipboardManagerClass;
typedef struct _FmbClipboardManager      FmbClipboardManager;

#define FMB_TYPE_CLIPBOARD_MANAGER             (fmb_clipboard_manager_get_type ())
#define FMB_CLIPBOARD_MANAGER(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), FMB_TYPE_CLIPBOARD_MANAGER, FmbClipboardManager))
#define FMB_CLIPBOARD_MANAGER_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((obj), FMB_TYPE_CLIPBOARD_MANAGER, FmbClipboardManagerClass))
#define FMB_IS_CLIPBOARD_MANAGER(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FMB_TYPE_CLIPBOARD_MANAGER))
#define FMB_IS_CLIPBOARD_MANAGER_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), FMB_TYPE_CLIPBOARD_MANAGER))
#define FMB_CLIPBOARD_MANAGER_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), FMB_TYPE_CLIPBOARD_MANAGER, FmbClipboardManagerClass))

GType                   fmb_clipboard_manager_get_type        (void) G_GNUC_CONST;

FmbClipboardManager *fmb_clipboard_manager_get_for_display (GdkDisplay             *display);

gboolean                fmb_clipboard_manager_get_can_paste   (FmbClipboardManager *manager);

gboolean                fmb_clipboard_manager_has_cutted_file (FmbClipboardManager *manager,
                                                                  const FmbFile       *file);

void                    fmb_clipboard_manager_copy_files      (FmbClipboardManager *manager,
                                                                  GList                  *files);
void                    fmb_clipboard_manager_cut_files       (FmbClipboardManager *manager,
                                                                  GList                  *files);
void                    fmb_clipboard_manager_paste_files     (FmbClipboardManager *manager,
                                                                  GFile                  *target_file,
                                                                  GtkWidget              *widget,
                                                                  GClosure               *new_files_closure);

G_END_DECLS;

#endif /* !__FMB_CLIPBOARD_MANAGER_H__ */

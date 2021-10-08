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

#ifndef __FMB_STANDARD_VIEW_H__
#define __FMB_STANDARD_VIEW_H__

#include <fmb/fmb-clipboard-manager.h>
#include <fmb/fmb-history.h>
#include <fmb/fmb-icon-factory.h>
#include <fmb/fmb-list-model.h>
#include <fmb/fmb-preferences.h>
#include <fmb/fmb-view.h>

G_BEGIN_DECLS;

typedef struct _FmbStandardViewPrivate FmbStandardViewPrivate;
typedef struct _FmbStandardViewClass   FmbStandardViewClass;
typedef struct _FmbStandardView        FmbStandardView;

#define FMB_TYPE_STANDARD_VIEW             (fmb_standard_view_get_type ())
#define FMB_STANDARD_VIEW(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), FMB_TYPE_STANDARD_VIEW, FmbStandardView))
#define FMB_STANDARD_VIEW_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), FMB_TYPE_STANDARD_VIEW, FmbStandardViewClass))
#define FMB_IS_STANDARD_VIEW(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FMB_TYPE_STANDARD_VIEW))
#define FMB_IS_STANDARD_VIEW_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), FMB_TYPE_STANDARD_VIEW))
#define FMB_STANDARD_VIEW_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), FMB_TYPE_STANDARD_VIEW, FmbStandardViewClass))

struct _FmbStandardViewClass
{
  GtkScrolledWindowClass __parent__;

  /* Called by the FmbStandardView class to let derived classes
   * connect to and disconnect from the UI manager.
   */
  void       (*connect_ui_manager)      (FmbStandardView *standard_view,
                                         GtkUIManager       *ui_manager);
  void       (*disconnect_ui_manager)   (FmbStandardView *standard_view,
                                         GtkUIManager       *ui_manager);

  /* Returns the list of currently selected GtkTreePath's, where
   * both the list and the items are owned by the caller. */
  GList       *(*get_selected_items)    (FmbStandardView *standard_view);

  /* Selects all items in the view */
  void         (*select_all)            (FmbStandardView *standard_view);

  /* Unselects all items in the view */
  void         (*unselect_all)          (FmbStandardView *standard_view);

  /* Invert selection in the view */
  void         (*selection_invert)      (FmbStandardView *standard_view);

  /* Selects the given item */
  void         (*select_path)           (FmbStandardView *standard_view,
                                         GtkTreePath        *path);

  /* Called by the FmbStandardView class to let derived class
   * place the cursor on the item/row referred to by path. If
   * start_editing is TRUE, the derived class should also start
   * editing that item/row.
   */
  void         (*set_cursor)            (FmbStandardView *standard_view,
                                         GtkTreePath        *path,
                                         gboolean            start_editing);

  /* Called by the FmbStandardView class to let derived class
   * scroll the view to the given path.
   */
  void         (*scroll_to_path)        (FmbStandardView *standard_view,
                                         GtkTreePath        *path,
                                         gboolean            use_align,
                                         gfloat              row_align,
                                         gfloat              col_align);

  /* Returns the path at the given position or NULL if no item/row
   * is located at that coordinates. The path is freed by the caller.
   */
  GtkTreePath *(*get_path_at_pos)       (FmbStandardView *standard_view,
                                         gint                x,
                                         gint                y);

  /* Returns the visible range */
  gboolean     (*get_visible_range)     (FmbStandardView *standard_view,
                                         GtkTreePath       **start_path,
                                         GtkTreePath       **end_path);

  /* Sets the item/row that is highlighted for feedback. NULL is
   * passed for path to disable the highlighting.
   */
  void         (*highlight_path)        (FmbStandardView  *standard_view,
                                         GtkTreePath         *path);

  /* external signals */
  void         (*start_open_location)   (FmbStandardView *standard_view,
                                         const gchar        *initial_text);

  /* Internal action signals */
  gboolean     (*delete_selected_files) (FmbStandardView *standard_view);

  /* The name of the property in FmbPreferences, that determines
   * the last (and default) zoom-level for the view classes (i.e. in
   * case of FmbIconView, this is "last-icon-view-zoom-level").
   */
  const gchar *zoom_level_property_name;
};

struct _FmbStandardView
{
  GtkScrolledWindow __parent__;

  FmbPreferences         *preferences;

  FmbClipboardManager    *clipboard;
  FmbListModel           *model;

  GtkActionGroup            *action_group;
  GtkUIManager              *ui_manager;
  guint                      ui_merge_id;

  FmbIconFactory         *icon_factory;
  GtkCellRenderer           *icon_renderer;
  GtkCellRenderer           *name_renderer;

  BlxoBinding                *loading_binding;
  gboolean                   loading;

  FmbStandardViewPrivate *priv;
};

GType fmb_standard_view_get_type           (void) G_GNUC_CONST;

void  fmb_standard_view_context_menu       (FmbStandardView *standard_view,
                                               guint               button,
                                               guint32             time);

void  fmb_standard_view_queue_popup        (FmbStandardView *standard_view,
                                               GdkEventButton     *event);

void  fmb_standard_view_selection_changed  (FmbStandardView *standard_view);


void  fmb_standard_view_set_history            (FmbStandardView *standard_view,
                                                   FmbHistory      *history);

FmbHistory *fmb_standard_view_copy_history  (FmbStandardView *standard_view);

G_END_DECLS;

#endif /* !__FMB_STANDARD_VIEW_H__ */

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <fmb/fmb-private.h>
#include <fmb/fmb-view.h>



static void fmb_view_class_init (gpointer klass);



GType
fmb_view_get_type (void)
{
  static volatile gsize type__volatile = 0;
  GType                 type;

  if (g_once_init_enter (&type__volatile))
    {
      type = g_type_register_static_simple (G_TYPE_INTERFACE,
                                            I_("FmbView"),
                                            sizeof (FmbViewIface),
                                            (GClassInitFunc) fmb_view_class_init,
                                            0,
                                            NULL,
                                            0);

      g_type_interface_add_prerequisite (type, GTK_TYPE_WIDGET);
      g_type_interface_add_prerequisite (type, FMB_TYPE_COMPONENT);

      g_once_init_leave (&type__volatile, type);
    }

  return type__volatile;
}



static void
fmb_view_class_init (gpointer klass)
{
  /**
   * FmbView:loading:
   *
   * Indicates whether the given #FmbView is currently loading or
   * layouting its contents. Implementations should invoke
   * #g_object_notify() on this property whenever they start to load
   * the contents and then once they have finished loading.
   *
   * Other modules can use this property to display some kind of
   * user visible notification about the loading state, e.g. a
   * progress bar or an animated image.
   **/
  g_object_interface_install_property (klass,
                                       g_param_spec_boolean ("loading",
                                                             "loading",
                                                             "loading",
                                                             FALSE,
                                                             BLXO_PARAM_READABLE));

  /**
   * FmbView:statusbar-text:
   *
   * The text to be displayed in the status bar, which is associated
   * with this #FmbView instance. Implementations should invoke
   * #g_object_notify() on this property, whenever they have a new
   * text to be display in the status bar (e.g. the selection changed
   * or similar).
   **/
  g_object_interface_install_property (klass,
                                       g_param_spec_string ("statusbar-text",
                                                            "statusbar-text",
                                                            "statusbar-text",
                                                            NULL,
                                                            BLXO_PARAM_READABLE));

  /**
   * FmbView:show-hidden:
   *
   * Tells whether to display hidden and backup files in the
   * #FmbView or whether to hide them.
   **/
  g_object_interface_install_property (klass,
                                       g_param_spec_boolean ("show-hidden",
                                                             "show-hidden",
                                                             "show-hidden",
                                                             FALSE,
                                                             BLXO_PARAM_READWRITE));

  /**
   * FmbView:zoom-level:
   *
   * The #FmbZoomLevel at which the items within this
   * #FmbView should be displayed.
   **/
  g_object_interface_install_property (klass,
                                       g_param_spec_enum ("zoom-level",
                                                          "zoom-level",
                                                          "zoom-level",
                                                          FMB_TYPE_ZOOM_LEVEL,
                                                          FMB_ZOOM_LEVEL_NORMAL,
                                                          BLXO_PARAM_READWRITE));
}



/**
 * fmb_view_get_loading:
 * @view : a #FmbView instance.
 *
 * Tells whether the given #FmbView is currently loading or
 * layouting its contents.
 *
 * Return value: %TRUE if @view is currently being loaded, else %FALSE.
 **/
gboolean
fmb_view_get_loading (FmbView *view)
{
  _fmb_return_val_if_fail (FMB_IS_VIEW (view), FALSE);
  return (*FMB_VIEW_GET_IFACE (view)->get_loading) (view);
}



/**
 * fmb_view_get_statusbar_text:
 * @view : a #FmbView instance.
 *
 * Queries the text that should be displayed in the status bar
 * associated with @view.
 *
 * Return value: the text to be displayed in the status bar
 *               asssociated with @view.
 **/
const gchar*
fmb_view_get_statusbar_text (FmbView *view)
{
  _fmb_return_val_if_fail (FMB_IS_VIEW (view), NULL);
  return (*FMB_VIEW_GET_IFACE (view)->get_statusbar_text) (view);
}



/**
 * fmb_view_get_show_hidden:
 * @view : a #FmbView instance.
 *
 * Returns %TRUE if hidden and backup files are shown
 * in @view. Else %FALSE is returned.
 *
 * Return value: whether @view displays hidden files.
 **/
gboolean
fmb_view_get_show_hidden (FmbView *view)
{
  _fmb_return_val_if_fail (FMB_IS_VIEW (view), FALSE);
  return (*FMB_VIEW_GET_IFACE (view)->get_show_hidden) (view);
}



/**
 * fmb_view_set_show_hidden:
 * @view        : a #FmbView instance.
 * @show_hidden : &TRUE to display hidden files, else %FALSE.
 *
 * If @show_hidden is %TRUE then @view will display hidden and
 * backup files, else those files will be hidden from the user
 * interface.
 **/
void
fmb_view_set_show_hidden (FmbView *view,
                             gboolean    show_hidden)
{
  _fmb_return_if_fail (FMB_IS_VIEW (view));
  (*FMB_VIEW_GET_IFACE (view)->set_show_hidden) (view, show_hidden);
}



/**
 * fmb_view_get_zoom_level:
 * @view : a #FmbView instance.
 *
 * Returns the #FmbZoomLevel currently used for the @view.
 *
 * Return value: the #FmbZoomLevel currently used for the @view.
 **/
FmbZoomLevel
fmb_view_get_zoom_level (FmbView *view)
{
  _fmb_return_val_if_fail (FMB_IS_VIEW (view), FMB_ZOOM_LEVEL_NORMAL);
  return (*FMB_VIEW_GET_IFACE (view)->get_zoom_level) (view);
}



/**
 * fmb_view_set_zoom_level:
 * @view       : a #FmbView instance.
 * @zoom_level : the new #FmbZoomLevel for @view.
 *
 * Sets the zoom level used for @view to @zoom_level.
 **/
void
fmb_view_set_zoom_level (FmbView     *view,
                            FmbZoomLevel zoom_level)
{
  _fmb_return_if_fail (FMB_IS_VIEW (view));
  _fmb_return_if_fail (zoom_level < FMB_ZOOM_N_LEVELS);
  (*FMB_VIEW_GET_IFACE (view)->set_zoom_level) (view, zoom_level);
}



/**
 * fmb_view_reset_zoom_level:
 * @view : a #FmbView instance.
 *
 * Resets the zoom level of @view to the default
 * #FmbZoomLevel for @view.
 **/
void
fmb_view_reset_zoom_level (FmbView *view)
{
  _fmb_return_if_fail (FMB_IS_VIEW (view));
  (*FMB_VIEW_GET_IFACE (view)->reset_zoom_level) (view);
}



/**
 * fmb_view_reload:
 * @view : a #FmbView instance.
 * @reload_info : whether to reload file info for all files too
 *
 * Tells @view to reread the currently displayed folder
 * contents from the underlying media. If reload_info is
 * TRUE, it will reload information for all files too.
 **/
void
fmb_view_reload (FmbView *view,
                    gboolean    reload_info)
{
  _fmb_return_if_fail (FMB_IS_VIEW (view));
  (*FMB_VIEW_GET_IFACE (view)->reload) (view, reload_info);
}



/**
 * fmb_view_get_visible_range:
 * @view       : a #FmbView instance.
 * @start_file : return location for start of region, or %NULL.
 * @end_file   : return location for end of region, or %NULL.
 *
 * Sets @start_file and @end_file to be the first and last visible
 * #FmbFile.
 *
 * The files should be freed with g_object_unref() when no
 * longer needed.
 *
 * Return value: %TRUE if valid files were placed in @start_file
 *               and @end_file.
 **/
gboolean
fmb_view_get_visible_range (FmbView  *view,
                               FmbFile **start_file,
                               FmbFile **end_file)
{
  _fmb_return_val_if_fail (FMB_IS_VIEW (view), FALSE);
  return (*FMB_VIEW_GET_IFACE (view)->get_visible_range) (view, start_file, end_file);
}



/**
 * fmb_view_scroll_to_file:
 * @view        : a #FmbView instance.
 * @file        : the #FmbFile to scroll to.
 * @select_file : %TRUE to also select the @file in the @view.
 * @use_align   : whether to use alignment arguments.
 * @row_align   : the vertical alignment.
 * @col_align   : the horizontal alignment.
 *
 * Tells @view to scroll to the @file. If @view is currently
 * loading, it'll remember to scroll to @file later when
 * the contents are loaded.
 **/
void
fmb_view_scroll_to_file (FmbView *view,
                            FmbFile *file,
                            gboolean    select_file,
                            gboolean    use_align,
                            gfloat      row_align,
                            gfloat      col_align)
{
  _fmb_return_if_fail (FMB_IS_VIEW (view));
  _fmb_return_if_fail (FMB_IS_FILE (file));
  _fmb_return_if_fail (row_align >= 0.0f && row_align <= 1.0f);
  _fmb_return_if_fail (col_align >= 0.0f && col_align <= 1.0f);
  (*FMB_VIEW_GET_IFACE (view)->scroll_to_file) (view, file, select_file, use_align, row_align, col_align);
}



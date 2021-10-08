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

#include <fmb/fmb-navigator.h>
#include <fmb/fmb-private.h>



enum
{
  CHANGE_DIRECTORY,
  OPEN_NEW_TAB,
  LAST_SIGNAL,
};



static void fmb_navigator_base_init  (gpointer klass);
static void fmb_navigator_class_init (gpointer klass);



static guint navigator_signals[LAST_SIGNAL];



GType
fmb_navigator_get_type (void)
{
  static volatile gsize type__volatile = 0;
  GType                 type;

  if (g_once_init_enter (&type__volatile))
    {
      static const GTypeInfo info =
      {
        sizeof (FmbNavigatorIface),
        (GBaseInitFunc) fmb_navigator_base_init,
        NULL,
        (GClassInitFunc) fmb_navigator_class_init,
        NULL,
        NULL,
        0,
        0,
        NULL,
      };

      type = g_type_register_static (G_TYPE_INTERFACE, I_("FmbNavigator"), &info, 0);
      g_type_interface_add_prerequisite (type, G_TYPE_OBJECT);
    
      g_once_init_leave (&type__volatile, type);
    }

  return type__volatile;
}



static void
fmb_navigator_base_init (gpointer klass)
{
  static gboolean initialized = FALSE;

  if (G_UNLIKELY (!initialized))
    {
      /**
       * FmbNavigator::change-directory:
       * @navigator : a #FmbNavigator instance.
       * @directory : a #FmbFile referring to the new directory.
       *
       * Invoked by implementing classes whenever the user requests
       * to changed the current directory to @directory from within
       * the @navigator instance (e.g. for the location buttons bar,
       * this signal would be invoked whenever the user clicks on
       * a path button).
       *
       * The @navigator must not apply the @directory to the
       * "current-directory" property directly. But
       * instead, it must wait for the surrounding module (usually
       * a #FmbWindow instance) to explicitly inform the
       * @navigator to change it's current directory using
       * the #fmb_navigator_set_current_directory() method
       * or the "current-directory" property.
       **/
      navigator_signals[CHANGE_DIRECTORY] =
        g_signal_new (I_("change-directory"),
                      G_TYPE_FROM_INTERFACE (klass),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (FmbNavigatorIface, change_directory),
                      NULL, NULL,
                      g_cclosure_marshal_VOID__OBJECT,
                      G_TYPE_NONE, 1, FMB_TYPE_FILE);

      navigator_signals[OPEN_NEW_TAB] =
        g_signal_new (I_("open-new-tab"),
                      G_TYPE_FROM_INTERFACE (klass),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (FmbNavigatorIface, open_new_tab),
                      NULL, NULL,
                      g_cclosure_marshal_VOID__OBJECT,
                      G_TYPE_NONE, 1, FMB_TYPE_FILE);

      initialized = TRUE;
    }
}



static void
fmb_navigator_class_init (gpointer klass)
{
  /**
   * FmbNavigator:current-directory:
   *
   * The directory currently displayed by this #FmbNavigator
   * instance or %NULL if no directory is currently displayed
   * (it's up to the implementing class to define the appearance
   * of a navigator that has no directory associated with it).
   *
   * Whenever a navigator wants the surrounding module (usually
   * a #FmbWindow) to change to another directory, it should
   * invoke the "change-directory" signal using the
   * #fmb_navigator_change_directory() method. It should
   * not directly change the "current-directory" property,
   * but wait for the surrounding module to change the
   * "current-directory" property afterwards.
   **/
  g_object_interface_install_property (klass,
                                       g_param_spec_object ("current-directory",
                                                            "current-directory",
                                                            "current-directory",
                                                            FMB_TYPE_FILE,
                                                            BLXO_PARAM_READWRITE));
}



/**
 * fmb_navigator_get_current_directory:
 * @navigator : a #FmbNavigator instance.
 *
 * Returns the directory currently displayed by @navigator
 * or %NULL, if @navigator does not currently display and
 * directory.
 *
 * Return value: the current directory of @navigator or %NULL.
 **/
FmbFile*
fmb_navigator_get_current_directory (FmbNavigator *navigator)
{
  _fmb_return_val_if_fail (FMB_IS_NAVIGATOR (navigator), NULL);
  return FMB_NAVIGATOR_GET_IFACE (navigator)->get_current_directory (navigator);
}



/**
 * fmb_navigator_set_current_directory:
 * @navigator         : a #FmbNavigator instance.
 * @current_directory : the new directory to display or %NULL.
 *
 * Sets a new current directory that should be displayed by
 * the @navigator.
 **/
void
fmb_navigator_set_current_directory (FmbNavigator *navigator,
                                        FmbFile      *current_directory)
{
  _fmb_return_if_fail (FMB_IS_NAVIGATOR (navigator));
  _fmb_return_if_fail (current_directory == NULL || FMB_IS_FILE (current_directory));
  FMB_NAVIGATOR_GET_IFACE (navigator)->set_current_directory (navigator, current_directory);
}



/**
 * fmb_navigator_change_directory:
 * @navigator : a #FmbNavigator instance.
 * @directory : a #FmbFile referring to a directory.
 *
 * Emits the "change-directory" signal on @navigator with
 * the specified @directory.
 *
 * Derived classes should invoke this method whenever the user
 * selects a new directory from within @navigator. The derived
 * class should not perform any directory changing operations
 * itself, but leave it up to the surrounding module (usually
 * a #FmbWindow instance) to change the directory.
 *
 * It should never ever be called from outside a #FmbNavigator
 * implementation, as that may led to unexpected results!
 **/
void
fmb_navigator_change_directory (FmbNavigator *navigator,
                                   FmbFile      *directory)
{
  _fmb_return_if_fail (FMB_IS_NAVIGATOR (navigator));
  _fmb_return_if_fail (FMB_IS_FILE (directory));
  _fmb_return_if_fail (fmb_file_is_directory (directory));

  g_signal_emit (G_OBJECT (navigator), navigator_signals[CHANGE_DIRECTORY], 0, directory);
}



void
fmb_navigator_open_new_tab (FmbNavigator *navigator,
                               FmbFile      *directory)
{
  _fmb_return_if_fail (FMB_IS_NAVIGATOR (navigator));
  _fmb_return_if_fail (FMB_IS_FILE (directory));
  _fmb_return_if_fail (fmb_file_is_directory (directory));

  g_signal_emit (G_OBJECT (navigator), navigator_signals[OPEN_NEW_TAB], 0, directory);
}

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <blxo/blxo.h>

#include <fmb/fmb-component.h>
#include <fmb/fmb-private.h>



static void fmb_component_class_init (gpointer klass);



GType
fmb_component_get_type (void)
{
  static volatile gsize type__volatile = 0;
  GType                 type;

  if (g_once_init_enter (&type__volatile))
    {
      type = g_type_register_static_simple (G_TYPE_INTERFACE,
                                            I_("FmbComponent"),
                                            sizeof (FmbComponentIface),
                                            (GClassInitFunc) fmb_component_class_init,
                                            0,
                                            NULL,
                                            0);

      g_type_interface_add_prerequisite (type, FMB_TYPE_NAVIGATOR);

      g_once_init_leave (&type__volatile, type);
    }

  return type__volatile;
}



static void
fmb_component_class_init (gpointer klass)
{
  /**
   * FmbComponent:selected-files:
   *
   * The list of currently selected files for the #FmbWindow to
   * which this #FmbComponent belongs.
   *
   * The exact semantics of this property depend on the implementor
   * of this interface. For example, #FmbComponent<!---->s will update
   * the property depending on the users selection with the
   * #GtkTreeComponent or #BlxoIconComponent. While other components in a window,
   * like the #FmbShortcutsPane, will not update this property on
   * their own, but rely on #FmbWindow to synchronize the selected
   * files list with the selected files list from the active #FmbComponent.
   *
   * This way all components can behave properly depending on the
   * set of selected files even though they don't have direct access
   * to the #FmbComponent.
   **/
  g_object_interface_install_property (klass,
                                       g_param_spec_boxed ("selected-files",
                                                           "selected-files",
                                                           "selected-files",
                                                           FMBX_TYPE_FILE_INFO_LIST,
                                                           BLXO_PARAM_READWRITE));

  /**
   * FmbComponent:ui-manager:
   *
   * The UI manager used by the surrounding #FmbWindow. The
   * #FmbComponent implementations may connect additional actions
   * to the UI manager.
   **/
  g_object_interface_install_property (klass,
                                       g_param_spec_object ("ui-manager",
                                                            "ui-manager",
                                                            "ui-manager",
                                                            GTK_TYPE_UI_MANAGER,
                                                            BLXO_PARAM_READWRITE));
}



/**
 * fmb_component_get_selected_files:
 * @component : a #FmbComponent instance.
 *
 * Returns the set of selected files. Check the description
 * of the :selected-files property for details.
 *
 * Return value: the set of selected files.
 **/
GList*
fmb_component_get_selected_files (FmbComponent *component)
{
  _fmb_return_val_if_fail (FMB_IS_COMPONENT (component), NULL);
  return (*FMB_COMPONENT_GET_IFACE (component)->get_selected_files) (component);
}



/**
 * fmb_component_set_selected_files:
 * @component      : a #FmbComponent instance.
 * @selected_files : a #GList of #FmbFile<!---->s.
 *
 * Sets the selected files for @component to @selected_files.
 * Check the description of the :selected-files property for
 * details.
 **/
void
fmb_component_set_selected_files (FmbComponent *component,
                                     GList           *selected_files)
{
  _fmb_return_if_fail (FMB_IS_COMPONENT (component));
  (*FMB_COMPONENT_GET_IFACE (component)->set_selected_files) (component, selected_files);
}



/**
 * fmb_component_restore_selection:
 * @component      : a #FmbComponent instance.
 *
 * Make sure that the @selected_files stay selected when a @component
 * updates. This may be necessary on row changes etc.
 **/
void
fmb_component_restore_selection (FmbComponent *component)
{
  GList           *selected_files;

  _fmb_return_if_fail (FMB_IS_COMPONENT (component));

  selected_files = fmb_g_file_list_copy (fmb_component_get_selected_files (component));
  fmb_component_set_selected_files (component, selected_files);
  fmb_g_file_list_free (selected_files);
}



/**
 * fmb_component_get_ui_manager:
 * @component : a #FmbComponent instance.
 *
 * Returns the #GtkUIManager associated with @component or
 * %NULL if @component has no #GtkUIManager associated with
 * it.
 *
 * Return value: the #GtkUIManager associated with @component
 *               or %NULL.
 **/
GtkUIManager*
fmb_component_get_ui_manager (FmbComponent *component)
{
  _fmb_return_val_if_fail (FMB_IS_COMPONENT (component), NULL);
  return (*FMB_COMPONENT_GET_IFACE (component)->get_ui_manager) (component);
}



/**
 * fmb_component_set_ui_manager:
 * @component  : a #FmbComponent instance.
 * @ui_manager : a #GtkUIManager or %NULL.
 *
 * Installs a new #GtkUIManager for @component or resets the ::ui-manager
 * property.
 *
 * Implementations of the #FmbComponent interface must first disconnect
 * from any previously set #GtkUIManager and then connect to the
 * @ui_manager if not %NULL.
 **/
void
fmb_component_set_ui_manager (FmbComponent *component,
                                 GtkUIManager    *ui_manager)
{
  _fmb_return_if_fail (FMB_IS_COMPONENT (component));
  _fmb_return_if_fail (ui_manager == NULL || GTK_IS_UI_MANAGER (ui_manager));
  (*FMB_COMPONENT_GET_IFACE (component)->set_ui_manager) (component, ui_manager);
}





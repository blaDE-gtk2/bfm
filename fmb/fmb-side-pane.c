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
#include <fmb/fmb-side-pane.h>



static void fmb_side_pane_class_init (gpointer klass);



GType
fmb_side_pane_get_type (void)
{
  static volatile gsize type__volatile = 0;
  GType                 type;

  if (g_once_init_enter (&type__volatile))
    {
      type = g_type_register_static_simple (G_TYPE_INTERFACE,
                                            I_("FmbSidePane"),
                                            sizeof (FmbSidePaneIface),
                                            (GClassInitFunc) fmb_side_pane_class_init,
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
fmb_side_pane_class_init (gpointer klass)
{
  /**
   * FmbSidePane:show-hidden:
   *
   * Tells whether hidden folders will be displayed in
   * the #FmbSidePane instance.
   **/
  g_object_interface_install_property (klass,
                                       g_param_spec_boolean ("show-hidden",
                                                             "show-hidden",
                                                             "show-hidden",
                                                             FALSE,
                                                             BLXO_PARAM_READWRITE));
}



/**
 * fmb_side_pane_get_show_hidden:
 * @side_pane : a #FmbSidePane.
 *
 * Returns %TRUE if hidden folders are shown
 * in the @side_pane.
 *
 * Return value: %TRUE if hidden folders are
 *               shown in the @side_pane.
 **/
gboolean
fmb_side_pane_get_show_hidden (FmbSidePane *side_pane)
{
  _fmb_return_val_if_fail (FMB_IS_SIDE_PANE (side_pane), FALSE);
  return (*FMB_SIDE_PANE_GET_IFACE (side_pane)->get_show_hidden) (side_pane);
}



/**
 * fmb_side_pane_set_show_hidden:
 * @side_pane   : a #FmbSidePane.
 * @show_hidden : %TRUE to display hidden folders.
 *
 * If @show_hidden is %TRUE, hidden folders will be
 * shown in the @side_pane.
 **/
void
fmb_side_pane_set_show_hidden (FmbSidePane *side_pane,
                                  gboolean        show_hidden)
{
  _fmb_return_if_fail (FMB_IS_SIDE_PANE (side_pane));
  (*FMB_SIDE_PANE_GET_IFACE (side_pane)->set_show_hidden) (side_pane, show_hidden);
}


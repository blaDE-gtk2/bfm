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

#ifndef __FMB_COMPONENT_H__
#define __FMB_COMPONENT_H__

#include <fmb/fmb-navigator.h>

G_BEGIN_DECLS;

typedef struct _FmbComponentIface FmbComponentIface;
typedef struct _FmbComponent      FmbComponent;

#define FMB_TYPE_COMPONENT           (fmb_component_get_type ())
#define FMB_COMPONENT(obj)           (G_TYPE_CHECK_INSTANCE_CAST ((obj), FMB_TYPE_COMPONENT, FmbComponent))
#define FMB_IS_COMPONENT(obj)        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FMB_TYPE_COMPONENT))
#define FMB_COMPONENT_GET_IFACE(obj) (G_TYPE_INSTANCE_GET_INTERFACE ((obj), FMB_TYPE_COMPONENT, FmbComponentIface))

struct _FmbComponentIface
{
  GTypeInterface __parent__;

  /* methods */
  GList        *(*get_selected_files) (FmbComponent *component);
  void          (*set_selected_files) (FmbComponent *component,
                                       GList           *selected_files);

  GtkUIManager *(*get_ui_manager)     (FmbComponent *component);
  void          (*set_ui_manager)     (FmbComponent *component,
                                       GtkUIManager    *ui_manager);
};

GType         fmb_component_get_type           (void) G_GNUC_CONST;

GList        *fmb_component_get_selected_files  (FmbComponent *component);
void          fmb_component_set_selected_files  (FmbComponent *component,
                                                    GList           *selected_files);

void          fmb_component_restore_selection   (FmbComponent *component);

GtkUIManager *fmb_component_get_ui_manager      (FmbComponent *component);
void          fmb_component_set_ui_manager      (FmbComponent *component,
                                                    GtkUIManager    *ui_manager);

G_END_DECLS;

#endif /* !__FMB_COMPONENT_H__ */

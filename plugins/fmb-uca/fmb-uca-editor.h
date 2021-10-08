/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2005 Benedikt Meurer <benny@xfce.org>
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

#ifndef __FMB_UCA_EDITOR_H__
#define __FMB_UCA_EDITOR_H__

#include <fmb-uca/fmb-uca-model.h>

G_BEGIN_DECLS;

typedef struct _FmbUcaEditorClass FmbUcaEditorClass;
typedef struct _FmbUcaEditor      FmbUcaEditor;

#define FMB_UCA_TYPE_EDITOR            (fmb_uca_editor_get_type ())
#define FMB_UCA_EDITOR(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), FMB_UCA_TYPE_EDITOR, FmbUcaEditor))
#define FMB_UCA_EDITOR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), FMB_UCA_TYPE_EDITOR, FmbUcaEditorwClass))
#define FMB_UCA_IS_EDITOR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FMB_UCA_TYPE_EDITOR)) 
#define FMB_UCA_IS_EDITOR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), FMB_UCA_TYPE_EDITOR))
#define FMB_UCA_EDITOR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), FMB_UCA_TYPE_EDITOR, FmbUcaEditorwClass))

GType fmb_uca_editor_get_type      (void) G_GNUC_CONST;
void  fmb_uca_editor_register_type (FmbxProviderPlugin  *plugin);

void  fmb_uca_editor_load          (FmbUcaEditor        *uca_editor,
                                       FmbUcaModel         *uca_model,
                                       GtkTreeIter            *iter);
void  fmb_uca_editor_save          (FmbUcaEditor        *uca_editor,
                                       FmbUcaModel         *uca_model,
                                       GtkTreeIter            *iter);

G_END_DECLS;

#endif /* !__FMB_UCA_EDITOR_H__ */

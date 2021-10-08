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

#ifndef __FMB_UCA_CONTEXT_H__
#define __FMB_UCA_CONTEXT_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS;

typedef struct _FmbUcaContext FmbUcaContext;

FmbUcaContext *fmb_uca_context_new        (GtkWidget              *window,
                                                 GList                  *files);

FmbUcaContext *fmb_uca_context_ref        (FmbUcaContext       *context);
void              fmb_uca_context_unref      (FmbUcaContext       *context);

GList            *fmb_uca_context_get_files  (const FmbUcaContext *context);
GtkWidget        *fmb_uca_context_get_window (const FmbUcaContext *context);

G_END_DECLS;

#endif /* !__FMB_UCA_CONTEXT_H__ */

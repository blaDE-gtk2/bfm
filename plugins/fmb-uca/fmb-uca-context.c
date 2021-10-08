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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <fmbx/fmbx.h>

#include <fmb-uca/fmb-uca-context.h>



struct _FmbUcaContext
{
  gint       ref_count;
  GList     *files;
  GtkWidget *window;
};



/**
 * fmb_uca_context_new:
 * @window : a #GtkWindow.
 * @files  : a #GList of #FmbxFileInfo<!---->s.
 *
 * Allocates a new #FmbUcaContext with the given @window and
 * @files.
 *
 * The caller is responsible to free the returned context
 * using fmb_uca_context_unref().
 *
 * Return value: the newly allocated #FmbUcaContext.
 **/
FmbUcaContext*
fmb_uca_context_new (GtkWidget *window,
                        GList     *files)
{
  FmbUcaContext *context;

  context = g_new (FmbUcaContext, 1);
  context->ref_count = 1;
  context->window = window;
  context->files = fmbx_file_info_list_copy (files);

  /* add a weak reference on the window */
  if (G_LIKELY (context->window != NULL))
    g_object_add_weak_pointer (G_OBJECT (context->window), (gpointer) &context->window);

  return context;
}



/**
 * fmb_uca_context_ref:
 * @context : a #FmbUcaContext.
 *
 * Increases the reference count on @context.
 *
 * Return value: reference to @context.
 **/
FmbUcaContext*
fmb_uca_context_ref (FmbUcaContext *context)
{
  context->ref_count += 1;
  return context;
}



/**
 * fmb_uca_context_unref:
 * @context : a #FmbUcaContext.
 *
 * Decreases the reference count on @context
 * and frees @context once the reference count
 * drops to zero.
 **/
void
fmb_uca_context_unref (FmbUcaContext *context)
{
  if (--context->ref_count == 0)
    {
      /* drop the weak ref on the window */
      if (G_LIKELY (context->window != NULL))
        g_object_remove_weak_pointer (G_OBJECT (context->window), (gpointer) &context->window);

      /* release the file infos */
      fmbx_file_info_list_free (context->files);

      /* remove the context data */
      g_free (context);
    }
}



/**
 * fmb_uca_context_get_files:
 * @context : a #FmbUcaContext.
 *
 * Return value: the #FmbxFileInfo<!---->s for @context.
 **/
GList*
fmb_uca_context_get_files (const FmbUcaContext *context)
{
  return context->files;
}



/**
 * fmb_uca_context_get_window:
 * @context : a #FmbUcaContext.
 *
 * Returns the #GtkWindow for @context or %NULL if the
 * window was destroyed in the meantime.
 *
 * Return value: the #GtkWindow for @context.
 **/
GtkWidget*
fmb_uca_context_get_window (const FmbUcaContext *context)
{
  return context->window;
}



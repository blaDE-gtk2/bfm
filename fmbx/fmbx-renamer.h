/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2005-2006 Benedikt Meurer <benny@xfce.org>
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

#if !defined(FMBX_INSIDE_FMBX_H) && !defined(FMBX_COMPILATION)
#error "Only <fmbx/fmbx.h> can be included directly, this file may disappear or change contents"
#endif

#ifndef __FMBX_RENAMER_H__
#define __FMBX_RENAMER_H__

#include <gtk/gtk.h>

#include <fmbx/fmbx-file-info.h>

G_BEGIN_DECLS;

typedef struct _FmbxRenamerPrivate FmbxRenamerPrivate;
typedef struct _FmbxRenamerClass   FmbxRenamerClass;
typedef struct _FmbxRenamer        FmbxRenamer;

#define FMBX_TYPE_RENAMER            (fmbx_renamer_get_type ())
#define FMBX_RENAMER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), FMBX_TYPE_RENAMER, FmbxRenamer))
#define FMBX_RENAMER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), FMBX_TYPE_RENAMER, FmbxRenamerClass))
#define FMBX_IS_RENAMER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FMBX_TYPE_RENAMER))
#define FMBX_IS_RENAMER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), FMBX_TYPE_RENAMER))
#define FMBX_RENAMER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), FMBX_TYPE_RENAMER, FmbxRenamerClass))

struct _FmbxRenamerClass
{
  /*< private >*/
  GtkVBoxClass __parent__;

  /*< public >*/

  /* virtual methods */
  gchar *(*process)     (FmbxRenamer  *renamer,
                         FmbxFileInfo *file,
                         const gchar     *text,
                         guint            index);

  void   (*load)        (FmbxRenamer  *renamer,
                         GHashTable      *settings);
  void   (*save)        (FmbxRenamer  *renamer,
                         GHashTable      *settings);

  GList *(*get_actions) (FmbxRenamer  *renamer,
                         GtkWindow       *window,
                         GList           *files);

  /*< private >*/
  void (*reserved0) (void);
  void (*reserved1) (void);
  void (*reserved2) (void);
  void (*reserved3) (void);
  void (*reserved4) (void);

  /*< public >*/

  /* signals */
  void (*changed) (FmbxRenamer *renamer);

  /*< private >*/
  void (*reserved6) (void);
  void (*reserved7) (void);
  void (*reserved8) (void);
  void (*reserved9) (void);
};

struct _FmbxRenamer
{
  /*< private >*/
  GtkVBox                __parent__;
  FmbxRenamerPrivate *priv;
};

GType        fmbx_renamer_get_type     (void) G_GNUC_CONST;

const gchar *fmbx_renamer_get_help_url (FmbxRenamer   *renamer);
void         fmbx_renamer_set_help_url (FmbxRenamer   *renamer,
                                           const gchar      *help_url);

const gchar *fmbx_renamer_get_name     (FmbxRenamer   *renamer);
void         fmbx_renamer_set_name     (FmbxRenamer   *renamer,
                                           const gchar      *name);

gchar       *fmbx_renamer_process      (FmbxRenamer   *renamer,
                                           FmbxFileInfo  *file,
                                           const gchar      *text,
                                           guint             index) G_GNUC_MALLOC;

void         fmbx_renamer_load         (FmbxRenamer   *renamer,
                                           GHashTable       *settings);
void         fmbx_renamer_save         (FmbxRenamer   *renamer,
                                           GHashTable       *settings);

GList       *fmbx_renamer_get_actions  (FmbxRenamer   *renamer,
                                           GtkWindow        *window,
                                           GList            *files) G_GNUC_MALLOC;

void         fmbx_renamer_changed      (FmbxRenamer   *renamer);

G_END_DECLS;

#endif /* !__FMBX_RENAMER_H__ */

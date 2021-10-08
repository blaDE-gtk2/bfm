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

#ifndef __FMB_RENAMER_PAIR_H__
#define __FMB_RENAMER_PAIR_H__

#include <fmb/fmb-file.h>

G_BEGIN_DECLS;

typedef struct _FmbRenamerPair FmbRenamerPair;

#define FMB_TYPE_RENAMER_PAIR (fmb_renamer_pair_get_type ())

struct _FmbRenamerPair
{
  FmbFile *file;
  gchar      *name;
};

GType              fmb_renamer_pair_get_type   (void) G_GNUC_CONST;

FmbRenamerPair *fmb_renamer_pair_new        (FmbFile        *file,
                                                   const gchar       *name) G_GNUC_MALLOC;

void               fmb_renamer_pair_free       (gpointer           data);

GList             *fmb_renamer_pair_list_copy  (GList             *renamer_pair_list) G_GNUC_MALLOC;
void               fmb_renamer_pair_list_free  (GList             *renamer_pair_list);

G_END_DECLS;

#endif /* !__FMB_RENAMER_PAIR_H__ */

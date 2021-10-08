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

#ifndef __FMB_APR_ABSTRACT_PAGE_H__
#define __FMB_APR_ABSTRACT_PAGE_H__

#include <fmbx/fmbx.h>

G_BEGIN_DECLS;

typedef struct _FmbAprAbstractPageClass FmbAprAbstractPageClass;
typedef struct _FmbAprAbstractPage      FmbAprAbstractPage;

#define FMB_APR_TYPE_ABSTRACT_PAGE            (fmb_apr_abstract_page_get_type ())
#define FMB_APR_ABSTRACT_PAGE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), FMB_APR_TYPE_ABSTRACT_PAGE, FmbAprAbstractPage))
#define FMB_APR_ABSTRACT_PAGE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), FMB_APR_TYPE_ABSTRACT_PAGE, FmbAprAbstractPageClass))
#define FMB_APR_IS_ABSTRACT_PAGE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FMB_APR_TYPE_ABSTRACT_PAGE))
#define FMB_APR_IS_ABSTRACT_PAGE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), FMB_APR_TYPE_ABSTRACT_PAGE))
#define FMB_APR_ABSTRACT_PAGE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), FMB_APR_TYPE_ABSTRACT_PAGE, FmbAprAbstractPageClass))

struct _FmbAprAbstractPageClass
{
  FmbxPropertyPageClass __parent__;

  /* signals */
  void (*file_changed) (FmbAprAbstractPage *abstract_page,
                        FmbxFileInfo       *file);
};

struct _FmbAprAbstractPage
{
  FmbxPropertyPage __parent__;
  FmbxFileInfo    *file;
};

GType            fmb_apr_abstract_page_get_type      (void) G_GNUC_CONST;
void             fmb_apr_abstract_page_register_type (FmbxProviderPlugin *plugin);

FmbxFileInfo *fmb_apr_abstract_page_get_file      (FmbAprAbstractPage *abstract_page);
void             fmb_apr_abstract_page_set_file      (FmbAprAbstractPage *abstract_page,
                                                         FmbxFileInfo       *file);

G_END_DECLS;

#endif /* !__FMB_APR_ABSTRACT_PAGE_H__ */

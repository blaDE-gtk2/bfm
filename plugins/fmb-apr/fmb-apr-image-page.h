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

#ifndef __FMB_APR_IMAGE_PAGE_H__
#define __FMB_APR_IMAGE_PAGE_H__

#include <fmb-apr/fmb-apr-abstract-page.h>

G_BEGIN_DECLS;

typedef struct _FmbAprImagePageClass FmbAprImagePageClass;
typedef struct _FmbAprImagePage      FmbAprImagePage;

#define FMB_APR_TYPE_IMAGE_PAGE            (fmb_apr_image_page_get_type ())
#define FMB_APR_IMAGE_PAGE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), FMB_APR_TYPE_IMAGE_PAGE, FmbAprImagePage))
#define FMB_APR_IMAGE_PAGE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), FMB_APR_TYPE_IMAGE_PAGE, FmbAprImagePageClass))
#define FMB_APR_IS_IMAGE_PAGE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FMB_APR_TYPE_IMAGE_PAGE))
#define FMB_APR_IS_IMAGE_PAGE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), FMB_APR_TYPE_IMAGE_PAGE))
#define FMB_APR_IMAGE_PAGE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), FMB_APR_TYPE_IMAGE_PAGE, FmbAprImagePageClass))

GType fmb_apr_image_page_get_type      (void) G_GNUC_CONST;
void  fmb_apr_image_page_register_type (FmbxProviderPlugin *plugin);

G_END_DECLS;

#endif /* !__FMB_APR_IMAGE_PAGE_H__ */

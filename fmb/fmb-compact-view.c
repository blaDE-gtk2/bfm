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

#include <fmb/fmb-compact-view.h>



static AtkObject   *fmb_compact_view_get_accessible (GtkWidget               *widget);



struct _FmbCompactViewClass
{
  FmbAbstractIconViewClass __parent__;
};

struct _FmbCompactView
{
  FmbAbstractIconView __parent__;
};



G_DEFINE_TYPE (FmbCompactView, fmb_compact_view, FMB_TYPE_ABSTRACT_ICON_VIEW)



static void
fmb_compact_view_class_init (FmbCompactViewClass *klass)
{
  FmbStandardViewClass *fmbstandard_view_class;
  GtkWidgetClass          *gtkwidget_class;

  gtkwidget_class = GTK_WIDGET_CLASS (klass);
  gtkwidget_class->get_accessible = fmb_compact_view_get_accessible;

  fmbstandard_view_class = FMB_STANDARD_VIEW_CLASS (klass);
  fmbstandard_view_class->zoom_level_property_name = "last-compact-view-zoom-level";

  /* override FmbAbstractIconView default row spacing */
  gtk_rc_parse_string ("style\"fmb-compact-view-style\"{FmbCompactView::row-spacing=0}\n"
                       "class\"FmbCompactView\"style\"fmb-compact-view-style\"\n");
}



static void
fmb_compact_view_init (FmbCompactView *compact_view)
{
 /* initialize the icon view properties */
  blxo_icon_view_set_margin (BLXO_ICON_VIEW (GTK_BIN (compact_view)->child), 3);
  blxo_icon_view_set_layout_mode (BLXO_ICON_VIEW (GTK_BIN (compact_view)->child), BLXO_ICON_VIEW_LAYOUT_COLS);
  blxo_icon_view_set_orientation (BLXO_ICON_VIEW (GTK_BIN (compact_view)->child), GTK_ORIENTATION_HORIZONTAL);

  /* setup the icon renderer */
  g_object_set (G_OBJECT (FMB_STANDARD_VIEW (compact_view)->icon_renderer),
                "ypad", 2u,
                NULL);

  /* setup the name renderer (wrap only very long names) */
  g_object_set (G_OBJECT (FMB_STANDARD_VIEW (compact_view)->name_renderer),
                "wrap-mode", PANGO_WRAP_WORD_CHAR,
                "wrap-width", 1280,
                "xalign", 0.0f,
                "yalign", 0.5f,
                NULL);
}



static AtkObject*
fmb_compact_view_get_accessible (GtkWidget *widget)
{
  AtkObject *object;

  /* query the atk object for the icon view class */
  object = (*GTK_WIDGET_CLASS (fmb_compact_view_parent_class)->get_accessible) (widget);

  /* set custom Atk properties for the icon view */
  if (G_LIKELY (object != NULL))
    {
      atk_object_set_description (object, _("Compact directory listing"));
      atk_object_set_name (object, _("Compact view"));
      atk_object_set_role (object, ATK_ROLE_DIRECTORY_PANE);
    }

  return object;
}




/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2006 Benedikt Meurer <benny@xfce.org>
 * Copyright (c) 2009 Jannis Pohlmann <jannis@xfce.org>
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

#include <fmb/fmb-file.h>
#include <fmb/fmb-stock.h>
#include <fmb/fmb-private.h>
#include <fmb/fmb-trash-action.h>
#include <fmb/fmb-icon-factory.h>



static void fmb_trash_action_constructed (GObject                *object);
static void fmb_trash_action_finalize    (GObject                *object);
static void fmb_trash_action_changed     (FmbTrashAction      *trash_action,
                                             FmbFile             *trash_bin);


struct _FmbTrashActionClass
{
  GtkActionClass __parent__;
};

struct _FmbTrashAction
{
  GtkAction   __parent__;
  FmbFile *trash_bin;
};



G_DEFINE_TYPE (FmbTrashAction, fmb_trash_action, GTK_TYPE_ACTION)



static void
fmb_trash_action_class_init (FmbTrashActionClass *klass)
{
  GObjectClass *gobject_class;

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->constructed = fmb_trash_action_constructed;
  gobject_class->finalize = fmb_trash_action_finalize;
}



static void
fmb_trash_action_init (FmbTrashAction *trash_action)
{
  GFile *trash_bin;

  /* try to connect to the trash bin */
  trash_bin = fmb_g_file_new_for_trash ();
  trash_action->trash_bin = fmb_file_get (trash_bin, NULL);
  g_object_unref (trash_bin);

  /* safety check for trash bin... */
  if (G_LIKELY (trash_action->trash_bin != NULL))
    {
      /* watch the trash bin for changes */
      fmb_file_watch (trash_action->trash_bin);

      /* stay informed about changes to the trash bin */
      g_signal_connect_swapped (G_OBJECT (trash_action->trash_bin), "changed",
                                G_CALLBACK (fmb_trash_action_changed),
                                trash_action);

      /* initially update the stock icon */
      fmb_trash_action_changed (trash_action, trash_action->trash_bin);

      /* schedule a reload in idle (fix for bug #9513) */
      fmb_file_reload_idle (trash_action->trash_bin);
    }
}



static void
fmb_trash_action_constructed (GObject *object)
{
  FmbTrashAction *trash_action = FMB_TRASH_ACTION (object);
  const gchar       *label;
  
  if (trash_action->trash_bin != NULL)
    label = fmb_file_get_display_name (trash_action->trash_bin);
  else
    label = _("T_rash");

  g_object_set (trash_action, "label", label, NULL);
}



static void
fmb_trash_action_finalize (GObject *object)
{
  FmbTrashAction *trash_action = FMB_TRASH_ACTION (object);

  /* check if we are connected to the trash bin */
  if (G_LIKELY (trash_action->trash_bin != NULL))
    {
      /* unwatch the trash bin */
      fmb_file_unwatch (trash_action->trash_bin);

      /* release the trash bin */
      g_signal_handlers_disconnect_by_func (G_OBJECT (trash_action->trash_bin), fmb_trash_action_changed, trash_action);
      g_object_unref (G_OBJECT (trash_action->trash_bin));
    }

  (*G_OBJECT_CLASS (fmb_trash_action_parent_class)->finalize) (object);
}



static void
fmb_trash_action_changed (FmbTrashAction *trash_action,
                             FmbFile        *trash_bin)
{
  _fmb_return_if_fail (FMB_IS_TRASH_ACTION (trash_action));
  _fmb_return_if_fail (trash_action->trash_bin == trash_bin);
  _fmb_return_if_fail (FMB_IS_FILE (trash_bin));

  /* unset the pixmap cache on the file */
  fmb_icon_factory_clear_pixmap_cache (trash_bin);

  /* adjust the stock icon appropriately */
  if (fmb_file_get_item_count (trash_bin) > 0) 
    g_object_set (G_OBJECT (trash_action), "stock-id", FMB_STOCK_TRASH_FULL, NULL);
  else
    g_object_set (G_OBJECT (trash_action), "stock-id", FMB_STOCK_TRASH_EMPTY, NULL);
}



/**
 * fmb_trash_action_new:
 *
 * Allocates a new #FmbTrashAction, whose associated widgets update their icons according to the
 * current trash state.
 *
 * Return value: the newly allocated #FmbTrashAction.
 **/
GtkAction*
fmb_trash_action_new (void)
{
  return g_object_new (FMB_TYPE_TRASH_ACTION,
                       "name", "open-trash",
                       "tooltip", _("Display the contents of the trash can"),
                       "stock-id", FMB_STOCK_TRASH_FULL,
                       NULL);
}


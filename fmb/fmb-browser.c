/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2009 Jannis Pohlmann <jannis@xfce.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib-object.h>

#include <fmb/fmb-browser.h>
#include <fmb/fmb-file.h>
#include <fmb/fmb-private.h>
#include <fmb/fmb-util.h>
#include <fmb/fmb-gtk-extensions.h>



typedef struct _PokeFileData   PokeFileData;
typedef struct _PokeDeviceData PokeDeviceData;



static void fmb_browser_poke_file_internal (FmbBrowser                *browser,
                                               GFile                        *location,
                                               FmbFile                   *source,
                                               FmbFile                   *file,
                                               gpointer                      widget,
                                               FmbBrowserPokeFileFunc     func,
                                               FmbBrowserPokeLocationFunc location_func,
                                               gpointer                      user_data);



struct _PokeFileData
{
  GFile                        *location;
  FmbBrowser                *browser;
  FmbFile                   *source;
  FmbFile                   *file;
  FmbBrowserPokeFileFunc     func;
  FmbBrowserPokeLocationFunc location_func;
  gpointer                      user_data;
};

struct _PokeDeviceData
{
  FmbBrowser              *browser;
  FmbDevice               *device;
  FmbFile                 *mount_point;
  FmbBrowserPokeDeviceFunc func;
  gpointer                    user_data;
};



GType
fmb_browser_get_type (void)
{
  static volatile gsize type__volatile = 0;
  GType                 type;

  if (g_once_init_enter (&type__volatile))
    {
      type = g_type_register_static_simple (G_TYPE_INTERFACE,
                                            I_("FmbBrowser"),
                                            sizeof (FmbBrowserIface),
                                            NULL,
                                            0,
                                            NULL,
                                            0);

      g_type_interface_add_prerequisite (type, G_TYPE_OBJECT);

      g_once_init_leave (&type__volatile, type);
    }

  return type__volatile;
}



static PokeFileData *
fmb_browser_poke_file_data_new (FmbBrowser                *browser,
                                   GFile                        *location,
                                   FmbFile                   *source,
                                   FmbFile                   *file,
                                   FmbBrowserPokeFileFunc     func,
                                   FmbBrowserPokeLocationFunc location_func,
                                   gpointer                      user_data)
{
  PokeFileData *poke_data;

  _fmb_return_val_if_fail (FMB_IS_BROWSER (browser), NULL);

  poke_data = g_slice_new0 (PokeFileData);
  poke_data->browser = g_object_ref (browser);

  if (location != NULL)
    poke_data->location = g_object_ref (location);

  if (source != NULL)
    poke_data->source = g_object_ref (source);

  if (file != NULL)
    poke_data->file = g_object_ref (file);

  poke_data->func = func;
  poke_data->location_func = location_func;

  poke_data->user_data = user_data;

  return poke_data;
}



static void
fmb_browser_poke_file_data_free (PokeFileData *poke_data)
{
  _fmb_return_if_fail (poke_data != NULL);
  _fmb_return_if_fail (FMB_IS_BROWSER (poke_data->browser));

  g_object_unref (poke_data->browser);

  if (poke_data->location != NULL)
    g_object_unref (poke_data->location);
  if (poke_data->source != NULL)
    g_object_unref (poke_data->source);
  if (poke_data->file != NULL)
    g_object_unref (poke_data->file);

  g_slice_free (PokeFileData, poke_data);
}



static PokeDeviceData *
fmb_browser_poke_device_data_new (FmbBrowser              *browser,
                                     FmbDevice               *device,
                                     FmbBrowserPokeDeviceFunc func,
                                     gpointer                    user_data)
{
  PokeDeviceData *poke_data;

  _fmb_return_val_if_fail (FMB_IS_BROWSER (browser), NULL);
  _fmb_return_val_if_fail (FMB_IS_DEVICE (device), NULL);

  poke_data = g_slice_new0 (PokeDeviceData);
  poke_data->browser = g_object_ref (browser);
  poke_data->device = g_object_ref (device);
  poke_data->func = func;
  poke_data->user_data = user_data;

  return poke_data;
}



static void
fmb_browser_poke_device_data_free (PokeDeviceData *poke_data)
{
  _fmb_return_if_fail (poke_data != NULL);
  _fmb_return_if_fail (FMB_IS_BROWSER (poke_data->browser));
  _fmb_return_if_fail (FMB_IS_DEVICE (poke_data->device));

  g_object_unref (poke_data->browser);
  g_object_unref (poke_data->device);

  g_slice_free (PokeDeviceData, poke_data);
}



static void
fmb_browser_poke_mountable_file_finish (GFile      *location,
                                           FmbFile *file,
                                           GError     *error,
                                           gpointer    user_data)
{
  PokeFileData *poke_data = user_data;

  _fmb_return_if_fail (G_IS_FILE (location));
  _fmb_return_if_fail (user_data != NULL);
  _fmb_return_if_fail (FMB_IS_BROWSER (poke_data->browser));
  _fmb_return_if_fail (FMB_IS_FILE (poke_data->file));
  _fmb_return_if_fail (FMB_IS_FILE (poke_data->source));

  if (poke_data->location_func != NULL)
    {
      (poke_data->location_func) (poke_data->browser,
                                  poke_data->location,
                                  poke_data->source,
                                  file,
                                  error,
                                  poke_data->user_data);
    }

  if (poke_data->func != NULL)
    {
      (poke_data->func) (poke_data->browser, poke_data->source, file, error,
                         poke_data->user_data);
    }

  fmb_browser_poke_file_data_free (poke_data);
}



static void
fmb_browser_poke_mountable_finish (GObject      *object,
                                      GAsyncResult *result,
                                      gpointer      user_data)
{
  PokeFileData *poke_data = user_data;
  GError       *error = NULL;
  GFile        *location;

  _fmb_return_if_fail (G_IS_FILE (object));
  _fmb_return_if_fail (G_IS_ASYNC_RESULT (result));
  _fmb_return_if_fail (user_data != NULL);
  _fmb_return_if_fail (FMB_IS_BROWSER (poke_data->browser));
  _fmb_return_if_fail (FMB_IS_FILE (poke_data->file));

  if (!g_file_mount_mountable_finish (G_FILE (object), result, &error))
    {
      if (error->domain == G_IO_ERROR)
        {
          if (error->code == G_IO_ERROR_ALREADY_MOUNTED)
            g_clear_error (&error);
        }
    }

  if (error == NULL)
    {
      fmb_file_reload (poke_data->file);

      location = fmb_file_get_target_location (poke_data->file);

      /* resolve the FmbFile for the target location asynchronously
       * and defer cleaning up the poke data until that has finished */
      fmb_file_get_async (location, NULL,
                             fmb_browser_poke_mountable_file_finish,
                             poke_data);

      g_object_unref (location);
    }
  else
    {
      if (poke_data->location_func != NULL)
        {
          (poke_data->location_func) (poke_data->browser,
                                      poke_data->location,
                                      poke_data->source,
                                      NULL,
                                      error,
                                      poke_data->user_data);
        }

      if (poke_data->func != NULL)
        {
          (poke_data->func) (poke_data->browser, poke_data->source, NULL, error,
                             poke_data->user_data);
        }

      fmb_browser_poke_file_data_free (poke_data);
    }

  g_clear_error (&error);
}



static void
fmb_browser_poke_file_finish (GObject      *object,
                                 GAsyncResult *result,
                                 gpointer      user_data)
{
  PokeFileData *poke_data = user_data;
  GError       *error = NULL;

  _fmb_return_if_fail (G_IS_FILE (object));
  _fmb_return_if_fail (G_IS_ASYNC_RESULT (result));
  _fmb_return_if_fail (user_data != NULL);
  _fmb_return_if_fail (FMB_IS_BROWSER (poke_data->browser));
  _fmb_return_if_fail (FMB_IS_FILE (poke_data->file));

  if (!g_file_mount_enclosing_volume_finish (G_FILE (object), result, &error))
    {
      if (error->domain == G_IO_ERROR)
        {
          if (error->code == G_IO_ERROR_ALREADY_MOUNTED)
            g_clear_error (&error);
        }
    }

  if (error == NULL)
    fmb_file_reload (poke_data->file);

  if (poke_data->location_func != NULL)
    {
      if (error == NULL)
        {
          (poke_data->location_func) (poke_data->browser,
                                      poke_data->location,
                                      poke_data->source,
                                      poke_data->file,
                                      NULL,
                                      poke_data->user_data);
        }
      else
        {
          (poke_data->location_func) (poke_data->browser,
                                      poke_data->location,
                                      poke_data->source,
                                      NULL,
                                      error,
                                      poke_data->user_data);
        }
    }

  if (poke_data->func != NULL)
    {
      if (error == NULL)
        {
          (poke_data->func) (poke_data->browser, poke_data->source, poke_data->file,
                             NULL, poke_data->user_data);
        }
      else
        {
          (poke_data->func) (poke_data->browser, poke_data->source, NULL, error,
                             poke_data->user_data);
        }
    }

  g_clear_error (&error);

  fmb_browser_poke_file_data_free (poke_data);
}



static void
fmb_browser_poke_shortcut_file_finish (GFile *location,
                                          FmbFile *file,
                                          GError *error,
                                          gpointer user_data)
{
  PokeFileData *poke_data = user_data;

  _fmb_return_if_fail (G_IS_FILE (location));
  _fmb_return_if_fail (user_data != NULL);
  _fmb_return_if_fail (FMB_IS_BROWSER (poke_data->browser));
  _fmb_return_if_fail (FMB_IS_FILE (poke_data->file));

  if (error == NULL)
    {
      fmb_browser_poke_file_internal (poke_data->browser,
                                         poke_data->location,
                                         poke_data->source,
                                         file,
                                         NULL,
                                         poke_data->func,
                                         poke_data->location_func,
                                         poke_data->user_data);
    }
  else
    {
      if (poke_data->location_func != NULL)
        {
          (poke_data->location_func) (poke_data->browser,
                                      poke_data->location,
                                      poke_data->source,
                                      NULL,
                                      error,
                                      poke_data->user_data);
        }

      if (poke_data->func != NULL)
        {
          (poke_data->func) (poke_data->browser, poke_data->source, NULL, error,
                             poke_data->user_data);
        }
    }

  fmb_browser_poke_file_data_free (poke_data);
}



static void
fmb_browser_poke_file_internal (FmbBrowser                *browser,
                                   GFile                        *location,
                                   FmbFile                   *source,
                                   FmbFile                   *file,
                                   gpointer                      widget,
                                   FmbBrowserPokeFileFunc     func,
                                   FmbBrowserPokeLocationFunc location_func,
                                   gpointer                      user_data)
{
  GMountOperation *mount_operation;
  PokeFileData    *poke_data;
  GFile           *target;

  _fmb_return_if_fail (FMB_IS_BROWSER (browser));
  _fmb_return_if_fail (G_IS_FILE (location));
  _fmb_return_if_fail (FMB_IS_FILE (source));
  _fmb_return_if_fail (FMB_IS_FILE (file));

  if (fmb_file_get_kind (file) == G_FILE_TYPE_SHORTCUT)
    {
      target = fmb_file_get_target_location (file);

      poke_data = fmb_browser_poke_file_data_new (browser, location, source,
                                                     file, func, location_func, user_data);

      fmb_file_get_async (target, NULL,
                             fmb_browser_poke_shortcut_file_finish,
                             poke_data);

      g_object_unref (target);
    }
  else if (fmb_file_get_kind (file) == G_FILE_TYPE_MOUNTABLE)
    {
      if (fmb_file_is_mounted (file))
        {
          target = fmb_file_get_target_location (file);

          poke_data = fmb_browser_poke_file_data_new (browser, location, source,
                                                         file, func, location_func, user_data);

          fmb_file_get_async (target, NULL,
                                 fmb_browser_poke_mountable_file_finish,
                                 poke_data);

          g_object_unref (target);
        }
      else
        {
          poke_data = fmb_browser_poke_file_data_new (browser, location, source,
                                                         file, func, location_func, user_data);

          mount_operation = fmb_gtk_mount_operation_new (widget);

          g_file_mount_mountable (fmb_file_get_file (file),
                                  G_MOUNT_MOUNT_NONE, mount_operation, NULL,
                                  fmb_browser_poke_mountable_finish,
                                  poke_data);

          g_object_unref (mount_operation);
        }
    }
  else if (!fmb_file_is_mounted (file))
    {
      poke_data = fmb_browser_poke_file_data_new (browser, location, source,
                                                     file, func, location_func, user_data);

      mount_operation = fmb_gtk_mount_operation_new (widget);

      g_file_mount_enclosing_volume (fmb_file_get_file (file),
                                     G_MOUNT_MOUNT_NONE, mount_operation, NULL,
                                     fmb_browser_poke_file_finish,
                                     poke_data);

      g_object_unref (mount_operation);
    }
  else
    {
      if (location_func != NULL)
        location_func (browser, location, source, file, NULL, user_data);

      if (func != NULL)
        func (browser, source, file, NULL, user_data);
    }
}



/**
 * fmb_browser_poke_file:
 * @browser : a #FmbBrowser.
 * @file      : a #FmbFile.
 * @widget    : a #GtkWidget, a #GdkScreen or %NULL.
 * @func      : a #FmbBrowserPokeFileFunc callback or %NULL.
 * @user_data : pointer to arbitrary user data or %NULL.
 *
 * Pokes a #FmbFile to see what's behind it.
 *
 * If @file has the type %G_FILE_TYPE_SHORTCUT, it tries to load and mount
 * the file that is referred to by the %G_FILE_ATTRIBUTE_STANDARD_TARGET_URI
 * of the @file.
 *
 * If @file has the type %G_FILE_TYPE_MOUNTABLE, it tries to mount the @file.
 *
 * In the other cases, if the enclosing volume of the @file is not mounted
 * yet, it tries to mount it.
 *
 * When finished or if an error occured, it calls @func with the provided
 * @user data. The #GError parameter to @func is set if, and only if there
 * was an error.
 **/
void
fmb_browser_poke_file (FmbBrowser            *browser,
                          FmbFile               *file,
                          gpointer                  widget,
                          FmbBrowserPokeFileFunc func,
                          gpointer                  user_data)
{
  _fmb_return_if_fail (FMB_IS_BROWSER (browser));
  _fmb_return_if_fail (FMB_IS_FILE (file));

  fmb_browser_poke_file_internal (browser, fmb_file_get_file (file), file, file, widget,
                                     func, NULL, user_data);
}



static void
fmb_browser_poke_device_file_finish (GFile      *location,
                                        FmbFile *file,
                                        GError     *error,
                                        gpointer    user_data)
{
  PokeDeviceData *poke_data = user_data;

  _fmb_return_if_fail (G_IS_FILE (location));
  _fmb_return_if_fail (user_data != NULL);
  _fmb_return_if_fail (FMB_IS_BROWSER (poke_data->browser));
  _fmb_return_if_fail (FMB_IS_DEVICE (poke_data->device));

  if (poke_data->func != NULL)
    {
      (poke_data->func) (poke_data->browser, poke_data->device, file, error,
                         poke_data->user_data);
    }

  fmb_browser_poke_device_data_free (poke_data);
}



static void
fmb_browser_poke_device_finish (FmbDevice *device,
                                   const GError *error,
                                   gpointer      user_data)
{
  PokeDeviceData *poke_data = user_data;
  GFile          *mount_point;

  _fmb_return_if_fail (FMB_IS_DEVICE (device));
  _fmb_return_if_fail (user_data != NULL);
  _fmb_return_if_fail (FMB_IS_BROWSER (poke_data->browser));
  _fmb_return_if_fail (FMB_IS_DEVICE (poke_data->device));
  _fmb_return_if_fail (device == poke_data->device);

  if (error == NULL)
    {
      mount_point = fmb_device_get_root (device);

      /* resolve the FmbFile for the mount point asynchronously
       * and defer cleaning up the poke data until that has finished */
      fmb_file_get_async (mount_point, NULL,
                             fmb_browser_poke_device_file_finish,
                             poke_data);

      g_object_unref (mount_point);
    }
  else
    {
      if (poke_data->func != NULL)
        {
          (poke_data->func) (poke_data->browser, poke_data->device, NULL, (GError *) error,
                             poke_data->user_data);
        }

      fmb_browser_poke_device_data_free (poke_data);
    }
}



/**
 * fmb_browser_poke_device:
 * @browser   : a #FmbBrowser.
 * @device    : a #FmbDevice.
 * @widget    : a #GtkWidget, a #GdkScreen or %NULL.
 * @func      : a #FmbBrowserPokeDeviceFunc callback or %NULL.
 * @user_data : pointer to arbitrary user data or %NULL.
 *
 * This function checks if @device is mounted or not. If it is, it loads
 * a #FmbFile for the mount root and calls @func. If it is not mounted,
 * it first mounts the device asynchronously and calls @func with the
 * #FmbFile corresponding to the mount root when the mounting is finished.
 *
 * The #FmbFile passed to @func will be %NULL if, and only if mounting
 * the @device failed. The #GError passed to @func will be set if, and only if
 * mounting failed.
 **/
void
fmb_browser_poke_device (FmbBrowser              *browser,
                            FmbDevice               *device,
                            gpointer                    widget,
                            FmbBrowserPokeDeviceFunc func,
                            gpointer                    user_data)
{
  GMountOperation *mount_operation;
  PokeDeviceData  *poke_data;
  GFile           *mount_point;

  _fmb_return_if_fail (FMB_IS_BROWSER (browser));
  _fmb_return_if_fail (FMB_DEVICE (device));

  if (fmb_device_is_mounted (device))
    {
      mount_point = fmb_device_get_root (device);

      poke_data = fmb_browser_poke_device_data_new (browser, device, func, user_data);

      fmb_file_get_async (mount_point, NULL,
                             fmb_browser_poke_device_file_finish,
                             poke_data);

      g_object_unref (mount_point);
    }
  else
    {
      poke_data = fmb_browser_poke_device_data_new (browser, device, func, user_data);

      mount_operation = fmb_gtk_mount_operation_new (widget);

      fmb_device_mount (device,
                           mount_operation,
                           NULL,
                           fmb_browser_poke_device_finish,
                           poke_data);

      g_object_unref (mount_operation);
    }
}



static void
fmb_browser_poke_location_file_finish (GFile      *location,
                                          FmbFile *file,
                                          GError     *error,
                                          gpointer    user_data)
{
  PokeFileData *poke_data = user_data;

  _fmb_return_if_fail (G_IS_FILE (location));
  _fmb_return_if_fail (user_data != NULL);
  _fmb_return_if_fail (FMB_IS_BROWSER (poke_data->browser));
  _fmb_return_if_fail (G_IS_FILE (poke_data->location));

  if (error == NULL)
    {
      fmb_browser_poke_file_internal (poke_data->browser,
                                         location,
                                         file,
                                         file,
                                         NULL,
                                         poke_data->func,
                                         poke_data->location_func,
                                         poke_data->user_data);
    }
  else
    {
      if (poke_data->location_func != NULL)
        {
          (poke_data->location_func) (poke_data->browser, location, NULL, NULL, error,
                                      poke_data->user_data);
        }
    }

  fmb_browser_poke_file_data_free (poke_data);
}



/**
 * fmb_browser_poke_location:
 * @browser   : a #FmbBrowser.
 * @location  : a #GFile.
 * @widget    : a #GtkWidget, a #GdkScreen or %NULL.
 * @func      : a #FmbBrowserPokeDeviceFunc callback or %NULL.
 * @user_data : pointer to arbitrary user data or %NULL.
 *
 * Pokes a #GFile to see what's behind it.
 *
 * It first resolves the #GFile into a #FmbFile asynchronously using
 * fmb_file_get_async(). It then performs the same steps as
 * fmb_browser_poke_file().
 **/
void
fmb_browser_poke_location (FmbBrowser                *browser,
                              GFile                        *location,
                              gpointer                      widget,
                              FmbBrowserPokeLocationFunc func,
                              gpointer                      user_data)
{
  PokeFileData *poke_data;

  _fmb_return_if_fail (FMB_IS_BROWSER (browser));
  _fmb_return_if_fail (G_IS_FILE (location));

  poke_data = fmb_browser_poke_file_data_new (browser, location, NULL, NULL,
                                                 NULL, func, user_data);

  fmb_file_get_async (location, NULL,
                         fmb_browser_poke_location_file_finish,
                         poke_data);
}

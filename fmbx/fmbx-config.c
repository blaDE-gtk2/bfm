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

#include <fmbx/fmbx-config.h>



/**
 * fmbx_major_version:
 *
 * The major version number of the
 * <systemitem class="library">fmbx</systemitem> library (e.g. in
 * version 0.5.1 this is 0).
 *
 * This variable is in the library, so represents the
 * <systemitem class="library">fmbx</systemitem> library you have
 * linked against. Contrast with the #FMBX_MAJOR_VERSION macro, which
 * represents the major version of the
 * <systemitem class="library">fmbx</systemitem> headers you have
 * included.
 **/
const guint fmbx_major_version = FMBX_MAJOR_VERSION;



/**
 * fmbx_minor_version:
 *
 * The minor version number of the
 * <systemitem class="library">fmbx</systemitem> library (e.g. in
 * version 0.5.1 this is 5).
 *
 * This variable is in the library, so represents the
 * <systemitem class="library">fmbx</systemitem> library you have
 * linked against. Contrast with the #FMBX_MINOR_VERSION macro, which
 * represents the minor version of the
 * <systemitem class="library">fmbx</systemitem> headers you have
 * included.
 **/
const guint fmbx_minor_version = FMBX_MINOR_VERSION;



/**
 * fmbx_micro_version:
 *
 * The micro version number of the
 * <systemitem class="library">fmbx</systemitem> library (e.g. in
 * version 0.5.1 this is 1).
 *
 * This variable is in the library, so represents the
 * <systemitem class="library">fmbx</systemitem> library you have
 * linked against. Contrast with the #FMBX_MICRO_VERSION macro, which
 * represents the micro version of the
 * <systemitem class="library">fmbx</systemitem> headers you have
 * included.
 **/
const guint fmbx_micro_version = FMBX_MICRO_VERSION;



/**
 * fmbx_check_version:
 * @required_major : the required major version.
 * @required_minor : the required minor version.
 * @required_micro : the required micro version.
 *
 * Checks that the <systemitem class="library">fmbx</systemitem> library
 * in use is compatible with the given version. Generally you would pass in
 * the constants #FMBX_MAJOR_VERSION, #FMBX_MINOR_VERSION and
 * #FMBX_VERSION_MICRO as the three arguments to this function; that produces
 * a check that the library in use is compatible with the version of
 * <systemitem class="library">fmbx</systemitem> the extension was
 * compiled against.
 *
 * This function should be called by extensions in the
 * fmb_extension_initialize() method to verify that the <systemitem
 * class="library">fmbx</systemitem> library used by file manager is
 * compatible with the version the extension was compiled with.
 *
 * <example>
 * <title>Checking the runtime version of the Fmb Extension library</title>
 * <programlisting>
 * const gchar *mismatch;
 * mismatch = fmbx_check_version (FMBX_VERSION_MAJOR,
 *                                   FMBX_VERSION_MINOR,
 *                                   FMBX_VERSION_MICRO);
 * if (G_UNLIKELY (mismatch != NULL))
 *   g_error ("Version mismatch: %<!---->s", mismatch);
 * </programlisting>
 * </example>
 *
 * Return value: %NULL if the library is compatible with the given version,
 *               or a string describing the version mismatch. The returned
 *               string is owned by the library and must not be freed or
 *               modified by the caller.
 **/
const gchar*
fmbx_check_version (guint required_major,
                       guint required_minor,
                       guint required_micro)
{
  return NULL;
}

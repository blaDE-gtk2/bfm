/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2006 Benedikt Meurer <benny@xfce.org>
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

#include <glib/gi18n-lib.h>

#include <fmb-sbr/fmb-sbr-enum-types.h>



static GType case_renamer_mode_type;
static GType insert_mode_type;
static GType number_mode_type;
static GType offset_mode_type;
static GType text_mode_type;
static GType date_mode_type;



GType
fmb_sbr_case_renamer_mode_get_type (void)
{
  return case_renamer_mode_type;
}



GType
fmb_sbr_insert_mode_get_type (void)
{
  return insert_mode_type;
}



GType
fmb_sbr_number_mode_get_type (void)
{
  return number_mode_type;
}



GType
fmb_sbr_offset_mode_get_type (void)
{
  return offset_mode_type;
}



GType
fmb_sbr_text_mode_get_type (void)
{
  return text_mode_type;
}



GType
fmb_sbr_date_mode_get_type (void)
{
  return date_mode_type;
}



void
fmb_sbr_register_enum_types (FmbxProviderPlugin *plugin)
{
  static const GEnumValue case_renamer_mode_values[] =
  {
    { FMB_SBR_CASE_RENAMER_MODE_LOWER,    "FMB_SBR_CASE_RENAMER_MODE_LOWER",    N_ ("lowercase"),     },
    { FMB_SBR_CASE_RENAMER_MODE_UPPER,    "FMB_SBR_CASE_RENAMER_MODE_UPPER",    N_ ("UPPERCASE"),     },
    { FMB_SBR_CASE_RENAMER_MODE_CAMEL,    "FMB_SBR_CASE_RENAMER_MODE_CAMEL",    N_ ("Camelcase")    , },
    { FMB_SBR_CASE_RENAMER_MODE_SENTENCE, "FMB_SBR_CASE_RENAMER_MODE_SENTENCE", N_ ("Sentence case"), },
    { 0,                                     NULL,                                    NULL,                 },
  };

  static const GEnumValue insert_mode_values[] =
  {
    { FMB_SBR_INSERT_MODE_INSERT,    "FMB_SBR_INSERT_MODE_INSERT",    N_ ("Insert"),    },
    { FMB_SBR_INSERT_MODE_OVERWRITE, "FMB_SBR_INSERT_MODE_OVERWRITE", N_ ("Overwrite"), },
    { 0,                                NULL,                               NULL,             },
  };

  static const GEnumValue number_mode_values[] =
  {
    { FMB_SBR_NUMBER_MODE_123,           "FMB_SBR_NUMBER_MODE_123",          N_ ("1, 2, 3, ..."),          },
    { FMB_SBR_NUMBER_MODE_010203,        "FMB_SBR_NUMBER_MODE_010203",       N_ ("01, 02, 03, ..."),       },
    { FMB_SBR_NUMBER_MODE_001002003,     "FMB_SBR_NUMBER_MODE_001002003",    N_ ("001, 002, 003, ..."),    },
    { FMB_SBR_NUMBER_MODE_000100020003,  "FMB_SBR_NUMBER_MODE_000100020003", N_ ("0001, 0002, 0003, ..."), },
    { FMB_SBR_NUMBER_MODE_ABC,           "FMB_SBR_NUMBER_MODE_ABC",          N_ ("a, b, c, d, ..."),       },
    { 0,                                    NULL,                                  NULL,                         },
  };

  static const GEnumValue offset_mode_values[] =
  {
    { FMB_SBR_OFFSET_MODE_LEFT,  "FMB_SBR_OFFSET_MODE_LEFT",  N_ ("From the front (left)"),  },
    { FMB_SBR_OFFSET_MODE_RIGHT, "FMB_SBR_OFFSET_MODE_RIGHT", N_ ("From the back (right)"), },
    { 0,                            NULL,                           NULL,              },
  };

  static const GEnumValue text_mode_values[] =
  {
    { FMB_SBR_TEXT_MODE_OTN, "FMB_SBR_TEXT_MODE_OTN", N_ ("Old Name - Text - Number"), },
    { FMB_SBR_TEXT_MODE_NTO, "FMB_SBR_TEXT_MODE_NTO", N_ ("Number - Text - Old Name"), },
    { FMB_SBR_TEXT_MODE_TN,  "FMB_SBR_TEXT_MODE_TN",  N_ ("Text - Number"),            },
    { FMB_SBR_TEXT_MODE_NT,  "FMB_SBR_TEXT_MODE_NT",  N_ ("Number - Text"),            },
    { 0,                        NULL,                       NULL,                            },
  };

  static const GEnumValue date_mode_values[] =
  {
    { FMB_SBR_DATE_MODE_NOW,    "FMB_SBR_DATE_MODE_NOW",   N_ ("Current"),            },
    { FMB_SBR_DATE_MODE_ATIME,  "FMB_SBR_DATE_MODE_ATIME", N_ ("Date Accessed"),      },
    { FMB_SBR_DATE_MODE_MTIME,  "FMB_SBR_DATE_MODE_MTIME", N_ ("Date Modified"),      },
#ifdef HAVE_EXIF
    { FMB_SBR_DATE_MODE_TAKEN,  "FMB_SBR_DATE_MODE_TAKEN", N_ ("Date Picture Taken"), },
#endif
    { 0,                           NULL,                         NULL,                      },
  };

  case_renamer_mode_type = fmbx_provider_plugin_register_enum (plugin, "FmbSbrCaseRenamerMode", case_renamer_mode_values);
  insert_mode_type = fmbx_provider_plugin_register_enum (plugin, "FmbSbrInsertMode", insert_mode_values);
  number_mode_type = fmbx_provider_plugin_register_enum (plugin, "FmbSbrNumberMode", number_mode_values);
  offset_mode_type = fmbx_provider_plugin_register_enum (plugin, "FmbSbrOffsetMode", offset_mode_values);
  text_mode_type = fmbx_provider_plugin_register_enum (plugin, "FmbSbrTextMode", text_mode_values);
  date_mode_type = fmbx_provider_plugin_register_enum (plugin, "FmbSbrDateMode", date_mode_values);
}





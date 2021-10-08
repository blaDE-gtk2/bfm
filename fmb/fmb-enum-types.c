/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2006-2007 Benedikt Meurer <benny@xfce.org>
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

#include <blxo/blxo.h>

#include <fmb/fmb-enum-types.h>



static void fmb_icon_size_from_zoom_level (const GValue *src_value,
                                              GValue       *dst_value);



GType
fmb_renamer_mode_get_type (void)
{
  static GType type = G_TYPE_INVALID;

  if (G_UNLIKELY (type == G_TYPE_INVALID))
    {
      static const GEnumValue values[] =
      {
        { FMB_RENAMER_MODE_NAME,   "FMB_RENAMER_MODE_NAME",   N_ ("Name only"),       },
        { FMB_RENAMER_MODE_SUFFIX, "FMB_RENAMER_MODE_SUFFIX", N_ ("Suffix only"),     },
        { FMB_RENAMER_MODE_BOTH,   "FMB_RENAMER_MODE_BOTH",   N_ ("Name and Suffix"), },
        { 0,                          NULL,                         NULL,                   },
      };

      type = g_enum_register_static (I_("FmbRenamerMode"), values);
    }

  return type;
}



GType
fmb_date_style_get_type (void)
{
  static GType type = G_TYPE_INVALID;

  if (G_UNLIKELY (type == G_TYPE_INVALID))
    {
      static const GEnumValue values[] =
      {
        { FMB_DATE_STYLE_SIMPLE, "FMB_DATE_STYLE_SIMPLE", "simple", },
        { FMB_DATE_STYLE_SHORT,  "FMB_DATE_STYLE_SHORT",  "short",  },
        { FMB_DATE_STYLE_LONG,   "FMB_DATE_STYLE_LONG",   "long",   },
        { FMB_DATE_STYLE_ISO,    "FMB_DATE_STYLE_ISO",    "iso",   },
        { 0,                        NULL,                       NULL,     },
      };

      type = g_enum_register_static (I_("FmbDateStyle"), values);
    }

  return type;
}



GType
fmb_column_get_type (void)
{
  static GType type = G_TYPE_INVALID;

  if (G_UNLIKELY (type == G_TYPE_INVALID))
    {
      static const GEnumValue values[] =
      {
        { FMB_COLUMN_DATE_ACCESSED, "FMB_COLUMN_DATE_ACCESSED", N_ ("Date Accessed"), },
        { FMB_COLUMN_DATE_MODIFIED, "FMB_COLUMN_DATE_MODIFIED", N_ ("Date Modified"), },
        { FMB_COLUMN_GROUP,         "FMB_COLUMN_GROUP",         N_ ("Group"),         },
        { FMB_COLUMN_MIME_TYPE,     "FMB_COLUMN_MIME_TYPE",     N_ ("MIME Type"),     },
        { FMB_COLUMN_NAME,          "FMB_COLUMN_NAME",          N_ ("Name"),          },
        { FMB_COLUMN_OWNER,         "FMB_COLUMN_OWNER",         N_ ("Owner"),         },
        { FMB_COLUMN_PERMISSIONS,   "FMB_COLUMN_PERMISSIONS",   N_ ("Permissions"),   },
        { FMB_COLUMN_SIZE,          "FMB_COLUMN_SIZE",          N_ ("Size"),          },
        { FMB_COLUMN_TYPE,          "FMB_COLUMN_TYPE",          N_ ("Type"),          },
        { FMB_COLUMN_FILE,          "FMB_COLUMN_FILE",          N_ ("File"),          },
        { FMB_COLUMN_FILE_NAME,     "FMB_COLUMN_FILE_NAME",     N_ ("File Name"),     },
        { 0,                           NULL,                          NULL,                 },
      };

      type = g_enum_register_static (I_("FmbColumn"), values);
    }

  return type;
}



GType
fmb_icon_size_get_type (void)
{
  static GType type = G_TYPE_INVALID;

  if (G_UNLIKELY (type == G_TYPE_INVALID))
    {
      static const GEnumValue values[] =
      {
        { FMB_ICON_SIZE_SMALLEST, "FMB_ICON_SIZE_SMALLEST", "smallest", },
        { FMB_ICON_SIZE_SMALLER,  "FMB_ICON_SIZE_SMALLER",  "smaller",  },
        { FMB_ICON_SIZE_SMALL,    "FMB_ICON_SIZE_SMALL",    "small",    },
        { FMB_ICON_SIZE_NORMAL,   "FMB_ICON_SIZE_NORMAL",   "normal",   },
        { FMB_ICON_SIZE_LARGE,    "FMB_ICON_SIZE_LARGE",    "large",    },
        { FMB_ICON_SIZE_LARGER,   "FMB_ICON_SIZE_LARGER",   "larger",   },
        { FMB_ICON_SIZE_LARGEST,  "FMB_ICON_SIZE_LARGEST",  "largest",  },
        { 0,                         NULL,                        NULL,       },
      };

      type = g_enum_register_static (I_("FmbIconSize"), values);
    }

  return type;
}



GType
fmb_recursive_permissions_get_type (void)
{
  static GType type = G_TYPE_INVALID;

  if (G_UNLIKELY (type == G_TYPE_INVALID))
    {
      static const GEnumValue values[] =
      {
        { FMB_RECURSIVE_PERMISSIONS_ASK,    "FMB_RECURSIVE_PERMISSIONS_ASK",    "ask",    },
        { FMB_RECURSIVE_PERMISSIONS_ALWAYS, "FMB_RECURSIVE_PERMISSIONS_ALWAYS", "always", },
        { FMB_RECURSIVE_PERMISSIONS_NEVER,  "FMB_RECURSIVE_PERMISSIONS_NEVER",  "never",  },
        { 0,                                   NULL,                                  NULL,     },
      };

      type = g_enum_register_static (I_("FmbRecursivePermissions"), values);
    }

  return type;
}



GType
fmb_zoom_level_get_type (void)
{
  static GType type = G_TYPE_INVALID;

  if (G_UNLIKELY (type == G_TYPE_INVALID))
    {
      static const GEnumValue values[] =
      {
        { FMB_ZOOM_LEVEL_SMALLEST, "FMB_ZOOM_LEVEL_SMALLEST", "smallest", },
        { FMB_ZOOM_LEVEL_SMALLER,  "FMB_ZOOM_LEVEL_SMALLER",  "smaller",  },
        { FMB_ZOOM_LEVEL_SMALL,    "FMB_ZOOM_LEVEL_SMALL",    "small",    },
        { FMB_ZOOM_LEVEL_NORMAL,   "FMB_ZOOM_LEVEL_NORMAL",   "normal",   },
        { FMB_ZOOM_LEVEL_LARGE,    "FMB_ZOOM_LEVEL_LARGE",    "large",    },
        { FMB_ZOOM_LEVEL_LARGER,   "FMB_ZOOM_LEVEL_LARGER",   "larger",   },
        { FMB_ZOOM_LEVEL_LARGEST,  "FMB_ZOOM_LEVEL_LARGEST",  "largest",  },
        { 0,                          NULL,                         NULL,       },
      };

      type = g_enum_register_static (I_("FmbZoomLevel"), values);

      /* register transformation function for FmbZoomLevel->FmbIconSize */
      g_value_register_transform_func (type, FMB_TYPE_ICON_SIZE, fmb_icon_size_from_zoom_level);
    }

  return type;
}



GType
fmb_thumbnail_mode_get_type (void)
{
  static GType type = G_TYPE_INVALID;

  if (G_UNLIKELY (type == G_TYPE_INVALID))
    {
      static const GEnumValue values[] =
      {
        { FMB_THUMBNAIL_MODE_NEVER,      "FMB_THUMBNAIL_MODE_NEVER",      "never",      },
        { FMB_THUMBNAIL_MODE_ONLY_LOCAL, "FMB_THUMBNAIL_MODE_ONLY_LOCAL", "only-local", },
        { FMB_THUMBNAIL_MODE_ALWAYS,     "FMB_THUMBNAIL_MODE_ALWAYS",     "always",     },
        { 0,                                NULL,                               NULL,         },
      };

      type = g_enum_register_static (I_("FmbThumbnailMode"), values);
    }

  return type;
}



/**
 * fmb_zoom_level_to_icon_size:
 * @zoom_level : a #FmbZoomLevel.
 *
 * Returns the #FmbIconSize corresponding to the @zoom_level.
 *
 * Return value: the #FmbIconSize for @zoom_level.
 **/
static FmbIconSize
fmb_zoom_level_to_icon_size (FmbZoomLevel zoom_level)
{
  switch (zoom_level)
    {
    case FMB_ZOOM_LEVEL_SMALLEST: return FMB_ICON_SIZE_SMALLEST;
    case FMB_ZOOM_LEVEL_SMALLER:  return FMB_ICON_SIZE_SMALLER;
    case FMB_ZOOM_LEVEL_SMALL:    return FMB_ICON_SIZE_SMALL;
    case FMB_ZOOM_LEVEL_NORMAL:   return FMB_ICON_SIZE_NORMAL;
    case FMB_ZOOM_LEVEL_LARGE:    return FMB_ICON_SIZE_LARGE;
    case FMB_ZOOM_LEVEL_LARGER:   return FMB_ICON_SIZE_LARGER;
    default:                         return FMB_ICON_SIZE_LARGEST;
    }
}



static void
fmb_icon_size_from_zoom_level (const GValue *src_value,
                                  GValue       *dst_value)
{
  g_value_set_enum (dst_value, fmb_zoom_level_to_icon_size (g_value_get_enum (src_value)));
}



GType
fmb_job_response_get_type (void)
{
	static GType type = G_TYPE_INVALID;

  if (G_UNLIKELY (type == G_TYPE_INVALID))
    {
	    static const GFlagsValue values[] = 
      {
	      { FMB_JOB_RESPONSE_YES,     "FMB_JOB_RESPONSE_YES",     "yes"     },
	      { FMB_JOB_RESPONSE_YES_ALL, "FMB_JOB_RESPONSE_YES_ALL", "yes-all" },
	      { FMB_JOB_RESPONSE_NO,      "FMB_JOB_RESPONSE_NO",      "no"      },
	      { FMB_JOB_RESPONSE_CANCEL,  "FMB_JOB_RESPONSE_CANCEL",  "cancel"  },
	      { FMB_JOB_RESPONSE_NO_ALL,  "FMB_JOB_RESPONSE_NO_ALL",  "no-all"  },
	      { FMB_JOB_RESPONSE_RETRY,   "FMB_JOB_RESPONSE_RETRY",   "retry"   },
	      { FMB_JOB_RESPONSE_FORCE,   "FMB_JOB_RESPONSE_FORCE",   "force"   },
	      { 0,                           NULL,                          NULL      }
	    };

	    type = g_flags_register_static (I_("FmbJobResponse"), values);
    }

	return type;
}



GType
fmb_file_mode_get_type (void)
{
	static GType type = G_TYPE_INVALID;

	if (type == G_TYPE_INVALID) 
    {
	    static const GFlagsValue values[] = 
      {
	      { FMB_FILE_MODE_SUID,      "FMB_FILE_MODE_SUID",      "suid"      },
	      { FMB_FILE_MODE_SGID,      "FMB_FILE_MODE_SGID",      "sgid"      },
	      { FMB_FILE_MODE_STICKY,    "FMB_FILE_MODE_STICKY",    "sticky"    },
	      { FMB_FILE_MODE_USR_ALL,   "FMB_FILE_MODE_USR_ALL",   "usr-all"   },
	      { FMB_FILE_MODE_USR_READ,  "FMB_FILE_MODE_USR_READ",  "usr-read"  },
	      { FMB_FILE_MODE_USR_WRITE, "FMB_FILE_MODE_USR_WRITE", "usr-write" },
	      { FMB_FILE_MODE_USR_EXEC,  "FMB_FILE_MODE_USR_EXEC",  "usr-exec"  },
	      { FMB_FILE_MODE_GRP_ALL,   "FMB_FILE_MODE_GRP_ALL",   "grp-all"   },
	      { FMB_FILE_MODE_GRP_READ,  "FMB_FILE_MODE_GRP_READ",  "grp-read"  },
	      { FMB_FILE_MODE_GRP_WRITE, "FMB_FILE_MODE_GRP_WRITE", "grp-write" },
	      { FMB_FILE_MODE_GRP_EXEC,  "FMB_FILE_MODE_GRP_EXEC",  "grp-exec"  },
	      { FMB_FILE_MODE_OTH_ALL,   "FMB_FILE_MODE_OTH_ALL",   "oth-all"   },
	      { FMB_FILE_MODE_OTH_READ,  "FMB_FILE_MODE_OTH_READ",  "oth-read"  },
	      { FMB_FILE_MODE_OTH_WRITE, "FMB_FILE_MODE_OTH_WRITE", "oth-write" },
	      { FMB_FILE_MODE_OTH_EXEC,  "FMB_FILE_MODE_OTH_EXEC",  "oth-exec"  },
	      { 0,                          NULL,                         NULL        }
	    };
	    
      type = g_flags_register_static ("FmbFileMode", values);
    }
	return type;
}

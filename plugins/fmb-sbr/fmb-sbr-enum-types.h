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

#ifndef __FMB_SBR_ENUM_TYPES_H__
#define __FMB_SBR_ENUM_TYPES_H__

#include <fmbx/fmbx.h>

G_BEGIN_DECLS;

#define FMB_SBR_TYPE_CASE_RENAMER_MODE (fmb_sbr_case_renamer_mode_get_type ())

/**
 * FmbSbrCaseRenamerMode:
 * @FMB_SBR_CASE_RENAMER_MODE_LOWER    : convert to lower case.
 * @FMB_SBR_CASE_RENAMER_MODE_UPPER    : convert to upper case.
 * @FMB_SBR_CASE_RENAMER_MODE_CAMEL    : convert to camel case.
 * @FMB_SBR_CASE_RENAMER_MODE_SENTENCE : convert to sentence case.
 *
 * The operation mode for the #FmbSbrCaseRenamer.
 **/
typedef enum
{
  FMB_SBR_CASE_RENAMER_MODE_LOWER,
  FMB_SBR_CASE_RENAMER_MODE_UPPER,
  FMB_SBR_CASE_RENAMER_MODE_CAMEL,
  FMB_SBR_CASE_RENAMER_MODE_SENTENCE,
} FmbSbrCaseRenamerMode;

GType fmb_sbr_case_renamer_mode_get_type (void) G_GNUC_CONST;


#define FMB_SBR_TYPE_INSERT_MODE (fmb_sbr_insert_mode_get_type ())

/**
 * FmbSbrInsertMode:
 * @FMB_SBR_INSERT_MODE_INSERT    : insert characters.
 * @FMB_SBR_INSERT_MODE_OVERWRITE : overwrite existing characters.
 *
 * The operation mode for the #FmbSbrInsertRenamer.
 **/
typedef enum
{
  FMB_SBR_INSERT_MODE_INSERT,
  FMB_SBR_INSERT_MODE_OVERWRITE,
} FmbSbrInsertMode;

GType fmb_sbr_insert_mode_get_type (void) G_GNUC_CONST;


#define FMB_SBR_TYPE_NUMBER_MODE (fmb_sbr_number_mode_get_type ())

/**
 * FmbSbrNumberMode:
 * @FMB_SBR_NUMBER_MODE_123          : 1, 2, 3, ...
 * @FMB_SBR_NUMBER_MODE_010203       : 01, 02, 03, ...
 * @FMB_SBR_NUMBER_MODE_001002003    : 001, 002, 003, ...
 * @FMB_SBR_NUMBER_MODE_000100020003 : 0001, 0002, 0003, ...
 * @FMB_SBR_NUMBER_MODE_ABC          : a, b, c, ...
 *
 * The numbering mode for the #FmbSbrNumberRenamer.
 **/
typedef enum
{
  FMB_SBR_NUMBER_MODE_123,
  FMB_SBR_NUMBER_MODE_010203,
  FMB_SBR_NUMBER_MODE_001002003,
  FMB_SBR_NUMBER_MODE_000100020003,
  FMB_SBR_NUMBER_MODE_ABC,
} FmbSbrNumberMode;

GType fmb_sbr_number_mode_get_type (void) G_GNUC_CONST;


#define FMB_SBR_TYPE_OFFSET_MODE (fmb_sbr_offset_mode_get_type ())

/**
 * FmbSbrOffsetMode:
 * @FMB_SBR_OFFSET_MODE_LEFT  : offset starting from the left.
 * @FMB_SBR_OFFSET_MODE_RIGHT : offset starting from the right.
 *
 * The offset mode for the #FmbSbrInsertRenamer and the
 * #FmbSbrRemoveRenamer.
 **/
typedef enum
{
  FMB_SBR_OFFSET_MODE_LEFT,
  FMB_SBR_OFFSET_MODE_RIGHT,
} FmbSbrOffsetMode;

GType fmb_sbr_offset_mode_get_type (void) G_GNUC_CONST;


#define FMB_SBR_TYPE_TEXT_MODE (fmb_sbr_text_mode_get_type ())

/**
 * FmbSbrTextMode:
 * @FMB_SBR_TEXT_MODE_OTN : Old Name - Text - Number
 * @FMB_SBR_TEXT_MODE_NTO : Number - Text - Old Name
 * @FMB_SBR_TEXT_MODE_TN  : Text - Number
 * @FMB_SBR_TEXT_MODE_NT  : Number - Text
 *
 * The text mode for the #FmbSbrNumberRenamer.
 **/
typedef enum
{
  FMB_SBR_TEXT_MODE_OTN,
  FMB_SBR_TEXT_MODE_NTO,
  FMB_SBR_TEXT_MODE_TN,
  FMB_SBR_TEXT_MODE_NT,
} FmbSbrTextMode;

GType fmb_sbr_text_mode_get_type (void) G_GNUC_CONST;


#define FMB_SBR_TYPE_DATE_MODE (fmb_sbr_date_mode_get_type ())

/**
 * FmbSbrDateMode:
 * @FMB_SBR_DATE_MODE_NOW   : Current Time
 * @FMB_SBR_DATE_MODE_ATIME : Access Time
 * @FMB_SBR_DATE_MODE_MTIME : Modification Time
 * @FMB_SBR_DATE_MODE_TAKEN : Picture Taken Time
 *
 * The date mode for the #FmbSbrDateRenamer.
 **/
typedef enum
{
  FMB_SBR_DATE_MODE_NOW,
  FMB_SBR_DATE_MODE_ATIME,
  FMB_SBR_DATE_MODE_MTIME,
#ifdef HAVE_EXIF
  FMB_SBR_DATE_MODE_TAKEN,
#endif
} FmbSbrDateMode;

GType fmb_sbr_date_mode_get_type (void) G_GNUC_CONST;


void fmb_sbr_register_enum_types (FmbxProviderPlugin *plugin);

G_END_DECLS;

#endif /* !__FMB_SBR_ENUM_TYPES_H__ */

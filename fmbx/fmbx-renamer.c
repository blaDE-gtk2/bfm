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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib/gi18n-lib.h>

#include <fmbx/fmbx-renamer.h>
#include <fmbx/fmbx-private.h>



#define FMBX_RENAMER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), FMBX_TYPE_RENAMER, FmbxRenamerPrivate))



/* Property identifiers */
enum
{
  PROP_0,
  PROP_HELP_URL,
  PROP_NAME,
};

/* Signal identifiers */
enum
{
  CHANGED,
  LAST_SIGNAL,
};



static void     fmbx_renamer_finalize          (GObject                *object);
static GObject *fmbx_renamer_constructor       (GType                   type,
                                                   guint                   n_construct_properties,
                                                   GObjectConstructParam  *construct_properties);
static void     fmbx_renamer_get_property      (GObject                *object,
                                                   guint                   prop_id,
                                                   GValue                 *value,
                                                   GParamSpec             *pspec);
static void     fmbx_renamer_set_property      (GObject                *object,
                                                   guint                   prop_id,
                                                   const GValue           *value,
                                                   GParamSpec             *pspec);
static gchar   *fmbx_renamer_real_process      (FmbxRenamer         *renamer,
                                                   FmbxFileInfo        *file,
                                                   const gchar            *text,
                                                   guint                   num);
static void     fmbx_renamer_real_load         (FmbxRenamer         *renamer,
                                                   GHashTable             *settings);
static void     fmbx_renamer_real_save         (FmbxRenamer         *renamer,
                                                   GHashTable             *settings);
static GList   *fmbx_renamer_real_get_actions  (FmbxRenamer         *renamer,
                                                   GtkWindow              *window,
                                                   GList                  *files);



struct _FmbxRenamerPrivate
{
  gchar *help_url;
  gchar *name;
};



static guint renamer_signals[LAST_SIGNAL];



G_DEFINE_ABSTRACT_TYPE (FmbxRenamer, fmbx_renamer, GTK_TYPE_VBOX)



static void
fmbx_renamer_class_init (FmbxRenamerClass *klass)
{
  GObjectClass *gobject_class;

  /* add private data */
  g_type_class_add_private (klass, sizeof (FmbxRenamerPrivate));

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = fmbx_renamer_finalize;
  gobject_class->constructor = fmbx_renamer_constructor;
  gobject_class->get_property = fmbx_renamer_get_property;
  gobject_class->set_property = fmbx_renamer_set_property;

  klass->process = fmbx_renamer_real_process;
  klass->load = fmbx_renamer_real_load;
  klass->save = fmbx_renamer_real_save;
  klass->get_actions = fmbx_renamer_real_get_actions;

  /**
   * FmbxRenamer:help-url:
   *
   * The URL to the documentation of this #FmbxRenamer.
   * Derived classes can set this property to point to the
   * documentation for the specific renamer. The documentation
   * of the specific renamer in turn should contain a link to
   * the general Fmb renamer documentation.
   *
   * May also be unset, in which case the general Fmb renamer
   * documentation will be shown when the user clicks the "Help"
   * button.
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_HELP_URL,
                                   g_param_spec_string ("help-url",
                                                        _("Help URL"),
                                                        _("The URL to the documentation of the renamer"),
                                                        NULL,
                                                        G_PARAM_READWRITE));

  /**
   * FmbxRenamer:name:
   *
   * The user visible name of the renamer, that is displayed
   * in the bulk rename dialog of the file manager. Derived
   * classes should set a useful name.
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_NAME,
                                   g_param_spec_string ("name",
                                                        _("Name"),
                                                        _("The user visible name of the renamer"),
                                                        NULL,
                                                        G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE));

  /**
   * FmbxRenamer::changed:
   * @renamer : a #FmbxRenamer.
   *
   * Derived classes should emit this signal using the
   * fmbx_renamer_changed() method whenever the user
   * changed a setting in the @renamer GUI.
   *
   * The file manager will then invoke fmbx_renamer_process()
   * for all files that should be renamed and update the preview.
   **/
  renamer_signals[CHANGED] =
    g_signal_new (I_("changed"),
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (FmbxRenamerClass, changed),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);
}



static void
fmbx_renamer_init (FmbxRenamer *renamer)
{
  /* grab a pointer on the private data */
  renamer->priv = FMBX_RENAMER_GET_PRIVATE (renamer);

  /* initialize the GtkVBox to sane defaults */
  gtk_container_set_border_width (GTK_CONTAINER (renamer), 12);
  gtk_box_set_homogeneous (GTK_BOX (renamer), FALSE);
  gtk_box_set_spacing (GTK_BOX (renamer), 6);
}



static void
fmbx_renamer_finalize (GObject *object)
{
  FmbxRenamer *renamer = FMBX_RENAMER (object);

  /* release private attributes */
  g_free (renamer->priv->help_url);
  g_free (renamer->priv->name);

  (*G_OBJECT_CLASS (fmbx_renamer_parent_class)->finalize) (object);
}



static GObject*
fmbx_renamer_constructor (GType                  type,
                             guint                  n_construct_properties,
                             GObjectConstructParam *construct_properties)
{
  GObject *object;

  /* let the parent class constructor create the instance */
  object = (*G_OBJECT_CLASS (fmbx_renamer_parent_class)->constructor) (type, n_construct_properties, construct_properties);

  /* check if a name was set, otherwise use the type name */
  if (G_UNLIKELY (FMBX_RENAMER (object)->priv->name == NULL))
    fmbx_renamer_set_name (FMBX_RENAMER (object), g_type_name (type));

  return object;
}



static void
fmbx_renamer_get_property (GObject    *object,
                              guint       prop_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
  FmbxRenamer *renamer = FMBX_RENAMER (object);

  switch (prop_id)
    {
    case PROP_HELP_URL:
      g_value_set_string (value, fmbx_renamer_get_help_url (renamer));
      break;

    case PROP_NAME:
      g_value_set_string (value, fmbx_renamer_get_name (renamer));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static void
fmbx_renamer_set_property (GObject      *object,
                              guint         prop_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
  FmbxRenamer *renamer = FMBX_RENAMER (object);

  switch (prop_id)
    {
    case PROP_HELP_URL:
      fmbx_renamer_set_help_url (renamer, g_value_get_string (value));
      break;

    case PROP_NAME:
      fmbx_renamer_set_name (renamer, g_value_get_string (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static gchar*
fmbx_renamer_real_process (FmbxRenamer  *renamer,
                              FmbxFileInfo *file,
                              const gchar     *text,
                              guint            num)
{
  /* the fallback method simply returns a copy of text */
  return g_strdup (text);
}



static void
fmbx_renamer_real_load (FmbxRenamer *renamer,
                           GHashTable     *settings)
{
  const gchar *setting;
  GParamSpec **specs;
  GValue       value = { 0, };
  GValue       tmp = { 0, };
  gchar       *key;
  guint        n_specs;
  guint        n;

  /* determine the parameters provided this class (and superclasses) */
  specs = g_object_class_list_properties (G_OBJECT_GET_CLASS (renamer), &n_specs);
  for (n = 0; n < n_specs; ++n)
    {
      /* check if this parameter is introduced by FmbxRenamer or a superclass of it, and ignore such parameters. */
      if (!g_type_is_a (specs[n]->owner_type, FMBX_TYPE_RENAMER) || specs[n]->owner_type == FMBX_TYPE_RENAMER)
        continue;

      /* verify that the type can be transformed from strings */
      if (!g_value_type_transformable (G_TYPE_STRING, specs[n]->value_type))
        continue;

      /* determine the key for the parameter */
      key = fmbx_param_spec_get_option_name (specs[n]);
      
      /* check if we have a value for that key */
      setting = g_hash_table_lookup (settings, key);
      if (G_LIKELY (setting != NULL))
        {
          /* initialize the value as the value type */
          g_value_init (&value, specs[n]->value_type);

          /* transform the setting to the value type */
          g_value_init (&tmp, G_TYPE_STRING);
          g_value_set_static_string (&tmp, setting);
          g_value_transform (&tmp, &value);
          g_value_unset (&tmp);

          /* apply the value to the property */
          g_object_set_property (G_OBJECT (renamer), specs[n]->name, &value);

          /* cleanup */
          g_value_unset (&value);
        }

      /* cleanup */
      g_free (key);
    }
  g_free (specs);
}



static void
fmbx_renamer_real_save (FmbxRenamer *renamer,
                           GHashTable     *settings)
{
  GParamSpec **specs;
  GValue       value = { 0, };
  GValue       tmp = { 0, };
  guint        n_specs;
  guint        n;

  /* determine the parameters provided this class (and superclasses) */
  specs = g_object_class_list_properties (G_OBJECT_GET_CLASS (renamer), &n_specs);
  for (n = 0; n < n_specs; ++n)
    {
      /* check if this parameter is introduced by FmbxRenamer or a superclass of it, and ignore such parameters. */
      if (!g_type_is_a (specs[n]->owner_type, FMBX_TYPE_RENAMER) || specs[n]->owner_type == FMBX_TYPE_RENAMER)
        continue;

      /* verify that the type can be transformed to strings */
      if (!g_value_type_transformable (specs[n]->value_type, G_TYPE_STRING))
        continue;

      /* initialize the value as string */
      g_value_init (&value, G_TYPE_STRING);

      /* transform the property value to a string */
      g_value_init (&tmp, specs[n]->value_type);
      g_object_get_property (G_OBJECT (renamer), specs[n]->name, &tmp);
      g_value_transform (&tmp, &value);
      g_value_unset (&tmp);

      /* save the value to the settings (replacing any existing settings with that name) */
      g_hash_table_replace (settings, fmbx_param_spec_get_option_name (specs[n]), g_value_dup_string (&value));

      /* cleanup */
      g_value_unset (&value);
    }
  g_free (specs);
}



static GList*
fmbx_renamer_real_get_actions (FmbxRenamer *renamer,
                                  GtkWindow      *window,
                                  GList          *files)
{
  /* return an empty list, derived classes may override this method */
  return NULL;
}



/**
 * fmbx_renamer_get_help_url:
 * @renamer : a #FmbxRenamer.
 *
 * Returns the URL of the documentation for @renamer
 * or %NULL if no specific documentation is available
 * for @renamer and the general documentation of the
 * Fmb renamers should be displayed instead.
 *
 * Return value: the URL of the documentation for @renamer.
 **/
const gchar*
fmbx_renamer_get_help_url (FmbxRenamer *renamer)
{
  g_return_val_if_fail (FMBX_IS_RENAMER (renamer), NULL);
  return renamer->priv->help_url;
}



/**
 * fmbx_renamer_set_help_url:
 * @renamer  : a #FmbxRenamer.
 * @help_url : the new URL to the documentation of @renamer.
 *
 * The URL to the documentation of this #FmbxRenamer.
 * Derived classes can set this property to point to the
 * documentation for the specific renamer. The documentation
 * of the specific renamer in turn should contain a link to
 * the general Fmb renamer documentation.
 *
 * May also be unset, in which case the general Fmb renamer
 * documentation will be shown when the user clicks the "Help"
 * button.
 **/
void
fmbx_renamer_set_help_url (FmbxRenamer *renamer,
                              const gchar    *help_url)
{
  g_return_if_fail (FMBX_IS_RENAMER (renamer));

  /* release the previous value */
  g_free (renamer->priv->help_url);

  /* apply the new value */
  renamer->priv->help_url = g_strdup (help_url);

  /* notify listeners */
  g_object_notify (G_OBJECT (renamer), "help-url");
}



/**
 * fmbx_renamer_get_name:
 * @renamer : a #FmbxRenamer.
 *
 * Returns the user visible name for @renamer, previously
 * set with fmbx_renamer_set_name().
 *
 * Return value: the user visible name for @renamer.
 **/
const gchar*
fmbx_renamer_get_name (FmbxRenamer *renamer)
{
  g_return_val_if_fail (FMBX_IS_RENAMER (renamer), NULL);
  return renamer->priv->name;
}



/**
 * fmbx_renamer_set_name:
 * @renamer : a #FmbxRenamer.
 * @name    : the new user visible name for @renamer.
 *
 * Sets the user visible name for @renamer to @name. This method should
 * only be called by derived classes and prior to returning the @renamer
 * is returned from fmbx_renamer_provider_get_renamers().
 **/
void
fmbx_renamer_set_name (FmbxRenamer *renamer,
                          const gchar    *name)
{
  g_return_if_fail (FMBX_IS_RENAMER (renamer));

  /* release the previous name */
  g_free (renamer->priv->name);

  /* setup the new name */
  renamer->priv->name = g_strdup (name);

  /* notify listeners */
  g_object_notify (G_OBJECT (renamer), "name");
}



/**
 * fmbx_renamer_process:
 * @renamer : a #FmbxRenamer.
 * @file    : the #FmbxFileInfo for the file whose new
 *            name - according to @renamer - should be
 *            determined.
 * @text    : the part of the filename to which the
 *            @renamer should be applied.
 * @idx     : the index of the file in the list, used
 *            for renamers that work on numbering.
 *
 * Determines the replacement for @text (which is the relevant
 * part of the full @file name, i.e. either the suffix, the name
 * or the name and the suffix).
 *
 * The caller is responsible to free the returned string using
 * g_free() when no longer needed.
 *
 * Return value: the string with which to replace @text.
 **/
gchar*
fmbx_renamer_process (FmbxRenamer  *renamer,
                         FmbxFileInfo *file,
                         const gchar     *text,
                         guint            idx)
{
  g_return_val_if_fail (FMBX_IS_FILE_INFO (file), NULL);
  g_return_val_if_fail (FMBX_IS_RENAMER (renamer), NULL);
  g_return_val_if_fail (g_utf8_validate (text, -1, NULL), NULL);
  return (*FMBX_RENAMER_GET_CLASS (renamer)->process) (renamer, file, text, idx);
}



/**
 * fmbx_renamer_load:
 * @renamer  : a #FmbxRenamer.
 * @settings : a #GHashTable which contains the previously saved
 *             settings for @renamer as key/value pairs of strings.
 *
 * Tells @renamer to load its internal settings from the specified
 * @settings. The @settings hash table contains previously saved
 * settings, see fmbx_renamer_save(), as key/value pairs of
 * strings. That is, both the keys and the values are strings.
 *
 * Implementations of #FmbxRenamer may decide to override this
 * method to perform custom loading of settings. If you do not
 * override this method, the default method of #FmbxRenamer
 * will be used, which simply loads all #GObject properties
 * provided by @renamer<!---->s class (excluding the ones
 * provided by the parent classes) from the @settings. The
 * #GObject properties must be transformable to strings and
 * from strings.
 *
 * If you decide to override this method for your #FmbxRenamer
 * implementation, you should also override fmbx_renamer_save().
 **/
void
fmbx_renamer_load (FmbxRenamer *renamer,
                      GHashTable     *settings)
{
  g_return_if_fail (FMBX_IS_RENAMER (renamer));
  g_return_if_fail (settings != NULL);
  (*FMBX_RENAMER_GET_CLASS (renamer)->load) (renamer, settings);
}



/**
 * fmbx_renamer_save:
 * @renamer  : a #FmbxRenamer.
 * @settings : a #GHashTable to which the current settings of @renamer
 *             should be stored as key/value pairs of strings.
 *
 * Tells @renamer to save its internal settings to the specified
 * @settings, which can afterwards be loaded by fmbx_renamer_load().
 *
 * The strings saved to @settings must be allocated by g_strdup(),
 * both the keys and the values. For example to store the string
 * <literal>Bar</literal> for the setting <literal>Foo</literal>,
 * you'd use:
 * <informalexample><programlisting>
 * g_hash_table_replace (settings, g_strdup ("Foo"), g_strdup ("Bar"));
 * </programlisting></informalexample>
 *
 * Implementations of #FmbxRenamer may decide to override this
 * method to perform custom saving of settings. If you do not overrride
 * this method, the default method of #FmbxRenamer will be used,
 * which simply stores all #GObject properties provided by the
 * @renamer<!---->s class (excluding the ones provided by the parent
 * classes) to the @settings. The #GObject properties must be transformable
 * to strings.
 *
 * If you decide to override this method for your #FmbxRenamer
 * implementation, you should also override fmbx_renamer_load().
 **/
void
fmbx_renamer_save (FmbxRenamer *renamer,
                      GHashTable     *settings)
{
  g_return_if_fail (FMBX_IS_RENAMER (renamer));
  g_return_if_fail (settings != NULL);
  (*FMBX_RENAMER_GET_CLASS (renamer)->save) (renamer, settings);
}



/**
 * fmbx_renamer_get_actions:
 * @renamer : a #FmbxRenamer.
 * @window  : a #GtkWindow or %NULL.
 * @files   : a #GList of #FmbxFileInfo<!---->s.
 *
 * Returns the list of #GtkAction<!---->s provided by @renamer for
 * the given list of @files. By default, this method returns %NULL
 * (the empty list), but derived classes may override this method
 * to provide additional actions for files in the bulk renamer
 * dialog list.
 * 
 * The returned #GtkAction<!---->s will be displayed in the file's
 * context menu of the bulk renamer dialog, when this @renamer is
 * active. For example, an ID3-Tag based renamer may add an action
 * "Edit Tags" to the context menus of supported media files and,
 * when activated, display a dialog (which should be transient and
 * modal for @window, if not %NULL), which allows the users to edit
 * media file tags on-the-fly.
 *
 * Derived classes that override this method should always check
 * first if all the #FmbxFileInfo<!---->s in the list of @files
 * are supported, and only return actions that can be performed on
 * this specific list of @files. For example, the ID3-Tag renamer
 * mentioned above, should first check whether all items in @files
 * are actually audio files. The fmbx_file_info_has_mime_type()
 * of the #FmbxFileInfo interface can be used to easily test
 * whether a file in the @files list is of a certain MIME type.
 *
 * Some actions may only work properly if only a single file ist
 * selected (for example, the ID3-Tag renamer will probably only
 * supporting editing one file at a time). In this case you have
 * basicly two options: Either you can return %NULL here if @files
 * does not contain exactly one item, or you can return the actions
 * as usual, but make them insensitive, using:
 * <informalexample><programlisting>
 * gtk_action_set_sensitive (action, FALSE);
 * </programlisting></informalexample>
 * The latter has the advantage that the user will still notice the
 * existance of the action and probably realize that it can only be
 * applied to a single item at once.
 *
 * The caller is responsible to free the returned list using something
 * like the following:
 * <informalexample><programlisting>
 * g_list_free_full (list, g_object_unref);
 * </programlisting></informalexample>
 *
 * As a special note, this method automatically takes a reference on the
 * @renamer for every #GtkAction object returned from the real implementation
 * of this method in @renamer. This is to make sure that the extension stays
 * in memory for atleast the time that the actions are used.
 *
 * The #GtkAction<!---->s returned from this method must be namespaced with
 * the module to avoid collision with internal file manager actions and
 * actions provided by other extensions. For example, the menu action
 * provided by the ID3-Tag renamer mentioned above, should be named
 * <literal>TagRenamer::edit-tags</literal> (if <literal>TagRenamer</literal>
 * is the class name). For additional information about the way #GtkAction<!---->s
 * should be returned from extensions and the way they are used, read the
 * description of the #FmbxMenuProvider interface or read the introduction
 * provided with this reference manual.
 *
 * A note of warning concerning the @window parameter. Plugins should
 * avoid taking a reference on @window, as that might introduce a
 * circular reference and can thereby cause a quite large memory leak.
 * Instead, if @window is not %NULL, add a weak reference using the
 * g_object_weak_ref() or g_object_add_weak_pointer() method. But don't
 * forget to release the weak reference if @window survived the lifetime
 * of your action (which is likely to be the case in most situations).
 *
 * Return value: the list of #GtkAction<!---->s provided by @renamer
 *               for the given list of @files.
 **/
GList*
fmbx_renamer_get_actions (FmbxRenamer *renamer,
                             GtkWindow      *window,
                             GList          *files)
{
  GList *actions;

  g_return_val_if_fail (FMBX_IS_RENAMER (renamer), NULL);
  g_return_val_if_fail (window == NULL || GTK_IS_WINDOW (window), NULL);

  /* query the actions from the implementation */
  actions = (*FMBX_RENAMER_GET_CLASS (renamer)->get_actions) (renamer, window, files);

  /* take a reference on the renamer for each action */
  fmbx_object_list_take_reference (actions, renamer);

  return actions;
}



/**
 * fmbx_renamer_changed:
 * @renamer : a #FmbxRenamer.
 *
 * This method should be used by derived classes
 * to emit the "changed" signal for @renamer. See
 * the documentation of the "changed" signal for
 * details.
 **/
void
fmbx_renamer_changed (FmbxRenamer *renamer)
{
  g_return_if_fail (FMBX_IS_RENAMER (renamer));
  g_signal_emit (G_OBJECT (renamer), renamer_signals[CHANGED], 0);
}

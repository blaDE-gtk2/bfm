# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:

AM_CPPFLAGS =								\
	-DGLIB_DISABLE_DEPRECATION_WARNINGS \
	-I$(top_builddir)						\
	-I$(top_srcdir)							\
	-DBINDIR=\"$(bindir)\"						\
	-DDATADIR=\"$(datadir)\"					\
	-DBLXO_DISABLE_DEPRECATED					\
	-DG_LOG_DOMAIN=\"fmb\"					\
	-DLIBDIR=\"$(libdir)\"						\
	-DHELPERDIR=\"$(HELPER_PATH_PREFIX)\"				\
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\"				\
	-DFMB_VERSION_API=\"$(FMB_VERSION_API)\"			\
	-DSN_API_NOT_YET_FROZEN						\
	-DG_UDEV_API_IS_SUBJECT_TO_CHANGE				\
	$(PLATFORM_CPPFLAGS)

bin_SCRIPTS =								\
	fmb-settings

bin_PROGRAMS =								\
	fmb

fmb_built_sources =							\
	fmb-marshal.c						\
	fmb-marshal.h						\
	fmb-abstract-icon-view-ui.h					\
	fmb-details-view-ui.h					\
	fmb-launcher-ui.h						\
	fmb-location-buttons-ui.h					\
	fmb-shortcuts-pane-ui.h					\
	fmb-renamer-dialog-ui.h					\
	fmb-standard-view-ui.h					\
	fmb-thumbnail-frame.c					\
	fmb-window-ui.h


fmb_SOURCES =							\
	$(fmb_include_HEADERS)					\
	$(fmb_built_sources)						\
	$(fmb_dbus_sources)						\
	main.c								\
	fmb-abstract-dialog.c					\
	fmb-abstract-dialog.h					\
	fmb-abstract-icon-view.c					\
	fmb-abstract-icon-view.h					\
	fmb-application.c						\
	fmb-application.h						\
	fmb-browser.c						\
	fmb-browser.h						\
	fmb-chooser-button.c						\
	fmb-chooser-button.h						\
	fmb-chooser-dialog.c						\
	fmb-chooser-dialog.h						\
	fmb-chooser-model.c						\
	fmb-chooser-model.h						\
	fmb-clipboard-manager.c					\
	fmb-clipboard-manager.h					\
	fmb-column-editor.c						\
	fmb-column-editor.h						\
	fmb-column-model.c						\
	fmb-column-model.h						\
	fmb-compact-view.c						\
	fmb-compact-view.h						\
	fmb-component.c						\
	fmb-component.h						\
	fmb-create-dialog.c						\
	fmb-create-dialog.h						\
	fmb-deep-count-job.h						\
	fmb-deep-count-job.c						\
	fmb-details-view.c						\
	fmb-details-view.h						\
	fmb-dialogs.c						\
	fmb-dialogs.h						\
	fmb-device.c							\
	fmb-device.h							\
	fmb-device-monitor.c						\
	fmb-device-monitor.h						\
	fmb-dnd.c							\
	fmb-dnd.h							\
	fmb-emblem-chooser.c						\
	fmb-emblem-chooser.h						\
	fmb-enum-types.c						\
	fmb-enum-types.h						\
	fmb-exec.c							\
	fmb-exec.h							\
	fmb-file.c							\
	fmb-file.h							\
	fmb-file-monitor.c						\
	fmb-file-monitor.h						\
	fmb-folder.c							\
	fmb-folder.h							\
	fmb-gdk-extensions.c						\
	fmb-gdk-extensions.h						\
	fmb-gio-extensions.c						\
	fmb-gio-extensions.h						\
	fmb-gobject-extensions.c					\
	fmb-gobject-extensions.h					\
	fmb-gtk-extensions.c						\
	fmb-gtk-extensions.h						\
	fmb-history-action.c						\
	fmb-history-action.h						\
	fmb-history.c						\
	fmb-history.h						\
	fmb-ice.c							\
	fmb-ice.h							\
	fmb-icon-factory.c						\
	fmb-icon-factory.h						\
	fmb-icon-renderer.c						\
	fmb-icon-renderer.h						\
	fmb-icon-view.c						\
	fmb-icon-view.h						\
	fmb-image.c							\
	fmb-image.h							\
	fmb-io-jobs.c						\
	fmb-io-jobs.h						\
	fmb-io-jobs-util.c						\
	fmb-io-jobs-util.h						\
	fmb-io-scan-directory.c					\
	fmb-io-scan-directory.h					\
	fmb-job.c							\
	fmb-job.h							\
	fmb-launcher.c						\
	fmb-launcher.h						\
	fmb-list-model.c						\
	fmb-list-model.h						\
	fmb-location-bar.c						\
	fmb-location-bar.h						\
	fmb-location-button.c					\
	fmb-location-button.h					\
	fmb-location-buttons.c					\
	fmb-location-buttons.h					\
	fmb-location-dialog.c					\
	fmb-location-dialog.h					\
	fmb-location-entry.c						\
	fmb-location-entry.h						\
	fmb-misc-jobs.c						\
	fmb-misc-jobs.h						\
	fmb-notify.c							\
	fmb-notify.h							\
	fmb-navigator.c						\
	fmb-navigator.h						\
	fmb-pango-extensions.c					\
	fmb-pango-extensions.h					\
	fmb-path-entry.c						\
	fmb-path-entry.h						\
	fmb-permissions-chooser.c					\
	fmb-permissions-chooser.h					\
	fmb-preferences-dialog.c					\
	fmb-preferences-dialog.h					\
	fmb-preferences.c						\
	fmb-preferences.h						\
	fmb-private.h						\
	fmb-progress-dialog.c					\
	fmb-progress-dialog.h					\
	fmb-progress-view.c						\
	fmb-progress-view.h						\
	fmb-properties-dialog.c					\
	fmb-properties-dialog.h					\
	fmb-renamer-dialog.c						\
	fmb-renamer-dialog.h						\
	fmb-renamer-model.c						\
	fmb-renamer-model.h						\
	fmb-renamer-pair.c						\
	fmb-renamer-pair.h						\
	fmb-renamer-progress.c					\
	fmb-renamer-progress.h					\
	fmb-sendto-model.c						\
	fmb-sendto-model.h						\
	fmb-session-client.c						\
	fmb-session-client.h						\
	fmb-shortcuts-icon-renderer.c				\
	fmb-shortcuts-icon-renderer.h				\
	fmb-shortcuts-model.c					\
	fmb-shortcuts-model.h					\
	fmb-shortcuts-pane.c						\
	fmb-shortcuts-pane.h						\
	fmb-shortcuts-view.c						\
	fmb-shortcuts-view.h						\
	fmb-side-pane.c						\
	fmb-side-pane.h						\
	fmb-simple-job.c						\
	fmb-simple-job.h						\
	fmb-size-label.c						\
	fmb-size-label.h						\
	fmb-standard-view.c						\
	fmb-standard-view.h						\
	fmb-statusbar.c						\
	fmb-statusbar.h						\
	fmb-stock.c							\
	fmb-stock.h							\
	fmb-templates-action.c					\
	fmb-templates-action.h					\
	fmb-text-renderer.c						\
	fmb-text-renderer.h						\
	fmb-thumbnail-cache.c					\
	fmb-thumbnail-cache.h					\
	fmb-thumbnailer.c						\
	fmb-thumbnailer.h						\
	fmb-thumbnail-frame.h					\
	fmb-transfer-job.c						\
	fmb-transfer-job.h						\
	fmb-trash-action.c						\
	fmb-trash-action.h						\
	fmb-tree-model.c						\
	fmb-tree-model.h						\
	fmb-tree-pane.c						\
	fmb-tree-pane.h						\
	fmb-tree-view.c						\
	fmb-tree-view.h						\
	fmb-user.c							\
	fmb-user.h							\
	fmb-util.c							\
	fmb-util.h							\
	fmb-view.c							\
	fmb-view.h							\
	fmb-window.c							\
	fmb-window.h

fmb_CFLAGS =								\
	$(BLXO_CFLAGS)							\
	$(GIO_CFLAGS)							\
	$(GTHREAD_CFLAGS)						\
	$(GUDEV_CFLAGS)							\
	$(LIBNOTIFY_CFLAGS)						\
	$(LIBSM_CFLAGS)							\
	$(LIBBLADEUI_CFLAGS)						\
	$(BLCONF_CFLAGS)						\
	$(PLATFORM_CFLAGS)

fmb_LDFLAGS =							\
	-no-undefined							\
	$(LIBSM_LDFLAGS)						\
	$(PLATFORM_LDFLAGS)

fmb_LDADD =								\
	$(top_builddir)/fmbx/libfmbx-$(FMBX_VERSION_API).la	\
	$(BLXO_LIBS)							\
	$(GIO_LIBS)							\
	$(GTHREAD_LIBS)							\
	$(GUDEV_LIBS)							\
	$(LIBNOTIFY_LIBS)						\
	$(LIBSM_LIBS)							\
	$(LIBBLADEUI_LIBS)						\
	$(BLCONF_LIBS)

fmb_DEPENDENCIES =							\
	$(top_builddir)/fmbx/libfmbx-$(FMBX_VERSION_API).la

if HAVE_DBUS
fmb_built_sources +=							\
	fmb-dbus-service-infos.h					\
	fmb-thumbnail-cache-proxy.h					\
	fmb-thumbnailer-proxy.h

fmb_dbus_sources =							\
	fmb-dbus-client.c						\
	fmb-dbus-client.h						\
	fmb-dbus-service.c						\
	fmb-dbus-service.h						\
	fmb-thumbnail-cache-proxy.h					\
	fmb-thumbnailer-proxy.h

fmb_CFLAGS +=							\
	-DDBUS_API_SUBJECT_TO_CHANGE					\
	$(DBUS_CFLAGS)

fmb_LDADD +=								\
	$(DBUS_LIBS)
endif

if HAVE_GIO_UNIX
fmb_CFLAGS +=							\
	$(GIO_UNIX_CFLAGS)

fmb_LDADD +=								\
	$(GIO_UNIX_LIBS)
endif

desktopdir = $(datadir)/applications
desktop_in_files = fmb-settings.desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

# install symlink to 'fmb'
install-exec-hook:
	$(mkinstalldirs) $(DESTDIR)$(bindir)
	-( cd $(DESTDIR)$(bindir) ; test -x Fmb || ln -sf fmb Fmb )

uninstall-local:
	-( cd $(DESTDIR)$(bindir) ; test -h Fmb && rm -f Fmb )

clean-local:
	rm -f *.core core core.* *.o

DISTCLEANFILES =							\
	fmb-settings.desktop

if MAINTAINER_MODE
CLEANFILES =								\
	xgen-tmc							\
	xgen-tmh

DISTCLEANFILES +=							\
	$(fmb_built_sources)						\
	stamp-fmb-marshal.h

BUILT_SOURCES =								\
	$(fmb_built_sources)

if HAVE_DBUS
fmb-dbus-service-infos.h: $(srcdir)/fmb-dbus-service-infos.xml Makefile
	$(AM_V_GEN) dbus-binding-tool --prefix=fmb_dbus_service --mode=glib-server $(srcdir)/fmb-dbus-service-infos.xml > fmb-dbus-service-infos.h

fmb-thumbnailer-proxy.h: $(srcdir)/fmb-thumbnailer-dbus.xml Makefile
	$(AM_V_GEN) ( \
		dbus-binding-tool --mode=glib-client \
			$(srcdir)/fmb-thumbnailer-dbus.xml > fmb-thumbnailer-proxy.h.tmp \
		&& $(SED) -e 's/org_freedesktop_thumbnails_Thumbnailer1/fmb_thumbnailer_proxy/g' \
			fmb-thumbnailer-proxy.h.tmp > fmb-thumbnailer-proxy.h \
		&& $(RM) fmb-thumbnailer-proxy.h.tmp \
	)

fmb-thumbnail-cache-proxy.h: $(srcdir)/fmb-thumbnail-cache-dbus.xml Makefile
	$(AM_V_GEN) ( \
		dbus-binding-tool --mode=glib-client \
			$(srcdir)/fmb-thumbnail-cache-dbus.xml > fmb-thumbnail-cache-proxy.h.tmp \
		&& $(SED) -e 's/org_freedesktop_thumbnails_Cache1/fmb_thumbnail_cache_proxy/g' \
			fmb-thumbnail-cache-proxy.h.tmp > fmb-thumbnail-cache-proxy.h \
		&& $(RM) fmb-thumbnail-cache-proxy.h.tmp \
	)
endif

fmb-thumbnail-frame.c: $(srcdir)/fmb-thumbnail-frame.png Makefile
	$(AM_V_GEN) (echo "#include <fmb/fmb-thumbnail-frame.h>" && gdk-pixbuf-csource --extern --raw --stream --name=fmb_thumbnail_frame $(srcdir)/fmb-thumbnail-frame.png) > fmb-thumbnail-frame.c

fmb-abstract-icon-view-ui.h: Makefile $(srcdir)/fmb-abstract-icon-view-ui.xml
	$(AM_V_GEN) blxo-csource --strip-comments --strip-content --static --name=fmb_abstract_icon_view_ui $(srcdir)/fmb-abstract-icon-view-ui.xml > fmb-abstract-icon-view-ui.h

fmb-details-view-ui.h: Makefile $(srcdir)/fmb-details-view-ui.xml
	$(AM_V_GEN) blxo-csource --strip-comments --strip-content --static --name=fmb_details_view_ui $(srcdir)/fmb-details-view-ui.xml > fmb-details-view-ui.h

fmb-launcher-ui.h: Makefile $(srcdir)/fmb-launcher-ui.xml
	$(AM_V_GEN) blxo-csource --strip-comments --strip-content --static --name=fmb_launcher_ui $(srcdir)/fmb-launcher-ui.xml > fmb-launcher-ui.h

fmb-location-buttons-ui.h: Makefile $(srcdir)/fmb-location-buttons-ui.xml
	$(AM_V_GEN) blxo-csource --strip-comments --strip-content --static --name=fmb_location_buttons_ui $(srcdir)/fmb-location-buttons-ui.xml > fmb-location-buttons-ui.h

fmb-shortcuts-pane-ui.h: Makefile $(srcdir)/fmb-shortcuts-pane-ui.xml
	$(AM_V_GEN) blxo-csource --strip-comments --strip-content --static --name=fmb_shortcuts_pane_ui $(srcdir)/fmb-shortcuts-pane-ui.xml > fmb-shortcuts-pane-ui.h

fmb-renamer-dialog-ui.h: Makefile $(srcdir)/fmb-renamer-dialog-ui.xml
	$(AM_V_GEN) blxo-csource --strip-comments --strip-content --static --name=fmb_renamer_dialog_ui $(srcdir)/fmb-renamer-dialog-ui.xml > fmb-renamer-dialog-ui.h

fmb-standard-view-ui.h: Makefile $(srcdir)/fmb-standard-view-ui.xml
	$(AM_V_GEN) blxo-csource --strip-comments --strip-content --static --name=fmb_standard_view_ui $(srcdir)/fmb-standard-view-ui.xml > fmb-standard-view-ui.h

fmb-window-ui.h: Makefile $(srcdir)/fmb-window-ui.xml
	$(AM_V_GEN) blxo-csource --strip-comments --strip-content --static --name=fmb_window_ui $(srcdir)/fmb-window-ui.xml > fmb-window-ui.h

fmb-marshal.h: stamp-fmb-marshal.h
	@true

stamp-fmb-marshal.h: fmb-marshal.list Makefile
	$(AM_V_GEN) ( \
		cd $(srcdir) \
		&& echo "#ifndef __FMB_MARSHAL_H__" > xgen-tmh \
		&& echo "#define __FMB_MARSHAL_H__" >> xgen-tmh \
		&& ( glib-genmarshal \
			--prefix=_fmb_marshal \
			--header fmb-marshal.list ) >> xgen-tmh \
		&& echo "#endif /* !__FMB_MARSHAL_H__ */" >> xgen-tmh \
		&& ( cmp -s xgen-tmh fmb-marshal.h || cp xgen-tmh fmb-marshal.h ) \
		&& rm -f xgen-tmh \
		&& echo timestamp > $(@F) \
	)

fmb-marshal.c: fmb-marshal.list Makefile
	$(AM_V_GEN) ( \
		cd $(srcdir) \
		&& echo "#include <fmb/fmb-marshal.h>" > xgen-tmc \
		&& ( glib-genmarshal \
			--prefix=_fmb_marshal \
			--body fmb-marshal.list ) >> xgen-tmc \
		&& cp xgen-tmc fmb-marshal.c \
		&& rm -f xgen-tmc \
	)
endif

EXTRA_DIST =								\
	fmb-abstract-icon-view-ui.xml				\
	fmb-dbus-service-infos.xml					\
	fmb-details-view-ui.xml					\
	fmb-launcher-ui.xml						\
	fmb-location-buttons-ui.xml					\
	fmb-marshal.list						\
	fmb-renamer-dialog-ui.xml					\
	fmb-standard-view-ui.xml					\
	fmb-thumbnail-frame.png					\
	fmb-window-ui.xml						\
	fmb-settings							\
	$(desktop_in_files)

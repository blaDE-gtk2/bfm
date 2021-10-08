AM_CPPFLAGS =								\
	-DGLIB_DISABLE_DEPRECATION_WARNINGS \
	-I$(top_builddir)						\
	-I$(top_srcdir)							\
	-DG_LOG_DOMAIN=\"fmb-wallpaper-plugin\"			\
	-DBLXO_API_SUBJECT_TO_CHANGE					\
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\"				\
	$(PLATFORM_CPPFLAGS)

extensionsdir = $(libdir)/fmbx-$(FMBX_VERSION_API)
extensions_LTLIBRARIES =						\
	fmb-wallpaper-plugin.la

fmb_wallpaper_plugin_la_SOURCES =					\
	twp-provider.h							\
	twp-provider.c							\
	fmb-wallpaper-plugin.c

fmb_wallpaper_plugin_la_CFLAGS =					\
	$(BLXO_CFLAGS)							\
	$(GLIB_CFLAGS)							\
	$(GTK_CFLAGS)							\
	$(LIBX11_CFLAGS)						\
	$(BLCONF_CFLAGS)						\
	$(PLATFORM_CFLAGS)

fmb_wallpaper_plugin_la_LDFLAGS =					\
	-avoid-version							\
	-export-dynamic							\
	-no-undefined							\
	-export-symbols-regex "^fmb_extension_(shutdown|initialize|list_types)" \
	-module								\
	$(PLATFORM_LDFLAGS)

fmb_wallpaper_plugin_la_LIBADD =					\
	$(top_builddir)/fmbx/libfmbx-$(FMBX_VERSION_API).la	\
	$(BLXO_LIBS)							\
	$(GLIB_LIBS)							\
	$(GTK_LIBS)							\
	$(BLCONF_LIBS)						\
	$(LIBX11_LIBS)

fmb_wallpaper_plugin_la_DEPENDENCIES =				\
	$(top_builddir)/fmbx/libfmbx-$(FMBX_VERSION_API).la

# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:

AM_CPPFLAGS =								\
	-DGLIB_DISABLE_DEPRECATION_WARNINGS \
	-I$(top_builddir)						\
	-I$(top_builddir)/plugins					\
	-I$(top_srcdir)							\
	-I$(top_srcdir)/plugins						\
	-DG_LOG_DOMAIN=\"fmb-apr\"					\
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\"				\
	$(PLATFORM_CPPFLAGS)

extensionsdir = $(libdir)/fmbx-$(FMBX_VERSION_API)
extensions_LTLIBRARIES =						\
	fmb-apr.la

fmb_apr_la_SOURCES =							\
	fmb-apr-abstract-page.c					\
	fmb-apr-abstract-page.h					\
	fmb-apr-desktop-page.c					\
	fmb-apr-desktop-page.h					\
	fmb-apr-image-page.c						\
	fmb-apr-image-page.h						\
	fmb-apr-plugin.c						\
	fmb-apr-private.c						\
	fmb-apr-private.h						\
	fmb-apr-provider.c						\
	fmb-apr-provider.h

fmb_apr_la_CFLAGS =							\
	$(EXIF_CFLAGS)							\
	$(BLXO_CFLAGS)							\
	$(GLIB_CFLAGS)							\
	$(GTK_CFLAGS)							\
	$(PLATFORM_CFLAGS)

fmb_apr_la_LDFLAGS =							\
	-avoid-version							\
	-export-dynamic							\
	-no-undefined							\
	-export-symbols-regex "^fmb_extension_(shutdown|initialize|list_types)" \
	-module								\
	$(PLATFORM_LDFLAGS)

fmb_apr_la_LIBADD =							\
	$(top_builddir)/fmbx/libfmbx-$(FMBX_VERSION_API).la	\
	$(EXIF_LIBS)							\
	$(BLXO_LIBS)							\
	$(GLIB_LIBS)							\
	$(GTK_LIBS)

fmb_apr_la_DEPENDENCIES =						\
	$(top_builddir)/fmbx/libfmbx-$(FMBX_VERSION_API).la


EXTRA_DIST =								\
	README

# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:

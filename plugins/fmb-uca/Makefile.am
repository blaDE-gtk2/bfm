AM_CPPFLAGS =								\
	-DGLIB_DISABLE_DEPRECATION_WARNINGS \
	-I$(top_builddir)						\
	-I$(top_builddir)/plugins					\
	-I$(top_srcdir)							\
	-I$(top_srcdir)/plugins						\
	-DBINDIR=\"$(bindir)\"						\
	-DDATADIR=\"$(datadir)\"					\
	-DG_LOG_DOMAIN=\"fmb-uca\"					\
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\"				\
	$(PLATFORM_CPPFLAGS)

extensionsdir = $(libdir)/fmbx-$(FMBX_VERSION_API)
extensions_LTLIBRARIES =						\
	fmb-uca.la

fmb_uca_la_SOURCES =							\
	fmb-uca-chooser.c						\
	fmb-uca-chooser.h						\
	fmb-uca-context.c						\
	fmb-uca-context.h						\
	fmb-uca-editor.c						\
	fmb-uca-editor.h						\
	fmb-uca-model.c						\
	fmb-uca-model.h						\
	fmb-uca-plugin.c						\
	fmb-uca-private.c						\
	fmb-uca-private.h						\
	fmb-uca-provider.c						\
	fmb-uca-provider.h

fmb_uca_la_CFLAGS =							\
	$(BLXO_CFLAGS)							\
	$(LIBBLADEUTIL_CFLAGS)						\
	$(LIBBLADEUI_CFLAGS)						\
	$(PLATFORM_CFLAGS)

fmb_uca_la_LDFLAGS =							\
	-avoid-version							\
	-export-dynamic							\
	-no-undefined							\
	-export-symbols-regex "^fmb_extension_(shutdown|initialize|list_types)" \
	-module								\
	$(PLATFORM_LDFLAGS)

fmb_uca_la_LIBADD =							\
	$(top_builddir)/fmbx/libfmbx-$(FMBX_VERSION_API).la	\
	$(BLXO_LIBS)							\
	$(LIBBLADEUTIL_LIBS)						\
	$(LIBBLADEUI_LIBS)

fmb_uca_la_DEPENDENCIES =						\
	$(top_builddir)/fmbx/libfmbx-$(FMBX_VERSION_API).la

defaultsdir = $(sysconfdir)/xdg/Fmb
defaults_in_files = uca.xml.in
defaults_DATA = $(defaults_in_files:.xml.in=.xml)
@INTLTOOL_XML_RULE@

EXTRA_DIST =								\
	README								\
	$(defaults_in_files)

CLEANFILES =								\
	$(defaults_DATA)

# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:

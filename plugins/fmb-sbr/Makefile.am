AM_CPPFLAGS =								\
	-DGLIB_DISABLE_DEPRECATION_WARNINGS \
	-I$(top_builddir)						\
	-I$(top_builddir)/plugins					\
	-I$(top_srcdir)							\
	-I$(top_srcdir)/plugins						\
	-DG_LOG_DOMAIN=\"fmb-sbr\"					\
	$(PLATFORM_CPPFLAGS)

extensionsdir = $(libdir)/fmbx-$(FMBX_VERSION_API)
extensions_LTLIBRARIES =						\
	fmb-sbr.la

fmb_sbr_la_SOURCES =							\
	fmb-sbr-case-renamer.c					\
	fmb-sbr-case-renamer.h					\
	fmb-sbr-date-renamer.c					\
	fmb-sbr-date-renamer.h					\
	fmb-sbr-enum-types.c						\
	fmb-sbr-enum-types.h						\
	fmb-sbr-insert-renamer.c					\
	fmb-sbr-insert-renamer.h					\
	fmb-sbr-number-renamer.c					\
	fmb-sbr-number-renamer.h					\
	fmb-sbr-plugin.c						\
	fmb-sbr-provider.c						\
	fmb-sbr-provider.h						\
	fmb-sbr-remove-renamer.c					\
	fmb-sbr-remove-renamer.h					\
	fmb-sbr-replace-renamer.c					\
	fmb-sbr-replace-renamer.h

fmb_sbr_la_CFLAGS =							\
	$(EXIF_CFLAGS)							\
	$(BLXO_CFLAGS)							\
	$(GLIB_CFLAGS)							\
	$(PCRE_CFLAGS)							\
	$(PLATFORM_CFLAGS)

fmb_sbr_la_LDFLAGS =							\
	-avoid-version							\
	-export-dynamic							\
	-no-undefined							\
	-export-symbols-regex "^fmb_extension_(shutdown|initialize|list_types)" \
	-module								\
	$(PLATFORM_LDFLAGS)

fmb_sbr_la_LIBADD =							\
	$(top_builddir)/fmbx/libfmbx-$(FMBX_VERSION_API).la	\
	$(EXIF_LIBS)							\
	$(BLXO_LIBS)							\
	$(GLIB_LIBS)							\
	$(PCRE_LIBS)

fmb_sbr_la_DEPENDENCIES =						\
	$(top_builddir)/fmbx/libfmbx-$(FMBX_VERSION_API).la

EXTRA_DIST =								\
	README

# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:

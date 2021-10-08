AM_CPPFLAGS =								\
	-DGLIB_DISABLE_DEPRECATION_WARNINGS \
	-I$(top_builddir)						\
	-I$(top_builddir)/plugins					\
	-I$(top_srcdir)							\
	-I$(top_srcdir)/plugins						\
	-DG_LOG_DOMAIN=\"libfmb-tpa\"				\
	$(PLATFORM_CPPFLAGS)

plugindir = $(libdir)/xfce4/bar/plugins
plugin_LTLIBRARIES =							\
	libfmb-tpa.la

libfmb_tpa_la_SOURCES =						\
	fmb-tpa.c							\
	fmb-tpa-bindings.h

libfmb_tpa_la_CFLAGS =						\
	$(DBUS_CFLAGS)							\
	$(LIBBLADEUI_CFLAGS)						\
	$(LIBBLADEUTIL_CFLAGS)						\
	$(LIBBLADEBAR_CFLAGS)						\
	$(PLATFORM_CFLAGS)

libfmb_tpa_la_LIBADD =						\
	$(DBUS_LIBS)							\
	$(LIBBLADEUTIL_LIBS)						\
	$(LIBBLADEUI_LIBS)						\
	$(LIBBLADEBAR_LIBS)

libfmb_tpa_la_LDFLAGS =						\
	-avoid-version							\
	-module								\
	-no-undefined							\
	-export-symbols-regex '^blade_bar_module_(preinit|init|construct)' \
	$(PLATFORM_LDFLAGS)

desktopdir = $(datadir)/xfce4/bar/plugins
desktop_in_files = fmb-tpa.desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

if MAINTAINER_MODE
BUILT_SOURCES =								\
	fmb-tpa-bindings.h

DISTCLEANFILES =							\
	fmb-tpa-bindings.h

fmb-tpa-bindings.h: $(srcdir)/fmb-tpa-bindings.xml Makefile
	$(AM_V_GEN) dbus-binding-tool --prefix=fmb_tpa_dbus --mode=glib-client $(srcdir)/fmb-tpa-bindings.xml > fmb-tpa-bindings.h
endif

EXTRA_DIST =								\
	$(desktop_in_files)						\
	fmb-tpa-bindings.xml						\
	README

CLEANFILES =								\
	$(desktop_DATA)

# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:

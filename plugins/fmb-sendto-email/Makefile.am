AM_CPPFLAGS =								\
	-DGLIB_DISABLE_DEPRECATION_WARNINGS \
	-I$(top_builddir)						\
	-I$(top_builddir)/plugins					\
	-I$(top_srcdir)							\
	-I$(top_srcdir)/plugins						\
	-DG_LOG_DOMAIN=\"fmb-sendto-email\"				\
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\"				\
	$(PLATFORM_CPPFLAGS)

fmb_sendto_emaildir =						\
	$(HELPER_PATH_PREFIX)/Fmb

fmb_sendto_email_PROGRAMS =						\
	fmb-sendto-email

fmb_sendto_email_SOURCES =						\
	main.c

fmb_sendto_email_CFLAGS =						\
	$(BLXO_CFLAGS)							\
	$(PLATFORM_CFLAGS)

fmb_sendto_email_LDADD =						\
	$(BLXO_LIBS)							\
	$(PLATFORM_LDFLAGS)

desktopdir = $(datadir)/Fmb/sendto
desktop_in_in_files = fmb-sendto-email.desktop.in.in
desktop_in_files = $(desktop_in_in_files:.desktop.in.in=.desktop.in)
%.desktop.in: %.desktop.in.in
	sed -e "s,\@HELPERDIR\@,$(HELPER_PATH_PREFIX),g" < $< > $@
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

EXTRA_DIST =								\
	$(desktop_in_in_files)						\
	README

DISTCLEANFILES =							\
	$(desktop_in_files)						\
	$(desktop_DATA)

# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:

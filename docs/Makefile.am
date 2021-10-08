SUBDIRS =								\
	design								\
	papers

doc_DATA =								\
	README.gtkrc

man_MANS =								\
	Fmb.1

if MAINTAINER_MODE
%.1: %.xml
	xsltproc -nonet http://docbook.sourceforge.net/release/xsl/current/manpages/docbook.xsl $<
endif

EXTRA_DIST =								\
	$(doc_DATA)							\
	$(man_MANS)							\
	Fmb.xml

# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:

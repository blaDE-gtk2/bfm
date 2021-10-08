dnl Copyright (c) 2004-2006
dnl         The Fmb development team. All rights reserved.
dnl
dnl Written for Fmb by Benedikt Meurer <benny@xfce.org>.
dnl



dnl # BM_FMB_PLUGIN_APR()
dnl #
dnl # Check whether the "Advanced Properties" plugin
dnl # should be built and installed.
dnl #
AC_DEFUN([BM_FMB_PLUGIN_APR],
[
AC_ARG_ENABLE([apr-plugin], [AC_HELP_STRING([--disable-apr-plugin], [Don't build the fmb-apr plugin, see plugins/fmb-apr/README])],
  [ac_bm_fmb_plugin_apr=$enableval], [ac_bm_fmb_plugin_apr=yes])
AC_MSG_CHECKING([whether to build the fmb-apr plugin])
AM_CONDITIONAL([FMB_PLUGIN_APR], [test x"$ac_bm_fmb_plugin_apr" = x"yes"])
AC_MSG_RESULT([$ac_bm_fmb_plugin_apr])

dnl Check for libexif (for the "Image" properties page)
XDT_CHECK_OPTIONAL_PACKAGE([EXIF], [libexif], [0.6.0], [exif], [Exif support])
])



dnl # BM_FMB_PLUGIN_SBR()
dnl #
dnl # Check whether the "Simple Builtin Renamers" plugin
dnl # should be built and installed.
dnl #
AC_DEFUN([BM_FMB_PLUGIN_SBR],
[
AC_ARG_ENABLE([sbr-plugin], AC_HELP_STRING([--disable-sbr-plugin], [Don't build the fmb-sbr plugin, see plugins/fmb-sbr/README]),
  [ac_bm_fmb_plugin_sbr=$enableval], [ac_bm_fmb_plugin_sbr=yes])
AC_MSG_CHECKING([whether to build the fmb-sbr plugin])
AM_CONDITIONAL([FMB_PLUGIN_SBR], [test x"$ac_bm_fmb_plugin_sbr" = x"yes"])
AC_MSG_RESULT([$ac_bm_fmb_plugin_sbr])

dnl Check for PCRE (for the "Search & Replace" renamer)
XDT_CHECK_OPTIONAL_PACKAGE([PCRE], [libpcre], [6.0], [pcre], [Regular expression support])
])



dnl # BM_FMB_PLUGIN_TPA()
dnl #
dnl # Check whether the "Trash Bar Applet" plugin should
dnl # be built and installed (this is actually a plugin
dnl # for the Xfce bar, not for Fmb).
dnl #
AC_DEFUN([BM_FMB_PLUGIN_TPA],
[
AC_ARG_ENABLE([tpa-plugin], AC_HELP_STRING([--disable-tpa-plugin], [Don't build the fmb-tpa plugin, see plugins/fmb-tpa/README]),
  [ac_bm_fmb_plugin_tpa=$enableval], [ac_bm_fmb_plugin_tpa=yes])
if test x"$ac_bm_fmb_plugin_tpa" = x"yes"; then
  XDT_CHECK_PACKAGE([LIBBLADEBAR], [libbladebar-1.0], [4.9.0],
  [
    dnl # Can only build fmb-tpa if D-BUS was found previously
    ac_bm_fmb_plugin_tpa=$DBUS_FOUND
  ],
  [
    dnl # Cannot build fmb-tpa if blade-bar is not installed
    ac_bm_fmb_plugin_tpa=no
  ])
else
  ac_bm_fmb_plugin_tpa=no
fi
AC_MSG_CHECKING([whether to build the fmb-tpa plugin])
AM_CONDITIONAL([FMB_PLUGIN_TPA], [test x"$ac_bm_fmb_plugin_tpa" = x"yes"])
AC_MSG_RESULT([$ac_bm_fmb_plugin_tpa])
])



dnl # BM_FMB_PLUGIN_UCA()
dnl #
dnl # Check whether the "User Customizable Actions" plugin
dnl # should be built and installed.
dnl #
AC_DEFUN([BM_FMB_PLUGIN_UCA],
[
AC_ARG_ENABLE([uca-plugin], AC_HELP_STRING([--disable-uca-plugin], [Don't build the fmb-uca plugin, see plugins/fmb-uca/README]),
  [ac_bm_fmb_plugin_uca=$enableval], [ac_bm_fmb_plugin_uca=yes])
AC_MSG_CHECKING([whether to build the fmb-uca plugin])
AM_CONDITIONAL([FMB_PLUGIN_UCA], [test x"$ac_bm_fmb_plugin_uca" = x"yes"])
AC_MSG_RESULT([$ac_bm_fmb_plugin_uca])
])

dnl # BM_FMB_PLUGIN_WALLPAPER()
dnl #
dnl # Check whether the "Wallpaper" plugin
dnl # should be built and installed.
dnl #
AC_DEFUN([BM_FMB_PLUGIN_WALLPAPER],
[
AC_ARG_ENABLE([wallpaper-plugin], AC_HELP_STRING([--disable-wallpaper-plugin], [Don't build the fmb-wallpaper plugin, see plugins/fmb-wallpaper/README]),
  [ac_bm_fmb_plugin_wallpaper=$enableval], [ac_bm_fmb_plugin_wallpaper=yes])
AC_MSG_CHECKING([whether to build the fmb-wallpaper plugin])
AM_CONDITIONAL([FMB_PLUGIN_WALLPAPER], [test x"$ac_bm_fmb_plugin_wallpaper" = x"yes"])
AC_MSG_RESULT([$ac_bm_fmb_plugin_wallpaper])
if test x"$ac_bm_fmb_plugin_wallpaper" = x"yes"; then
	AC_CHECK_PROG([blconf_query_found], [blconf-query], [yes], [no])
	if test x"$blconf_query_found" = x"no"; then
		echo "***"
		echo "*** blconf-query was not found on your system."
		echo "*** The wallpaper won't work without it installed."
		echo "***"
	fi
fi
])


AC_INIT([main.c])


AC_PROG_CC 
AC_LANG_C 
AC_PROG_MAKE_SET 
AC_PROG_INSTALL 
AC_HEADER_STDC 
AC_SYS_LARGEFILE

STATIC_LIBS="libUseful/libUseful.a"
AC_CONFIG_SUBDIRS([libUseful])

AC_CHECK_FILE([/usr/X11/include/X11/X.h], [X11ROOT="/usr/X11"],
              AC_CHECK_FILE([/usr/X11R7/include/X11/X.h], [X11ROOT="/usr/X11R7"],
              AC_CHECK_FILE([/usr/X11R6/include/X11/X.h], [X11ROOT="/usr/X11R6"])
              )
              )

if test "X11ROOT" != ""
then
  X11INC="-I$X11ROOT/include"
  X11LIB="-L$X11ROOT/lib"
fi

AC_ARG_WITH(x-includes, [  --with-x-includes       path to X11 includes directory], X11INC="-I$withval" )
AC_ARG_WITH(x-libraries, [  --with-x-libraries     path to X11 libraries directory], X11LIB="-L$withval" )

LDFLAGS="$LDFLAGS $X11LIB"
CFLAGS="$CFLAGS $X11INC"

AC_CHECK_LIB(X11,XOpenDisplay,,)
cf_have_x11=$ac_cv_lib_X11_XOpenDisplay
if test "$cf_have_x11" = "yes"
then
AC_DEFINE([HAVE_X11])
else
echo "ERROR: cannot find libX11"
exit
fi


dnl read Makefile.in and write Makefile 
AC_OUTPUT(Makefile)


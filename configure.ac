#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(epichord, 0.0.0, evanrinehart@gmail.com)
AM_INIT_AUTOMAKE(epichord, 0.0.0)
AC_CONFIG_SRCDIR([src/ui.h])
AC_CONFIG_HEADER([config.h])



# Checks for programs.
AC_PROG_CXX
#AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_HEADER_STDC
#AC_CHECK_HEADERS([arpa/inet.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
#AC_FUNC_MALLOC
#AC_CHECK_FUNCS([floor sqrt])

AC_CHECK_HEADER([jack/jack.h],[],[
  AC_MSG_ERROR([
    Cannot find JACK header file.
    Please install JACK.
  ])
],[])
AC_CHECK_LIB([jack],[jack_midi_event_write],[],[
  AC_MSG_ERROR([
     JACK midi support missing.
     Upgrade to a recent version of JACK.
  ])
],[])

AC_CHECK_PROG(fltk2found,fltk2-config,[yes],[no])
if test "$fltk2found" == "no"; then
  AC_MSG_ERROR([
    FLTK 2 not found. 
    Make sure FLTK 2 is installed correctly.
  ])
fi

PKG_CHECK_MODULES(LASH, lash-1.0 >= 0.5.0, HAVE_LASH="yes", HAVE_LASH="no");
if test "$HAVE_LASH" = "yes"
then
  AC_DEFINE([HAVE_LASH], 1, [Defined if LASH is present.])
fi
AM_CONDITIONAL(HAVE_LASH, test "$HAVE_LASH" = "yes")

AC_OUTPUT(Makefile src/Makefile doc/Makefile gfx/Makefile)

AC_MSG_RESULT([])
AC_MSG_RESULT([========================])
AC_MSG_RESULT([Prefix                    : $prefix])
AC_MSG_RESULT([LASH support              : $HAVE_LASH])

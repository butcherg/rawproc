#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([rawproc], [0.6Dev], [glenn.butcher@gmail.com])
#AC_CONFIG_SRCDIR([PicProcessorCurve.h])
AC_CONFIG_HEADERS([config.h])

m4_include(wxwin.m4)

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_MAKE_SET

# Checks for libraries.

AM_OPTIONS_WXCONFIG
reqwx=3.1.0
AM_PATH_WXCONFIG($reqwx, wxWin=1)
if test "$wxWin" != 1; then
	AC_MSG_ERROR([
		wxWidgets must be installed on your system.
 
		Please check that wx-config is in path, the directory
		where wxWidgets libraries are installed (returned by
		'wx-config --libs' or 'wx-config --static --libs' command)
		is in LD_LIBRARY_PATH or equivalent variable and
		wxWidgets version is $reqwx or above.
		])
fi
 
CPPFLAGS="$CPPFLAGS $WX_CPPFLAGS"
CXXFLAGS="$CXXFLAGS $WX_CXXFLAGS_ONLY"
CFLAGS="$CFLAGS $WX_CFLAGS_ONLY"
LIBS="$LIBS $WX_LIBS"

# FIXME: Replace `main' with a function in `-lgimage':
AC_CHECK_LIB([gimage], [Version])
# FIXME: Replace `main' with a function in `-ljpeg':
AC_CHECK_LIB([jpeg], [jpeg_create_compress])
# FIXME: Replace `main' with a function in `-llcms2':
AC_CHECK_LIB([lcms2], [cmsGetEncodedCMMversion])
# FIXME: Replace `main' with a function in `-lraw':
AC_CHECK_LIB([raw], [LibRaw::version])
# FIXME: Replace `main' with a function in `-ltiff':
AC_CHECK_LIB([tiff], [TIFFGetVersion])

# Checks for header files.
AC_CHECK_HEADERS([inttypes.h stdint.h stdlib.h string.h sys/time.h unistd.h wchar.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_CHECK_FUNCS([clock_gettime floor pow sqrt])

#AC_CONFIG_FILES([Makefile])
AC_OUTPUT(Makefile)

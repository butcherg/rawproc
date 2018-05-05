#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([rawproc], [0.7], [glenn.butcher@gmail.com])
#AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_CONFIG_SRCDIR([src/tiffimage.cpp])
#AC_CONFIG_HEADERS([config.h])
m4_include(wxwin.m4)

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC


#Process enables:
AC_ARG_ENABLE([openmp],
        AS_HELP_STRING([--enable-openmp], [enables OpenMP threading (-fopenmp in compile and link)])
)
if test "$enable_openmp" == "yes"
then
	CXXFLAGS="$CXXFLAGS -fopenmp"
        LDFLAGS="$LDFLAGS -fopenmp"
fi

AC_ARG_ENABLE([debug],
        AS_HELP_STRING([--enable-debug], [enables debug compilation with -g, default is no debug switch])
)
if test "$enable_debug" == "yes"
then
	CXXFLAGS="$CXXFLAGS -g"
fi


AC_ARG_ENABLE([optimization],
        AS_HELP_STRING([--enable-optimization], [enables -O4 optimization, default is -O2])
)
if test "$enable_optimization" == "yes"
then
	CXXFLAGS="$CXXFLAGS -O3"
else
	CXXFLAGS="$CXXFLAGS -O2"
fi


# Checks for libraries.

#libjpeg:
PKG_CHECK_MODULES([JPEG],[libjpeg], [
	CPPFLAGS="$JPEG_CFLAGS $CPPFLAGS"
	LIBS="$JPEG_LIBS $LIBS"
],[
	AC_MSG_ERROR([not found.])
])

#AC_CHECK_LIB([tiff], [TIFFSetErrorHandler])
#libtiff:
PKG_CHECK_MODULES([TIFF],[libtiff-4], [
	CPPFLAGS="$TIFF_CFLAGS $CPPFLAGS"
	LIBS="$TIFF_LIBS $LIBS"
],[
	AC_MSG_ERROR([not found.])
])

#libpng:
PKG_CHECK_MODULES([PNG],[libpng], [
	CPPFLAGS="$PNG_CFLAGS $CPPFLAGS"
	LIBS="$PNG_LIBS $LIBS"
],[
	AC_MSG_ERROR([not found.])
])

#littlecms2:
PKG_CHECK_MODULES([LCMS2],[lcms2], [
	CPPFLAGS="$LCMS2_CFLAGS $CPPFLAGS"
	LIBS="$LCMS2_LIBS $LIBS"
],[
	AC_MSG_ERROR([not found.])
])


#libraw-dcraw:
AC_ARG_ENABLE([dcraw],
        AS_HELP_STRING([--enable-dcraw], [enables use of a dcraw executable for 
raw file input, in lieu of LibRaw. dcraw mus be in a directory listed in PATH, o
r an absolute path specified with input.raw.dcraw.programpath])
)
if test "$enable_dcraw" == "yes"
then
	CXXFLAGS="$CXXFLAGS -DUSE_DCRAW"
else
	PKG_CHECK_MODULES([LIBRAW],[libraw], [
		CPPFLAGS="$LIBRAW_CFLAGS $CPPFLAGS"
		LIBS="$LIBRAW_LIBS $LIBS"
	],[
		AC_MSG_ERROR([not found.])
	])
fi

#lensfun:
AC_ARG_ENABLE([lensfun],
        AS_HELP_STRING([--enable-lensfun], [enables lensfun.])
)
if test "$enable_lensfun" == "yes"
then
	PKG_CHECK_MODULES([LENSFUN],[lensfun], [
		CPPFLAGS="$LENSFUN_CFLAGS $CPPFLAGS"
		LIBS="$LENSFUN_LIBS $LIBS"
	],[
		AC_MSG_ERROR([not found.])
	])
	CXXFLAGS="$CXXFLAGS -DUSE_LENSFUN"
	
fi
AM_CONDITIONAL(LENSFUN, test x$enable_lensfun = xyes)


# Check for wxWidgets.
#AM_OPTIONS_WXCONFIG
WX_CONFIG_OPTIONS
reqwx=3.1.0
#AM_PATH_WXCONFIG($reqwx, wxWin=1)
WX_CONFIG_CHECK([$reqwx], [wxWin=1], [wxWin=0], [std,aui,propgrid], [])
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


# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h locale.h stdint.h stdlib.h string.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([clock_gettime floor modf pow rint setlocale sqrt strtol])

AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT

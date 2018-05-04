#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([rawproc], [0.7], [glenn.butcher@gmail.com])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_CONFIG_SRCDIR([src/tiffimage.cpp])
AC_CONFIG_HEADERS([config.h])
m4_include(wxwin.m4)

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lgimage':   (or, replace this with the missing png check...)
AC_CHECK_LIB([gimage], [main])
AC_CHECK_LIB([m], [pow])
AC_CHECK_LIB([jpeg], [jpeg_set_defaults])
AC_CHECK_LIB([tiff], [TIFFSetErrorHandler])
AC_CHECK_LIB([lcms2], [cmsSetLogErrorHandler])


AC_ARG_ENABLE([dcraw],
        AS_HELP_STRING([--enable-dcraw], [enables use of a dcraw executable for 
raw file input, in lieu of LibRaw. dcraw mus be in a directory listed in PATH, o
r an absolute path specified with input.raw.dcraw.programpath])
)
if test "$enable_dcraw" == "yes"
then
	CXXFLAGS="$CXXFLAGS -DUSE_DCRAW"
else
	AC_CHECK_LIB([raw], [libraw_init])
fi





AC_ARG_ENABLE([lensfun],
        AS_HELP_STRING([--enable-lensfun], [enables lensfun])
)
if test "$enable_lensfun" == "yes"
then
	#do the lensfun package check...
	CXXFLAGS="$CXXFLAGS -DUSE_LENSFUN"
	RAWPROC_SOURCEFILES="$RAWPROC_SOURCEFILES PicProcessorLensCorrection.cpp"
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
AC_SUBST(RAWPROC_SOURCEFILES)
AC_OUTPUT

#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([rawproc], [1.0.1dev], [glenn.butcher@gmail.com])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_CONFIG_SRCDIR([src/tiffimage.cpp])
#AC_CONFIG_HEADERS([config.h])
m4_include(wxwin.m4)
m4_include(ax_cxx_compile_stdcxx.m4)


# Checks for programs.
AC_PROG_CXX
AC_OPENMP
#AX_CXX_COMPILE_STDCXX(11, noext, mandatory)

# MINGW_AC_WIN32_NATIVE_HOST
# --------------------------
# Check if the runtime platform is a native Win32 host.
#
AC_DEFUN([MINGW_AC_WIN32_NATIVE_HOST],
[AC_CACHE_CHECK([whether we are building for a Win32 host], 
                [mingw_cv_win32_host],
AC_COMPILE_IFELSE([AC_LANG_SOURCE([
#ifdef _WIN32
choke me
#endif
])
], [mingw_cv_win32_host=no], [mingw_cv_win32_host=yes]))])

#run the win32 check macro:
MINGW_AC_WIN32_NATIVE_HOST



#Process enables:

AC_ARG_WITH([pixtype],
	AS_HELP_STRING([--with-pixtype], [=half|float|double, specifies data type for pixel structure, default is float])
)
if test "$with_pixtype" != ""
then
	CXXFLAGS="$CXXFLAGS -DPIXTYPE=$with_pixtype -DPIX$with_pixtype"
fi

#do a bunch of stuff if win32:
if test "$mingw_cv_win32_host" == "yes" 
then
	AC_MSG_NOTICE([adding win32 libs and flags])
	AC_CHECK_LIB([ws2_32],[main])
	CXXFLAGS="$CXXFLAGS -DLIBRAW_NODLL"
#	LDFLAGS="$LDFLAGS -static"  #commented out when I switched to mxe.static...
	INNO_SRCDIR="$srcdir"
	INNO_SUFFIX="win32"
	case "$host_alias" in
		x86_64-w64-mingw32* )
			INNO_SUFFIX="w64"
			INNO_PF="{commonpf64}"
			INNO_ARCHITECURESALLOWED="ArchitecturesAllowed=x64"
			INNO_ARCHITECTURESINSTALLEDIN64BITMODE="ArchitecturesInstallIn64BitMode=x64"
		;;
		i686-w64-mingw32* )
			INNO_SUFFIX="w32"
			INNO_PF="{commonpf32}"
			INNO_ARCHITECURES="ArchitecturesAllowed=x64\nArchitecturesInstallIn64BitMode=x64"
		;;
	esac
fi
AM_CONDITIONAL(WIN32, test x$mingw_cv_win32_host = xyes)


# Checks for libraries.

# Check for wxWidgets.
AC_ARG_ENABLE([wxwidgets],
	AS_HELP_STRING([--disable-wxwidgets], [Bypasses wxWidgets checks, use for only compiling the img or exif command line programs.]))

AS_IF([test "x$enable_wxwidgets" != "xno"], [
	WX_CONFIG_OPTIONS
	reqwx=3.1.0
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
])


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

AC_ARG_WITH([lcms2-fastfloat],
	AS_HELP_STRING([--with-lcms2-fastfloat], [use LittleCMS fast_float plugin])
)
if test "$with_lcms2_fastfloat" != ""
then
	CXXFLAGS="$CXXFLAGS -DUSE_LCMS_FASTFLOAT"
fi


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
	if test "$OPENMP_CFLAGS" == ""
	then
		PKG_CHECK_MODULES([LIBRAW],[libraw], [
			CPPFLAGS="$LIBRAW_CFLAGS $CPPFLAGS"
			LIBS="$LIBRAW_LIBS $LIBS"
		],[
			AC_MSG_ERROR([not found.])
		])
	else
		PKG_CHECK_MODULES([LIBRAW],[libraw_r], [
			CPPFLAGS="$LIBRAW_CFLAGS $CPPFLAGS"
			LIBS="$LIBRAW_LIBS $LIBS"
		],[
			AC_MSG_ERROR([not found.])
		])
	fi

fi

#lensfun:
AC_ARG_ENABLE([lensfun],
        AS_HELP_STRING([--enable-lensfun], [enables lensfun.])
)
if test "$enable_lensfun" == "yes"
then
	PKG_CHECK_MODULES([LENSFUN],[lensfun], [
		CPPFLAGS="$LENSFUN_CFLAGS $CPPFLAGS"
		LIBS="$LENSFUN_LIBS -lcurl -larchive $LIBS"
	],[
		AC_MSG_ERROR([not found.])
	])
	CXXFLAGS="$CXXFLAGS -DUSE_LENSFUN"
	if test "${LENSFUN_DATABASE_VERSION+set}" != set; then
		LENSFUN_DATABASE_VERSION="version_1"
	fi
	
fi
AM_CONDITIONAL(LENSFUN, test x$enable_lensfun = xyes)

#lensfun-update:
AC_ARG_ENABLE([lensfunupdate],
        AS_HELP_STRING([--enable-lensfunupdate], [enables lensfun update. lensfun must also be enabled.])
)
if test "$enable_lensfunupdate" == "yes"
then
	PKG_CHECK_MODULES([CURL],[libcurl], [
		CPPFLAGS="$CURL_CFLAGS $CPPFLAGS"
		LIBS="$CURL_LIBS $LIBS"
	],[
		AC_MSG_ERROR([not found.])
	])
	PKG_CHECK_MODULES([ARCHIVE],[libarchive], [
		CPPFLAGS="$ARCHIVE_CFLAGS $CPPFLAGS"
		LIBS="$ARCHIVE_LIBS $LIBS"
	],[
		AC_MSG_ERROR([not found.])
	])
	CXXFLAGS="$CXXFLAGS -DUSE_LENSFUNUPDATE"
	
fi
AM_CONDITIONAL(LENSFUNUPDATE, test x$enable_lensfunupdate = xyes)


#librtprocess:

AC_ARG_ENABLE([librtprocess],
        AS_HELP_STRING([--enable-librtprocess], [enables librtprocess, which contains additional demosaic algorithms.])
)
if test "$enable_librtprocess" == "yes"
then
	PKG_CHECK_MODULES([RTPROCESS],[rtprocess], [
		CPPFLAGS="$RTPROCESS_CFLAGS $CPPFLAGS"
		LIBS="$RTPROCESS_LIBS $LIBS"
	],[
		AC_MSG_ERROR([not found.])
	])
	CXXFLAGS="$CXXFLAGS -DUSE_LIBRTPROCESS"	
fi
AM_CONDITIONAL(LIBRTPROCESS, test x$enable_librtprocess = xyes)

#old LibRaw dcraw params, pre 0.18.7:
AC_ARG_ENABLE([old-libraw],
        AS_HELP_STRING([--enable-old-libraw], [enables old LibRaw demosaic parameters.])
)
if test "$enable_old_libraw" == "yes"
then
	CXXFLAGS="$CXXFLAGS -DOLD_LIBRAW"
	
fi

 
CPPFLAGS="$CPPFLAGS $WX_CPPFLAGS"
CXXFLAGS="$CXXFLAGS $WX_CXXFLAGS_ONLY"
CFLAGS="$CFLAGS $WX_CFLAGS_ONLY"
#LIBS="$LIBS $WX_LIBS"


# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h locale.h stdint.h stdlib.h string.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T

# Checks for library functions.
#AC_FUNC_MALLOC
#AC_CHECK_FUNCS([clock_gettime floor modf pow rint setlocale sqrt strtol])

CXXFLAGS="$OPENMP_CFLAGS $CXXFLAGS"
LIBS="$OPENMP_CFLAGS $LIBS"

#AC_SUBST(WX_CPPFLAGS)
#AC_SUBST(WX_CXXFLAGS_ONLY)
#AC_SUBST(WX_CFLAGS_ONLY)
AC_SUBST(WX_LIBS)

AC_SUBST(INNO_SRCDIR)
AC_SUBST(INNO_ARCHITECURESALLOWED)
AC_SUBST(INNO_ARCHITECTURESINSTALLEDIN64BITMODE)
AC_SUBST(INNO_PF)
AC_SUBST(INNO_SUFFIX)
AC_SUBST(LENSFUN_DATABASE_VERSION)

AC_CONFIG_FILES([Makefile src/Makefile])
if test "$mingw_cv_win32_host" == "yes"
then
	AC_CONFIG_FILES([rawproc.iss])
fi
AC_OUTPUT

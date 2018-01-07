AC_PREREQ([2.68])
AC_INIT(rawproc, 0.6.2)

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
])], [mingw_cv_win32_host=no], [mingw_cv_win32_host=yes]))])


AC_LANG([C++])
: ${CXXFLAGS=""}
AC_PROG_CXX
AC_PROG_CC
AC_PROG_MAKE_SET

MINGW_AC_WIN32_NATIVE_HOST

#Process enables:
AC_ARG_ENABLE([openmp],
        AS_HELP_STRING([--enable-openmp], [enables OpenMP threading (-fopenmp in compile and link)])
)
if test "$enable_openmp" == "yes"
then
        #APP_CXXFLAGS="$APP_CXXFLAGS -fopenmp"
        #APP_LDFLAGS="$APP_LDFLAGS -fopenmp"
	CXXFLAGS="$CXXFLAGS -fopenmp"
        LDFLAGS="$LDFLAGS -fopenmp"
fi

AC_ARG_ENABLE([debug],
        AS_HELP_STRING([--enable-debug], [enables debug compilation with -g, default is no debug switch])
)
if test "$enable_debug" == "yes"
then
	#APP_CXXFLAGS="$APP_CXXFLAGS -g"
	CXXFLAGS="$CXXFLAGS -g"
fi


AC_ARG_ENABLE([optimization],
        AS_HELP_STRING([--enable-optimization], [enables -O4 optimization, default is -O2])
)
if test "$enable_optimization" == "yes"
then
	#APP_CXXFLAGS="$APP_CXXFLAGS -O3"
	CXXFLAGS="$CXXFLAGS -O3"
else
	#APP_CXXFLAGS="$APP_CXXFLAGS -O2"
	CXXFLAGS="$CXXFLAGS -O2"
fi


# Checks for libraries.

m4_include(wxwin.m4)

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

AC_ARG_WITH([pixtype],
	AS_HELP_STRING([--with-pixtype], [=half|float|double, specifies data type for pixel structure, default is float])
)
if test "$with_pixtype" != ""
then
	#APP_CXXFLAGS="$APP_CXXFLAGS -DPIXTYPE=$with_pixtype"
	CXXFLAGS="$CXXFLAGS -DPIXTYPE=$with_pixtype"
fi


AC_ARG_WITH([localprefix],
	AS_HELP_STRING([--with-localprefix], [=PATH, specifies path to a non-standard location of usr/local/lib and usr/local/include directories.  The lib and include paths are prepended to LDFLAGS and CPPFLAGS, respectively.  Use this to make and use alternate combinations of the supporting libraries])
)
if test "$with_localprefix" != "" 
then
	if test "$with_localprefix" != "yes" ;
	then
		CPPFLAGS="-I$with_localprefix/usr/local/include $CPPFLAGS"
		LDFLAGS="-L$with_localprefix/usr/local/lib $LDFLAGS"
	fi
fi


if test "$mingw_cv_win32_host" == "yes" 
then
#	AC_MSG_NOTICE ([adding win32 libs and flags])
	AC_CHECK_LIB([ws2_32],[main])
	CXXFLAGS="$CXXFLAGS -DLIBRAW_NODLL"
	LDFLAGS="$LDFLAGS -static"
	INNO_SRCDIR="$srcdir"
	if test "$host_alias" == "x86_64-w64-mingw32"
	then
		INNO_SUFFIX="w64"
		INNO_PF="{pf64}"
		INNO_ARCHITECURESALLOWED="ArchitecturesAllowed=x64"
		INNO_ARCHITECTURESINSTALLEDIN64BITMODE="ArchitecturesInstallIn64BitMode=x64"
	fi
	if test "$host_alias" == "i686-w64-mingw32"
	then
		INNO_SUFFIX="w32"
		INNO_PF="{pf32}"
		INNO_ARCHITECURES="ArchitecturesAllowed=x64\nArchitecturesInstallIn64BitMode=x64"
	fi
fi


#libz:

AC_CHECK_HEADERS(zlib.h, [], [AC_MSG_ERROR([include directory for zlib.h not found.])])
AC_SEARCH_LIBS(inflateInit2_,z, [], [AC_MSG_ERROR([A working libz is required for some image libraries.])])


#libjpeg:


AC_ARG_WITH([jpeg-include],
	AS_HELP_STRING([--with-jpeg-include], [=PATH, specifies path to non-standard location of the libjpeg header (jpeglib.h).]),
	[
		echo "jpeglib.h: using $with_jpeg_include"
		if test -f "$with_jpeg_include/jpeglib.h" 
		then
			#APP_CPPFLAGS="-I$with_jpeg_include $APP_CPPFLAGS"
			CPPFLAGS="-I$with_jpeg_include $CPPFLAGS"
		else
			AC_MSG_ERROR([jpeglib.h not found in $with_jpeg_include]) 
		fi
	],
	[
		AC_CHECK_HEADERS(jpeglib.h, [], [AC_MSG_ERROR([include directory for jpeglib.h not found.])])
	]
)

AC_ARG_WITH([jpeg-lib],
	AS_HELP_STRING([--with-jpeg-lib], [=PATH, specifies location of the jpeg library (libjpeg.a).]),
	[
		echo "libjpeg: using $with_jpeg_lib"
		if test -f "$with_jpeg_lib" 
		then
			AC_CHECK_LIB(jpeg, jpeg_set_defaults, [AC_MSG_RESULT([$with_jpeg_lib found.])], [AC_MSG_ERROR([A working libjpeg is required.])])
			LIBS="$with_jpeg_lib $LIBS"
		else
			AC_MSG_ERROR([$with_jpeg_lib not found.]) 
		fi
	],
	[
		AC_SEARCH_LIBS(jpeg_set_defaults,jpeg, [], [AC_MSG_ERROR([A working libjpeg is required.])])
	]
)

#libtiff:

AC_ARG_WITH([tiff-include],
	AS_HELP_STRING([--with-tiff-include], [=PATH, specifies path to non-standard location of the tiff header (tiffio.h).]),
	[
		echo "libtiff: using $with_tiff_include"
		if test -f "$with_tiff_include/tiffio.h"
		then
			#APP_CPPFLAGS="-I$with_tiff_include $APP_CPPFLAGS"
			CPPFLAGS="-I$with_tiff_include $CPPFLAGS"
		else
			AC_MSG_ERROR([tiffio.h not found in $with_tiff_include.]) 
		fi
	],
	[
		AC_CHECK_HEADERS(tiffio.h, [], [AC_MSG_ERROR([tiffio.h not found.])])
	]
)

AC_ARG_WITH([tiff-lib],
	AS_HELP_STRING([--with-tiff-lib], [=PATH, specifies path to non-standard location of the tiff library (libtiff.a).]),
	[
		echo "libtiff: using $with_tiff_lib" 
		if test -f "$with_tiff_lib"
		then
			AC_CHECK_LIB(tiff, TIFFSetErrorHandler, [AC_MSG_RESULT([$with_tiff_lib found.])], [AC_MSG_ERROR([A working libtiff is required.])])
			LIBS="$with_tiff_lib $LIBS"
		else
			AC_MSG_ERROR([$with_tiff_lib not found.]) 
		fi
	],
	[
		AC_SEARCH_LIBS(TIFFSetErrorHandler,tiff, [], [AC_MSG_ERROR([A working libtiff is required.])])
	]
)

#libpng:

AC_ARG_WITH([png-include],
	AS_HELP_STRING([--with-png-include], [=PATH, specifies path to non-standard location of the png header (png.h).]),
	[
		echo "libpng: using $with_png_include"
		if test -f "$with_png_include/png.h"
		then
			CPPFLAGS="-I$with_png_include $CPPFLAGS"
		else
			AC_MSG_ERROR([png.h not found in $with_png_include.]) 
		fi
	],
	[
		AC_CHECK_HEADERS(png.h, [], [AC_MSG_ERROR([png.h not found.])])
	]
)

AC_ARG_WITH([png-lib],
	AS_HELP_STRING([--with-png-lib], [=PATH, specifies path to non-standard location of the png library (libpng.a).]),
	[
		echo "libpng: using $with_png_lib"
		if test -f "$with_png_lib"
		then
			AC_CHECK_LIB(png, png_get_io_ptr, [AC_MSG_RESULT([$with_png_lib found.])], [AC_MSG_ERROR([A working libpng is required.])])
			LIBS="$with_png_lib $LIBS"
		else
			AC_MSG_ERROR([$with_png_lib not found.]) 
		fi
	],
	[
		AC_SEARCH_LIBS(png_get_io_ptr,png, [], [AC_MSG_ERROR([A working libpng is required.])])
	]
)


#lcms2:
AC_ARG_WITH([lcms2-include],
	AS_HELP_STRING([--with-lcms2-include], [=PATH, specifies path to non-standard location of the Little CMS2 header (lcms2.h).]),
	[
	echo "lcms2.h: using $with_lcms2_include"
		if test -f "$with_lcms2_include/lcms2.h" 
		then
			#APP_CPPFLAGS="-I$with_lcms2_include $APP_CPPFLAGS "
			CPPFLAGS="-I$with_lcms2_include $CPPFLAGS "
		else
			AC_MSG_ERROR([lcms2.h not found in $with_lcms2_include.]) 
		fi
	],
	[
		AC_CHECK_HEADERS(lcms2.h, [], [AC_MSG_ERROR([lcms2.h not found])])
	]
)

AC_ARG_WITH([lcms2-lib],
	AS_HELP_STRING([--with-lcms2-lib], [=PATH, specifies location of the Little CMS2 library (liblcms2.a).]),
	[
	echo "lcms2.h: using $with_lcms2_lib"
		if test -f $with_lcms2_lib 
		then
			AC_CHECK_LIB(lcms2, cmsSetLogErrorHandler, [AC_MSG_RESULT([$with_lcms2_lib found.])], [AC_MSG_ERROR([A working liblcms2 is required.])])
			LIBS="$with_lcms2_lib $LIBS"
		else
			AC_MSG_ERROR([$with_lcms2_lib not found.]) 
		fi
	],
	[
		AC_SEARCH_LIBS(cmsSetLogErrorHandler,lcms2, [], [AC_MSG_ERROR([A working libcms2 is required])])
	]
)


#libraw:
AC_ARG_WITH([libraw-include],
	AS_HELP_STRING([--with-libraw-include], [=PATH, specifies path to non-standard location of the LibRaw header (libraw/libraw.h).]),
	[
	echo "libraw: using $with_libraw_include"
		if test -f "$with_libraw_include/libraw/libraw.h"
		then
			#APP_CPPFLAGS="-I$with_libraw_include $APP_CPPFLAGS"
			CPPFLAGS="-I$with_libraw_include $CPPFLAGS"
		else
			AC_MSG_ERROR([libraw/libraw.h not found in $with_libraw_include.]) 
		fi
	],
	[
		AC_CHECK_HEADERS(libraw/libraw.h, [], [AC_MSG_ERROR([libraw.h not found])])
	]
)

AC_ARG_WITH([libraw-lib],
	AS_HELP_STRING([--with-libraw-lib], [=PATH, specifies location of the LibRaw library (libraw.a).]),
	[
	echo "libraw: using $with_libraw_lib"
		if test -f $with_libraw_lib 
		then
			AC_CHECK_LIB(raw, libraw_init, [AC_MSG_RESULT([$with_libraw_lib found.])], [AC_MSG_ERROR([A working libraw is required.])])
			LIBS="$with_libraw_lib $LIBS"
		else
			AC_MSG_ERROR([$with_libraw_lib not found.]) 
		fi
	],
	[
		AC_SEARCH_LIBS(libraw_init,raw, [], [AC_MSG_ERROR([A working libraw is required])])
	]
)

# Checks for header files.
AC_CHECK_HEADERS([inttypes.h stdint.h stdlib.h string.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
#AC_CHECK_HEADER_STDBOOL
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_CHECK_FUNCS([clock_gettime floor pow sqrt])


AC_SUBST(WX_CPPFLAGS)
AC_SUBST(WX_CXXFLAGS_ONLY)
AC_SUBST(WX_CFLAGS_ONLY)
AC_SUBST(WX_LIBS)

AC_SUBST(LIBS)
AC_SUBST(CPPFLAGS)
AC_SUBST(CXXFLAGS)
AC_SUBST(CFLAGS)

AC_SUBST(INNO_SRCDIR)
AC_SUBST(INNO_ARCHITECURESALLOWED)
AC_SUBST(INNO_ARCHITECTURESINSTALLEDIN64BITMODE)
AC_SUBST(INNO_PF)
AC_SUBST(INNO_SUFFIX)


AC_OUTPUT(Makefile winbuild/rawproc.iss)

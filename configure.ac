AC_PREREQ([2.68])
AC_INIT(rawproc, 0.6.2)
AC_CONFIG_MACRO_DIR([m4])

RAWPROCVERSION=`grep 'wxString version' ../rawprocFrm.cpp |sed 's/wxString version = //' | sed 's/"//g' |sed 's/;//'`

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

objs="util.o elapsedtime.o myConfig.o myEXIFDialog.o myDoubleSlider.o myPropertyDialog.o myHistogramPane.o myHistogramDialog.o CurvePane.o PicProcessor.o PicProcessorBlackWhitePoint.o PicProcessorHighlight.o PicProcessorShadow.o PicProcessorCurve.o PicProcessorGamma.o PicProcessorBright.o PicProcessorContrast.o PicProcessorSaturation.o PicProcessorExposure.o PicProcessorRedEye.o PicProcessorSharpen.o PicProcessorResize.o PicProcessorDenoise.o PicProcessorRotate.o PicProcessorGray.o PicProcessorCrop.o PicProcessorColorSpace.o rawprocFrm.o rawprocApp.o PicProcPanel.o PicPanel.o"

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


AC_ARG_ENABLE([dcraw],
        AS_HELP_STRING([--enable-dcraw], [enables use of a dcraw executable for raw file input, in lieu of LibRaw. dcraw mus be in a directory listed in PATH, or an absolute path specified with input.raw.dcraw.programpath])
)
if test "$enable_dcraw" == "yes"
then
	CXXFLAGS="$CXXFLAGS -DUSE_DCRAW"
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


#libraries possibly needed by the primary libraries:

AC_SEARCH_LIBS(inflateInit2_,z,  [], [])
AC_SEARCH_LIBS(lzma_code,lzma,   [], [])
AC_SEARCH_LIBS(jas_init,jasper,  [], [])

if test "$host_alias" != ""
then
	PKG_CONFIG_SYSROOT_DIR="/usr/$host_alias"
fi

#libjpeg:

AC_MSG_CHECKING([for libjpeg library])
libjpeg="libjpeg"
LIBJPEG_EXISTS=`pkg-config $libjpeg --exists`
if test "$?" == "0"
then
	LIBJPEG_CFLAGS=`pkg-config $libjpeg --cflags`
	LIBJPEG_LIBS=`pkg-config $libjpeg --static --libs-only-l`
	LIBJPEG_LIBDIRS=`pkg-config $libjpeg --libs-only-L`
	LIBS="$LIBJPEG_LIBS $LIBS"
	LDFLAGS="$LIBJPEG_LIBDIRS $LDFLAGS"
	CPPFLAGS="$LIBJPEG_CFLAGS $CPPFLAGS"
	AC_MSG_RESULT([installed library: $LIBJPEG_CFLAGS $LIBJPEG_LIBS])
else
	AC_CHECK_HEADERS(jpeglib.h, [], [AC_MSG_ERROR([jpeglib.h not found.])])
	AC_SEARCH_LIBS(jpeg_set_defaults,jpeg, [], [AC_MSG_ERROR([A working libjpeg is required.])])
fi


#libtiff:

AC_MSG_CHECKING([for libtiff library])
libtiff=libtiff-4
LIBTIFF_EXISTS=`pkg-config $libtiff --exists`
if test "$?" == "0"
then
	LIBTIFF_CFLAGS=`pkg-config $libtiff --cflags`
	LIBTIFF_LIBS=`pkg-config $libtiff --static --libs-only-l`
	LIBTIFF_LIBDIRS=`pkg-config $libtiff --libs-only-L`
	LIBS="$LIBTIFF_LIBS $LIBS"
	#m4_append_uniq_w([LIBS], [LIBTIFF_LIBS])dnl
	LDFLAGS="$LIBTIFF_LIBDIRS $LDFLAGS"
	CPPFLAGS="$LIBTIFF_CFLAGS $CPPFLAGS"
	AC_MSG_RESULT([installed library: $LIBTIFF_CFLAGS $LIBTIFF_LIBS])
else
	AC_CHECK_HEADERS(tiffio.h, [], [AC_MSG_ERROR([tiffio.h not found.])])
	AC_SEARCH_LIBS(TIFFSetErrorHandler,tiff, [], [AC_MSG_ERROR([A working libtiff is required.])])
#	AC_MSG_ERROR([libtiff not found.])
fi


#libpng:

AC_MSG_CHECKING([for libpng library])
libpng="libpng"
LIBPNG_EXISTS=`pkg-config $libpng --exists`
if test "$?" == "0"
then
	LIBPNG_CFLAGS=`pkg-config $libpng --cflags`
	LIBPNG_LIBS=`pkg-config $libpng --static --libs-only-l`
	LIBPNG_LIBDIRS=`pkg-config $libpng --libs-only-L`
	LIBS="$LIBPNG_LIBS $LIBS"
	LDFLAGS="$LIBPNG_LIBDIRS $LDFLAGS"
	CPPFLAGS="$LIBPNG_CFLAGS $CPPFLAGS"
	AC_MSG_RESULT([installed library: $LIBPNG_CFLAGS $LIBPNG_LIBS])
else
	AC_CHECK_HEADERS(png.h, [], [AC_MSG_ERROR([png.h not found.])])
	AC_SEARCH_LIBS(png_get_io_ptr,png, [], [AC_MSG_ERROR([A working libpng is required.])])
fi




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
if test "$enable_dcraw" != "yes"
then
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
fi

AC_ARG_ENABLE([lensfun],
        AS_HELP_STRING([--enable-lensfun], [enables lensfun])
)
if test "$enable_lensfun" == "yes"
then
	AC_MSG_CHECKING([for lensfun library])
	lensfun="lensfun"
	LENSFUN_EXISTS=`pkg-config $lensfun --exists`
	if test "$?" == "0"
	then
		LENSFUN_CFLAGS=`pkg-config $lensfun --cflags`
		LENSFUN_LIBS=`pkg-config $lensfun --static --libs-only-l`
		LENSFUN_LDFLAGS=`pkg-config $lensfun --libs-only-L`
		LIBS="$LENSFUN_LIBS $LIBS"
		CPPFLAGS="$LENSFUN_CFLAGS $CPPFLAGS"
		LDFLAGS="$LENSFUN_LDFLAGS $LDFLAGS"
		AC_MSG_RESULT([$LENSFUN_LIBS])
	else
		AC_MSG_ERROR([lensfun not found.])

	fi
	CXXFLAGS="$CXXFLAGS -DUSE_LENSFUN"
	objs="$objs PicProcessorLensCorrection.o"
fi

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

AC_SUBST(RAWPROCVERSION)

AC_SUBST(WX_CPPFLAGS)
AC_SUBST(WX_CXXFLAGS_ONLY)
AC_SUBST(WX_CFLAGS_ONLY)
AC_SUBST(WX_LIBS)

AC_SUBST(LIBS)
AC_SUBST(LDFLAGS)
AC_SUBST(CPPFLAGS)
AC_SUBST(CXXFLAGS)
AC_SUBST(CFLAGS)
AC_SUBST(objs)

AC_SUBST(INNO_SRCDIR)
AC_SUBST(INNO_ARCHITECURESALLOWED)
AC_SUBST(INNO_ARCHITECTURESINSTALLEDIN64BITMODE)
AC_SUBST(INNO_PF)
AC_SUBST(INNO_SUFFIX)


AC_OUTPUT(Makefile rawproc.iss)

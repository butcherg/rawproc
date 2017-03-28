#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT(rawproc, 0.6Dev)
AC_LANG([C++])
: ${CXXFLAGS=""}


# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
#AC_PROG_CPP
AC_PROG_MAKE_SET

#LDFLAGS=-static 

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
        CXXFLAGS="$CXXFLAGS -O4"
else
        CXXFLAGS="$CXXFLAGS -O2"
fi




#Process withs:
AC_ARG_WITH([localprefix],
        AS_HELP_STRING([--with-localprefix], [=PATH, specifies path to a non-standard location of usr/local/lib and usr/local/include directories.  The lib and include paths are prepended to LDFLAGS and CPPFLAGS, respectively.  Use this to make and use alternate combinations of the supporting libraries])
)
if test "$with_localprefix" != ""
then
        if test "$with_localprefix" != "yes"
        then
                CPPFLAGS="-I$with_localprefix/usr/local/include -I$with_localprefix/usr/local/include/gimage $CPPFLAGS"
                LDFLAGS="-L$with_localprefix/usr/local/lib $LDFLAGS"
        fi
fi

AC_ARG_WITH([libraw],
        AS_HELP_STRING([--with-libraw], [=PATH, specifies path to non-standard location of LibRaw library. Omitting the path looks for a Libraw* directory in the cwd.])
)
if test "$with_libraw" == "yes"  #look for unpacked tiff library in ./
then
        LIBRAW=`stat --printf='%n' LibRaw-* 2>/dev/null`
        if test $LIBRAW == ""  
        then 
                AC_ERROR([no LibRaw library found in ./]) 
        fi
        if test -d $LIBRAW
        then
                CPPFLAGS="$CPPFLAGS -I$LIBRAW"
                LDFLAGS="$LDFLAGS -L$LIBRAW/lib/.libs"
        else
                AC_ERROR([thing found is not a libraw library in ./])
        fi
else
        if test "$with_libraw" != ""
        then
                CPPFLAGS="$CPPFLAGS -I$with_libraw"
                LDFLAGS="$LDFLAGS -L$with_libraw/lib/.libs"
        fi
fi



# Checks for libraries.

m4_include(wxwin.m4)

AM_OPTIONS_WXCONFIG
reqwx=3.1.0
AM_PATH_WXCONFIG([$reqwx], [wxWin=1], [wxWin=0], [std,aui,propgrid], [])
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



AC_CHECK_HEADERS(jpeglib.h, [], [AC_ERROR([jpeglib.h not found])])
AC_SEARCH_LIBS(jpeg_set_defaults,jpeg, [], [AC_ERROR([A working libjpeg is required])])

AC_CHECK_HEADERS(tiffio.h, [], [AC_ERROR([tiffio.h not found])])
AC_SEARCH_LIBS(TIFFSetErrorHandler,tiff, [], [AC_ERROR([A working libtiff is required])])

AC_CHECK_HEADERS(lcms2.h, [], [AC_ERROR([lcms2.h not found])])
AC_SEARCH_LIBS(cmsSetLogErrorHandler,lcms2, [], [AC_ERROR([A working libcms2 is required])])

AC_CHECK_HEADERS(libraw/libraw.h, [], [AC_ERROR([libraw.h not found])])
AC_SEARCH_LIBS(libraw_init,raw, [], [AC_ERROR([A working libraw is required])])

AC_CHECK_HEADERS(gimage.h, [], [AC_ERROR([gimage.h not found])])
AC_CHECK_LIB([gimage],[main],[],[AC_ERROR([A working gimage is required])])

# Checks for header files.
AC_CHECK_HEADERS([inttypes.h stdint.h stdlib.h string.h sys/time.h unistd.h])

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

AC_OUTPUT(Makefile)

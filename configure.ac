#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([FULL-PACKAGE-NAME], [VERSION], [BUG-REPORT-ADDRESS])
AC_CONFIG_SRCDIR([PicProcessorCurve.h])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_MAKE_SET

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lgimage':
AC_CHECK_LIB([gimage], [main])
# FIXME: Replace `main' with a function in `-ljpeg':
AC_CHECK_LIB([jpeg], [main])
# FIXME: Replace `main' with a function in `-llcms2':
AC_CHECK_LIB([lcms2], [main])
# FIXME: Replace `main' with a function in `-lraw':
AC_CHECK_LIB([raw], [main])
# FIXME: Replace `main' with a function in `-ltiff':
AC_CHECK_LIB([tiff], [main])

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

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

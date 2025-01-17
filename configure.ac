#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([FCA-AF], [0.0.1])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/affca.c])
AC_CONFIG_HEADERS([config.h])
AC_PROG_LEX
AC_PROG_YACC
AC_PROG_RANLIB
AC_C_INLINE

# Checks for programs.
AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_INT8_T
AC_TYPE_UINT8_T
AC_TYPE_SIZE_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([memset])
AC_CHECK_FUNCS([strdup])

AC_CONFIG_FILES([Makefile 
	src/Makefile 
	src/utils/Makefile
	src/parser/Makefile
	src/af/Makefile
	src/algorithms/Makefile
	src/validator/Makefile
	src/test/Makefile])
AC_OUTPUT

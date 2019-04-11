# init
AC_INIT([libgatha], [0.2], [alexis@lamiable.fr])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([-Wall -Werror])
LT_INIT

# check for programs
AC_PROG_CC
AC_C_INLINE

# check for headers
AC_CHECK_HEADERS([math.h string.h stdarg.h])
AC_HEADER_ASSERT
AC_CHECK_FUNCS_ONCE([snprintf])
AC_CHECK_LIB([m], [sqrt])

# check for cairo
AC_ARG_WITH([cairo], [AS_HELP_STRING([--with-cairo],
 			 	     [Build with the Cairo visualization programs])],
	    [],
	    [with_cairo=yes])

AS_IF([test "x$with_cairo" = "xyes"], [
   PKG_CHECK_MODULES([GLIB], [glib-2.0])
   PKG_CHECK_MODULES([CAIRO], [cairo >= 1.10.0],
   			      [AC_DEFINE([HAVE_CAIRO], [1], [Define to 1 if cairo is available])
			       AM_CONDITIONAL([CAIRO], [true])],
			      [echo "Cairo is not available. Install Cairo and reconfigure if you want visualization programs."])
], [
   AM_CONDITIONAL([CAIRO], [false])
])

# finish
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile lib/Makefile examples/Makefile])
AC_OUTPUT
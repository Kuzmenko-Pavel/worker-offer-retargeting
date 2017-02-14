dnl Process this file with autoconf

AC_INIT([worker-offer-retargeting], m4_esyscmd([echo -n $(git describe --all --tags --long | sed "s/v\([0-9]\.[0-9]-.*\)-.*/\1/")]), [kyzmenko.pavel@gmail.com])

AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([subdir-objects])

AC_PROG_CXX
AC_LANG_CPLUSPLUS
AC_LANG_PUSH([C++])
AC_PROG_MAKE_SET
AC_DISABLE_STATIC


dnl Check for libtool
LT_PREREQ([2.2.6])
LT_INIT

AC_SUBST([DOLLAR_SIGN],[$])


AC_HAVE_HEADERS(string.h fcntl.h sys/file.h sys/param.h)
AC_CHECK_FUNCS([alarm])
AC_CHECK_FUNCS([bzero])
AC_CHECK_FUNCS([clock_gettime])
AC_CHECK_FUNCS([memset])
AC_CHECK_FUNCS([socket])
AC_CHECK_FUNCS([strchr])
AC_CHECK_FUNCS([strerror])
AC_CHECK_FUNCS([strstr])
AC_CHECK_FUNCS([strtol])
AC_CHECK_FUNCS([clock_gettime fdatasync gettimeofday localtime_r memmove memset munmap strdup strerror strtol utime])
AC_CHECK_HEADERS([arpa/inet.h])
AC_CHECK_HEADERS([netdb.h])
AC_CHECK_HEADERS([netinet/in.h])
AC_CHECK_HEADERS([sys/socket.h])
AC_CHECK_HEADERS([syslog.h])
AC_CHECK_HEADERS([fcntl.h inttypes.h limits.h malloc.h stddef.h stdint.h stdlib.h string.h sys/file.h sys/ioctl.h sys/mount.h sys/param.h sys/time.h unistd.h utime.h])
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_FUNC_ERROR_AT_LINE
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_MKTIME
AC_PROG_RANLIB
AC_TYPE_INT64_T
AC_TYPE_INT8_T
AC_TYPE_MODE_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UID_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

AC_ARG_ENABLE(debug,
AS_HELP_STRING([--enable-debug],
	[enable debugging, default: no]),
		[case "${enableval}" in
		yes) debug=true ;;
		no)  debug=false ;;
		*)   AC_MSG_ERROR([bad value ${enableval} for --enable-debug]) ;;
		esac],
		[debug=false])
AM_CONDITIONAL(DEBUG, test x"$debug" = x"true")


PKG_CHECK_MODULES(MONGO, libmongocxx)
PKG_CHECK_MODULES(SQL, kompex-sqlite-wrapper)
PKG_CHECK_MODULES(AMQP, librabbitmq)
AC_CHECK_LIB([amqpcpp],[main],[],[AC_MSG_ERROR([amqpcpp library missing])])
AC_CHECK_LIB([mongocxx],[main],[],[AC_MSG_WARN([mongocxx library missing])])
AC_CHECK_LIB([bsoncxx],[main],[],[AC_MSG_WARN([bsoncxx library missing])])
AC_CHECK_LIB([fcgi], [FCGX_FPrintF], [], [AC_MSG_ERROR([fcgi was not found])])
AC_CHECK_LIB([GeoIP], [GeoIP_region_name_by_code], [], [AC_MSG_ERROR([GeoIP was not found])])
AC_CHECK_LIB([tinyxml], [main], [], [AC_MSG_ERROR([tinyxml was not found])])
AC_CHECK_LIB([icuuc],[main],[],[AC_MSG_WARN([icuuc library missing])])
AC_CHECK_LIB([icui18n],[main],[],[AC_MSG_ERROR([icui18n library missing])])
AC_CHECK_LIB([icudata],[main],[],[AC_MSG_ERROR([icudata library missing])])

BOOST_REQUIRE([1.58],AC_MSG_ERROR([boost>=1.58 required]))
BOOST_FILESYSTEM()
BOOST_THREADS()
BOOST_DATE_TIME()
BOOST_REGEX()


AC_CONFIG_HEADERS([config.h])

AC_CONFIG_FILES([
 Makefile
])

AC_OUTPUT

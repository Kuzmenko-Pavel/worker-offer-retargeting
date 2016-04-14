dnl Process this file with autoconf

AC_INIT([worker-offer-retargeting], m4_esyscmd([echo -n $(git describe --all --tags --long | sed "s/v\([0-9]\.[0-9]-.*\)-.*/\1/")]), [kyzmenko.pavel@gmail.com])

AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE

AC_PROG_CC
AC_PROG_CXX
AC_PROG_MAKE_SET
AC_PROG_RANLIB
AC_PROG_INSTALL
AM_PROG_CC_C_O

AC_LANG_CPLUSPLUS


dnl Check for libtool
LT_PREREQ([2.2.6])
LT_INIT([disable-static])
LTCFLAGS="-O2"
LTCXXFLAGS="-O2"

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


AC_CHECK_LIB([amqpcpp],[main],[],[AC_MSG_ERROR([amqpcpp library missing])])
AC_CHECK_LIB([mongoclient],[main],[],[AC_MSG_ERROR([mongoclient(mongodb build with client shared library) missing])])

AC_LANG(C++)
SAVED_LDFLAGS=$LDFLAGS
LDFLAGS="$LDFLAGS -lamqpcpp"
AC_LINK_IFELSE(
    [AC_LANG_PROGRAM([#include <AMQPcpp.h>],
    [AMQPMessage *m;
    unsigned len;
    char *pMes;
    pMes = m->getMessage(&len);])],
    [],
    [AC_DEFINE(AMQPCPP_OLD,[1],[amqpcpp library old])])
LDFLAGS="$SAVED_LDFLAGS -lmongoclient -lboost_system"
AC_LINK_IFELSE(
    [AC_LANG_PROGRAM([#include <mongo/client/connpool.h>],
    [mongo::ScopedDbConnection *db;
     db = mongo::ScopedDbConnection::getScopedDbConnection("", 0);])],
     [],
    [AC_DEFINE(MONGO_2_0,[],[mongoclient library old])])

lDFLAGS=$SAVED_LDFLAGS


AC_CHECK_LIB([icuuc],[u_tolower],[],[AC_MSG_WARN([icuuc library missing])])
AC_CHECK_LIB([icui18n],[main],[],[AC_MSG_ERROR([icui18n library missing])])
AC_CHECK_LIB([icudata],[main],[],[AC_MSG_ERROR([icudata library missing])])

BOOST_REQUIRE([1.48],AC_MSG_ERROR([boost>=1.48 required]))
BOOST_FILESYSTEM()
BOOST_THREADS()
BOOST_DATE_TIME()
BOOST_REGEX()

AC_CHECK_LIB([fcgi], [FCGX_FPrintF], [], [AC_MSG_ERROR([fcgi was not found])])
AC_CHECK_LIB([tinyxml], [main], [], [AC_MSG_ERROR([tinyxml was not found])])
AC_CHECK_LIB([rabbitmq],[main],[],[AC_MSG_ERROR([rabbitmq library missing])])

CPPFLAGS="-W -Wall"

AC_CONFIG_HEADERS([config.h])

AC_CONFIG_FILES([
 Makefile
])

AC_OUTPUT

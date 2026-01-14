# SPDX-License-Identifier: GPL-2.0+

# Copyright © 2022-2023,2026 Collabora Ltd
# Copyright © 2022-2023,2026 Valve Corporation

AC_INIT([steamos-reset],[0.02],[vivek@collabora.com])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([-Wno-portability foreign subdir-objects])
AM_SILENT_RULES([yes])

AC_ARG_WITH([service-port],
            [AS_HELP_STRING([--with-service-port=PORT],
                            [the TCP port to listen on [8080]])],
            [SRVPORT="$with_service_port"],
            [SRVPORT=8080])
AC_SUBST([SRVPORT])

AC_ARG_WITH([service-address],
            [AS_HELP_STRING([--with-service-address=ADDRESS],
                            [IPv4 address to bind to [127.0.0.1]])],
            [SRVADDR="$with_service_address"],
            [SRVADDR="127.0.0.1"])
AC_SUBST([SRVADDR])

AC_ARG_WITH([service-user],
            [AS_HELP_STRING([--with-service-user=USER],
                            [User to run the backend service as [http]])],
            [SRVUSER="$with_service_user"],
            [SRVUSER="http"])
AC_SUBST([SRVUSER])

AC_ARG_WITH([service-group],
            [AS_HELP_STRING([--with-service-group=GROUP],
                            [User to run the backend service as [same as --with-service-user]])],
            [SRVGROUP="$with_service_group"],
            [SRVGROUP="$SRVUSER"])
AC_SUBST([SRVGROUP])

AC_ARG_WITH([ui],
            [AS_HELP_STRING([--with-ui=<all,qml,cef,cli>],[UI to build [cli]])],
            [UI="$with_ui"],
            [UI="cli"])
AC_SUBST([UI])

dnl Only need X11 if we're building CEF or QML front ends
AS_CASE([$UI],
        [cli],[true],
        [PKG_CHECK_MODULES([X11],[x11])])

AC_PROG_INSTALL
AC_PROG_CC
AC_PROG_CXX
AC_PROG_RANLIB
AC_SUBST([prefix],[$prefix])
AC_SUBST([PKGDATADIR],[$datarootdir])
AC_SUBST([LIBEXECDIR],[$libexecdir])
AC_SUBST([PKGLIBDIR],[$libdir/$PACKAGE])

AC_CONFIG_FILES([Makefile holo-reset.sh holo-reset holo-reset-tool
                 lighttpd/holo-reset.conf])
AC_OUTPUT

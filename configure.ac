# SPDX-License-Identifier: GPL-2.0+

# Copyright © 2022-2023,2026 Collabora Ltd
# Copyright © 2022-2023,2026 Valve Corporation

AC_INIT([holo-reset],[0.03],[vivek@collabora.com])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([-Wno-portability foreign subdir-objects])
AM_SILENT_RULES([yes])

AC_PROG_INSTALL
AC_PROG_CC
AC_PROG_CXX
AC_PROG_RANLIB
AC_SUBST([prefix],[$prefix])
AC_SUBST([PKGDATADIR],[$datarootdir])
AC_SUBST([LIBEXECDIR],[$libexecdir])
AC_SUBST([PKGLIBDIR],[$libdir/$PACKAGE])

AC_CONFIG_FILES([Makefile holo-reset.sh holo-reset-tool])
AC_OUTPUT

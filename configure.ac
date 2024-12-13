# Copyright (c) 2021 The Linux Foundation. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above
#       copyright notice, this list of conditions and the following
#       disclaimer in the documentation and/or other materials provided
#       with the distribution.
#     * Neither the name of The Linux Foundation nor the names of its
#       contributors may be used to endorse or promote products derived
#       from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
# WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
# ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
# BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
# BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
# OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
# IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

AC_PREREQ(2.61)

m4_define([mesa_version], [21.1.1])
m4_define([gbm_version], [mesa_version])
AC_INIT([libgbm],
    [gbm_version],
    [],
    libgbm)

AM_INIT_AUTOMAKE([-Wall -Wno-error foreign subdir-objects])
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])
LT_INIT
AM_MAINTAINER_MODE

AC_CONFIG_HEADER([config.h])
AC_CONFIG_MACRO_DIR([m4])

# Check for programs
AM_PROG_AR

PKG_CHECK_MODULES([LIBDMABUFHEAP], [libdmabufheap])
AC_SUBST([LIBDMABUFHEAP_CFLAGS])
AC_SUBST([LIBDMABUFHEAP_LIBS])

PKG_CHECK_MODULES([LIBVMMEM], [libvmmem])
AC_SUBST([LIBVMMEM_CFLAGS])
AC_SUBST([LIBVMMEM_LIBS])

AC_ARG_WITH([glib],
      AC_HELP_STRING([--with-glib],
         [enable glib, building HLOS systems which use glib]))

if (test "x${with_glib}" = "xyes"); then
        AC_DEFINE(ENABLE_USEGLIB, 1, [Define if HLOS systems uses glib])
        PKG_CHECK_MODULES(GTHREAD, gthread-2.0 >= 2.16, dummy=yes,
                                AC_MSG_ERROR(GThread >= 2.16 is required))
        PKG_CHECK_MODULES(GLIB, glib-2.0 >= 2.16, dummy=yes,
                                AC_MSG_ERROR(GLib >= 2.16 is required))
        GLIB_CFLAGS="$GLIB_CFLAGS $GTHREAD_CFLAGS"
        GLIB_LIBS="$GLIB_LIBS $GTHREAD_LIBS"

        AC_SUBST(GLIB_CFLAGS)
        AC_SUBST(GLIB_LIBS)
fi

if test "x${with_glib}" = "xyes"; then
    CFLAGS="$CFLAGS -I$GLIB_CFLAGS -DUSE_GLIB"
    LDFLAGS="$LDFLAGS -lglib-2.0"
fi

AC_ARG_WITH([core_includes],
    AC_HELP_STRING([--with-core-includes=@<:@dir@:>@],
       [Specify the location of the core headers]),
    [core_incdir=$withval],
    with_core_includes=no)

if test "x$with_core_includes" != "xno"; then
   CFLAGS="${CFLAGS} -I${core_incdir}"
fi

#pkgconfigdir=${libdir}/pkgconfig
#AC_SUBST(pkgconfigdir)

AC_ARG_WITH(sanitized-headers,
   AS_HELP_STRING([--with-sanitized-headers=DIR],
       [Specify the location of the sanitized Linux headers]),
   [CPPFLAGS="$CPPFLAGS -I$withval"])

AC_ARG_ENABLE([wayland-support],
   AC_HELP_STRING([--enable-wayland-support],
      [with wayland support, build gbm with wayland-support]),
      [enable_waylandsupport="${enableval}"],
      enable_waylandsupport=no)
AM_CONDITIONAL(BUILD_HAS_WAYLAND_SUPPORT, test "x${enable_waylandsupport}" = "xyes")
if test "x${enable_waylandsupport}" = "xyes"; then
    AC_DEFINE(BUILD_HAS_WAYLAND_SUPPORT, 1, [Enable when build has wayland support])
fi


# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_CXX
AC_PROG_LIBTOOL
AC_PROG_AWK
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

#PKG_CHECK_MODULES(LIBDRM, [libdrm])

ALLOCATE_SURFACE_BO_AT_CREATION=yes

AM_CONDITIONAL(ALLOCATE_SURFACE_BO_AT_CREATION, test $ALLOCATE_SURFACE_BO_AT_CREATION = yes)

if test $ALLOCATE_SURFACE_BO_AT_CREATION = yes; then
        AC_DEFINE(ALLOCATE_SURFACE_BO_AT_CREATION, 1, [Enable surface's BO creation at surface creation time])
fi

GET_FD_WITH_NEW=yes

AM_CONDITIONAL(GET_FD_WITH_NEW, test "x${GET_FD_WITH_NEW}" = "xyes")

if test "x${GET_FD_WITH_NEW}" = "xyes"; then
    AC_DEFINE(GET_FD_WITH_NEW, 1, [each call returns a new file descriptor with get_fd])
fi

AC_CONFIG_FILES([ \
    Makefile \
    gbm.pc
    ])

AC_SUBST([CFLAGS])
AC_SUBST([CPPFLAGS])
AC_SUBST([LIBS])

AC_OUTPUT
#AC_OUTPUT([Makefile src/Makefile])

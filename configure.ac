dnl                                               -*- Autoconf -*-
dnl Process this file with autoconf to produce a configure script.

dnl Not sure which autoconf version we need, but 2.68 (from 2010)
dnl is widely available.
AC_PREREQ([2.68])

AC_INIT([PyNormaliz], VERSION, [https://github.com/Normaliz/PyNormaliz])
AC_COPYRIGHT([GNU Lesser General Public License version 2 or later])
AC_CONFIG_FILES([config.py])
AC_PROG_CXX

AC_LANG(C++)

dnl Check and enforce C++11
m4_include([m4/ax_cxx_compile_stdcxx.m4])
AX_CXX_COMPILE_STDCXX(11)

dnl Check Normaliz presence
have_normaliz=no
AC_LINK_IFELSE([AC_LANG_PROGRAM([
#include <gmpxx.h>
#include <libnormaliz/cone.h>
],[
libnormaliz::Cone<mpz_class> *C;
])], [have_normaliz=yes])

if test "$have_normaliz" = "no"; then
    AC_MSG_ERROR("Normaliz not found")
fi

dnl Check whether Normaliz has been built with number field support
have_enfnormaliz=no
AC_RUN_IFELSE([AC_LANG_PROGRAM([
#include <libnormaliz/nmz_config.h>
],[
#ifndef ENFNORMALIZ
exit(1);
#endif
])], [have_enfnormaliz=yes])

if test "$have_enfnormaliz" = "yes"; then
    AC_SUBST(ENFNORMALIZ, True)
else
    AC_SUBST(ENFNORMALIZ, False)
fi

AC_OUTPUT

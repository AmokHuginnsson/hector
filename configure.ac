dnl configure.ac is integral part of hector project.

dnl Initialization data.
pushdef([MY_PACKAGE_VERSION],[0.0.0-esyscmd(date +%Y%m%d|tr -d '\n')])
AC_INIT([[hector]],[MY_PACKAGE_VERSION],[[amok@codestation.org]])

AC_MSG_RESULT([--- Welcome to hector configurator ---                   [[0%]]])
AC_MSG_RESULT([--- Initialization ... ---                               [[6%]]])

dnl Unique file in project: hectorrc is sample configuration file.
AC_MSG_RESULT([--- Finding base directory ... ---                      [[12%]]])
AC_CONFIG_SRCDIR([[hectorrc]])
AC_CONFIG_AUX_DIR([../yaal/_aux])

dnl hector is not Free Software.
AC_COPYRIGHT([[hector (c) 2003 All rights reserved.]])

AC_CHECK_PROG(HAS_GIT,[git],["true"],["false"])
AC_MSG_CHECKING([git id sub-command])
AC_SUBST(GITID,[true])
if test ["$HAS_GIT"] = ["true"] ; then
	THIS_ID=`git id ${0}.ac 2> /dev/null`
	if test ["x${THIS_ID}"] != ["x"] ; then
		AC_SUBST(GITID,["git id"])
		AC_MSG_RESULT([ok])
	else
		AC_MSG_WARN([not present])
	fi
else
	AC_MSG_WARN([no git in the first place])
fi

dnl hector is still in alpha stage.
AC_REVISION(${THIS_ID})

dnl We select default language.
AC_LANG([C++])
CXXFLAGS=["-L~/lib -L/usr/local/lib -I~/usr/include -I/usr/local/include"]

dnl Set up how configure will handle header checks.
AC_CONFIG_HEADERS([config.hxx])

dnl Cheching for machine type.
AC_MSG_RESULT([--- Checking for machine type ... ---                   [[14%]]])
AC_CANONICAL_HOST()

dnl Testing presence of c compiler
if test ["$CXX"] = ["colorgcc"] -o ["$CXX"] = ["colorg++"] ; then
	CXX=["g++"]
fi
if test ["$CC"] = ["colorgcc"] ; then
	CC=["gcc"]
fi
AC_MSG_RESULT([--- Testing presence of c compiler ... ---              [[19%]]])
AC_PROG_CC
dnl and for c++ compiler
AC_MSG_RESULT([--- Testing presence of c++ compiler ... ---            [[25%]]])
AC_PROG_CXX
dnl and for c preprocesor
AC_MSG_RESULT([--- Looking standrad development software ... ---       [[33%]]])
AC_PROG_CPP
AC_PROG_CXXCPP
AC_CHECK_PROG([CTAGS],[exctags],[exctags],[ctags])

dnl Check compiler version.
AC_MSG_CHECKING([compiler version])
GCC_MAJOR=`echo | cpp -dM | grep __GNUC__ | awk '{print $3}'`
GCC_MINOR=`echo | cpp -dM | grep __GNUC_MINOR__ | awk '{print $3}'`
AC_MSG_RESULT([major $GCC_MAJOR, minor $GCC_MINOR. ])

dnl Looking for main libc headers
AC_MSG_RESULT([--- Looking for main libc headers ... ---               [[41%]]])
AC_HEADER_MAJOR

dnl We need to know what operating system hector will be compiled on.
AC_MSG_RESULT([--- Guessing host operating system ... ---              [[64%]]])
AC_CHECK_FILE(
	[[/etc/rc]],
	[AC_DEFINE([__HOST_OS_TYPE_FREEBSD__],[],[The operating system is FreeBSD.]) _HOST________=['FreeBSD. ---  ']],
	[AC_CHECK_FILE(
		[[/etc/debconf.conf]],
		[AC_DEFINE([__HOST_OS_TYPE_DEBIAN__],[],[The operating system is Debian.]) _HOST________=['Debian. ---   ']],
		[AC_CHECK_FILE(
			[[/etc/poldek/poldek.conf]],
			[AC_DEFINE([__HOST_OS_TYPE_PLD__],[],[The operating system is PLD.]) _HOST________=['PLD. ---      ']],
			[AC_CHECK_FILE(
				[[/etc/random-seed]],
				[AC_DEFINE([__HOST_OS_TYPE_SLACKWARE__],[],[The operating system is Slackware.]) _HOST________=['Slackware. ---']],
				[AC_MSG_ERROR([Can not recognize host operating system type!])]
			)]
		)]
	)]
)
AC_MSG_RESULT([--- Guessed host operating system is $_HOST________     [[70%]]])

dnl What special compiler flags we can set?
AC_MSG_RESULT([--- Guessing specific compiler/linker features ... ---  [[71%]]])
_FLAGS=''
CXXFLAGS_ORIG=$CXXFLAGS;
CXXFLAGS=["-Wextra"]
AC_MSG_CHECKING([does gcc support -Wextra])
RESULT=["no"]
AC_TRY_COMPILE([],[],[AC_SUBST([EXTRA_WARNINGS],[-Wextra])] [RESULT=["yes"]],
							[AC_SUBST([EXTRA_WARNINGS],[-W])])
AC_MSG_RESULT([$RESULT])
CXXFLAGS=["-Wshadow -Werror"]
AC_TRY_COMPILE([#include <pthread.h>],[],
							[AC_SUBST([WARN_SHADOW],[-Wshadow])
							_FLAGS=["-Wshadow"]],
							[AC_MSG_WARN([[Can not use -Wshadow!]])])
CXXFLAGS=["-pedantic-errors"]
AC_TRY_COMPILE([#include <stdlib.h>],[],
							[AC_SUBST([PEDANTIC_ERRORS],	[-pedantic-errors])
							_FLAGS=["$_FLAGS -pedantic-errors"]],
							[AC_MSG_WARN([[Can not use -pedantic-errors!]])])
CXXFLAGS=["$CXXFLAGS -Wl,--no-undefined -Wl,--fatal-warnings"]
AC_LINK_IFELSE([[extern int my_undefined_symbol;int main(void){return(my_undefined_symbol);}]],
				[CXXFLAGS=$CXXFLAGS_ORIG],
				[AC_SUBST([SPECIAL_LINKER_FLAGS],
					[[-Wl,--allow-shlib-undefined]])]
					_FLAGS=[['-Wl,--allow-shlib-undefined. ---']]
					CXXFLAGS=["$CXXFLAGS_ORIG -Wl,--allow-shlib-undefined"]
					[AC_MSG_WARN([[We must use -Wl,--allow-shlib-undefined!]])])
if test ["$_FLAGS"] = [""] ; then
	_FLAGS='none. ---                      [[76%]]'
fi
AC_MSG_RESULT([--- Guessed fetures are: $_FLAGS])

dnl Now we can look for all needed libraries.
CPPFLAGS=$CXXFLAGS;
AC_MSG_RESULT([--- Looking for needed libraries ... ---                [[77%]]])
AC_CHECK_LIB([ncurses],[initscr],
							[],[AC_MSG_ERROR([Can not continue without ncurses.])])
dnl WARNING!!! Names in c++ objects are _mangled_ so we need to use
dnl WARNING!!! this cryptic names here (I hope it is portable).
dnl UPDATE!: Now I know it is not portable across different versions of
dnl UPDATE!: compiller, so I used tricky hack with 'main'.
AC_CHECK_LIB([hcore],[yaal_hcore_main],
							[],[AC_MSG_ERROR([Can not continue without hcore.])])
AC_CHECK_LIB([hconsole],[yaal_hconsole_main],
							[],[AC_MSG_ERROR([Can not continue without hconsole.])])
AC_CHECK_LIB([tools],[yaal_tools_main],
							[],[AC_MSG_ERROR([Can not continue without tools.])])
AC_CHECK_LIB([dbwrapper],[yaal_dbwrapper_main],
							[],[AC_MSG_ERROR([Can not continue without dbwrapper.])])
AC_CHECK_LIB([hdata],[yaal_hdata_main],
							[],[AC_MSG_ERROR([Can not continue without hdata.])])
AC_MSG_RESULT([--- All needed libraries found. ---                     [[83%]]])

AC_MSG_RESULT([--- Looking for needed headers ... ---                  [[84%]]])
AC_CHECK_HEADERS([yaal/yaal.hxx],,
			[AC_MSG_ERROR([[Can not continue without yaal devel!]])])
AC_MSG_RESULT([--- Done looking for needed header, all found. ---      [[92%]]])

AH_TOP([/* $Id$ */

#ifndef __HECTOR_CONFIG_H
#define __HECTOR_CONFIG_H

/* Define core behavior here. */])
AH_BOTTOM([
#endif /* not __HECTOR_CONFIG_H */
])

dnl We have to set up proper compiler/linker flags.
AC_OUTPUT([Makefile.mk])
AC_MSG_RESULT([--- All ok, good luck! ---                     	       [[100%]]])
AC_MSG_RESULT([[Now you can type \`make'.]])


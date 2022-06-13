dnl config.m4 for extension pdo_mimer

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary.

dnl If your extension references something external, use 'with':

dnl PHP_ARG_WITH([pdo_mimer],
dnl   [for pdo_mimer support],
dnl   [AS_HELP_STRING([--with-pdo_mimer],
dnl     [Include pdo_mimer support])])

dnl Otherwise use 'enable':

PHP_ARG_ENABLE([pdo_mimer],
  [whether to enable pdo_mimer support],
  [AS_HELP_STRING([--enable-pdo_mimer],
    [Enable pdo_mimer support])],
  [no])

if test "$PHP_PDO_MIMER" != "no"; then
  dnl Write more examples of tests here...

  dnl Remove this code block if the library does not support pkg-config.
  dnl PKG_CHECK_MODULES([LIBFOO], [foo])
  dnl PHP_EVAL_INCLINE($LIBFOO_CFLAGS)
  dnl PHP_EVAL_LIBLINE($LIBFOO_LIBS, PDO_MIMER_SHARED_LIBADD)

  dnl If you need to check for a particular library version using PKG_CHECK_MODULES,
  dnl you can use comparison operators. For example:
  dnl PKG_CHECK_MODULES([LIBFOO], [foo >= 1.2.3])
  dnl PKG_CHECK_MODULES([LIBFOO], [foo < 3.4])
  dnl PKG_CHECK_MODULES([LIBFOO], [foo = 1.2.3])

  dnl Remove this code block if the library supports pkg-config.
  dnl --with-pdo_mimer -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/pdo_mimer.h"  # you most likely want to change this
  dnl if test -r $PHP_PDO_MIMER/$SEARCH_FOR; then # path given as parameter
  dnl   PDO_MIMER_DIR=$PHP_PDO_MIMER
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for pdo_mimer files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       PDO_MIMER_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$PDO_MIMER_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the pdo_mimer distribution])
  dnl fi

  dnl Remove this code block if the library supports pkg-config.
  dnl --with-pdo_mimer -> add include path
  dnl PHP_ADD_INCLUDE($PDO_MIMER_DIR/include)

  dnl Remove this code block if the library supports pkg-config.
  dnl --with-pdo_mimer -> check for lib and symbol presence
  dnl LIBNAME=PDO_MIMER # you may want to change this
  dnl LIBSYMBOL=PDO_MIMER # you most likely want to change this

  dnl If you need to check for a particular library function (e.g. a conditional
  dnl or version-dependent feature) and you are using pkg-config:
  dnl PHP_CHECK_LIBRARY($LIBNAME, $LIBSYMBOL,
  dnl [
  dnl   AC_DEFINE(HAVE_PDO_MIMER_FEATURE, 1, [ ])
  dnl ],[
  dnl   AC_MSG_ERROR([FEATURE not supported by your pdo_mimer library.])
  dnl ], [
  dnl   $LIBFOO_LIBS
  dnl ])

  dnl If you need to check for a particular library function (e.g. a conditional
  dnl or version-dependent feature) and you are not using pkg-config:
  dnl PHP_CHECK_LIBRARY($LIBNAME, $LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $PDO_MIMER_DIR/$PHP_LIBDIR, PDO_MIMER_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_PDO_MIMER_FEATURE, 1, [ ])
  dnl ],[
  dnl   AC_MSG_ERROR([FEATURE not supported by your pdo_mimer library.])
  dnl ],[
  dnl   -L$PDO_MIMER_DIR/$PHP_LIBDIR -lm
  dnl ])
  dnl
  dnl PHP_SUBST(PDO_MIMER_SHARED_LIBADD)

  dnl In case of no dependencies
  AC_DEFINE(HAVE_PDO_MIMER, 1, [ Have pdo_mimer support ])

  PHP_NEW_EXTENSION(pdo_mimer, pdo_mimer.c, $ext_shared)
fi

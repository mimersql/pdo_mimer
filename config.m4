dnl config.m4 for extension pdo_mimer

PHP_ARG_WITH([pdo-mimer],
  [for Mimer SQL support for PDO],
  [AS_HELP_STRING([[--with-pdo-mimer]],
    [PDO: Mimer SQL support])])

if test "$PHP_PDO_MIMER" != "no"; then

  if test "$PHP_PDO" = "no" && test "$ext_shared" = "no"; then
    AC_MSG_ERROR([PDO is not enabled! Add --enable-pdo to your configure line.])
  fi

  PHP_CHECK_PDO_INCLUDES
  PHP_ADD_LIBRARY(mimerapi,, PDO_MIMER_SHARED_LIBADD)
  PHP_SUBST(PDO_MIMER_SHARED_LIBADD)

  PHP_NEW_EXTENSION(pdo_mimer, pdo_mimer.c mimer_driver.c mimer_stmt.c , $ext_shared,,-I$pdo_cv_inc_path)
  PHP_ADD_EXTENSION_DEP(pdo_mimer, pdo)
fi

#!/bin/sh

is_force() {
  if [ -z "${1}" ] && [ "${FORCE_CONFIG}" = "1" ]; then
    return 0
  fi

  if [ "${1}" = "--force" ] || [ "${1}" = "-f" ]; then
    return 0
  fi

  return 1
}

is_help() {
  if [ "${1}" = "-h" ] || [ "${1}" = "--help" ] || [ "${1}" = "help" ]; then
    return 0
  fi

  return 1
}

check_config() {
  php_config="${1:-$PHP_CONFIG}"
  if [ -d "${php_config}" ]; then
    if [ -d "${php_config}/bin" ]; then php_config="${php_config}/bin"; fi
    if [ -x "${php_config}/php-config" ]; then php_config="${php_config}/php-config"; fi
  fi

  if [ -x "config.nice" ] && ! is_force; then
    ./config.nice
  elif [ -x "configure" ]; then
    if [ -z "${php_config}" ]; then
      ./configure
    else
      ./configure "--with-php-config=${php_config}"
    fi
  else
    phpize && check_config "${@}"
  fi
}


for arg in "${@}"; do :
  shift
  if is_help "${arg}"; then
    echo "usage: clean.sh [-f|--force] [/path/to/php/install/dir]"
    exit 0
  fi
  set -- "$@" "$arg"
done

for arg in "${@}"; do :
  shift
  if is_force "${arg}"; then
    if ! is_force; then
      phpize --clean
      FORCE_CONFIG=1
    fi
    continue
  fi

  set -- "$@" "$arg"
done

if ! [ -f "Makefile" ] || is_force; then
  check_config "${@}"
fi

test "Makefile" -ot "configure" && check_config "${@}"
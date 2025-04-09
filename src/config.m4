dnl config.m4 for extension pdo_tsurugi

PHP_ARG_WITH(
  [lib-dir],
  [for pdo_tsurugi configure option],
  [--with-lib-dir=PATH: Set custom path to tsubakuro-rust-ffi],
  no
)

PHP_CHECK_PDO_INCLUDES

AC_MSG_CHECKING([for tsubakuro-rust installation directory])

if test -n "$PHP_LIB_DIR" && test "$PHP_LIB_DIR" != "no"; then
  if test -d "$PHP_LIB_DIR"; then
    lib_dir="$PHP_LIB_DIR"
    AC_MSG_RESULT([found: $lib_dir])
  else
    AC_MSG_ERROR([directory not found])
  fi
else
  for prefix in /usr/local /opt; do
    for dir in $prefix/tsubakuro-rust*; do
      if test -d "$dir"; then
        lib_dir="$dir"
        break 2
      fi
    done
  done

  if test -z "$lib_dir"; then
    AC_MSG_ERROR([directory not found. Please use --with-lib-dir=PATH])
  else
    AC_MSG_RESULT([found: $lib_dir])
  fi
fi

PDO_TSURUGI_INCLUDE_DIR="$lib_dir/tsubakuro-rust-ffi"
PDO_TSURUGI_LIB_DIR="$PDO_TSURUGI_INCLUDE_DIR/target/release"

AC_MSG_CHECKING([for tsubakuro-rust-ffi.h])
if test ! -f "$PDO_TSURUGI_INCLUDE_DIR/tsubakuro-rust-ffi.h"; then
  AC_MSG_ERROR([header file not found: $PDO_TSURUGI_INCLUDE_DIR/tsubakuro-rust-ffi.h])
fi
AC_MSG_RESULT([found])

AC_MSG_CHECKING([for libtsubakuro_rust_ffi.so])
if test ! -f "$PDO_TSURUGI_LIB_DIR/libtsubakuro_rust_ffi.so"; then
  AC_MSG_ERROR([executable file not found: $PDO_TSURUGI_LIB_DIR/libtsubakuro_rust_ffi.so])
fi
AC_MSG_RESULT([found])

PHP_ADD_INCLUDE([$PDO_TSURUGI_INCLUDE_DIR])
PHP_ADD_LIBRARY_WITH_PATH([tsubakuro_rust_ffi], [$PDO_TSURUGI_LIB_DIR], [PDO_TSURUGI_SHARED_LIBADD])
PHP_SUBST([PDO_TSURUGI_SHARED_LIBADD])

PHP_NEW_EXTENSION([pdo_tsurugi], [pdo_tsurugi.c tsurugi_driver.c tsurugi_stmt.c pdo_tsurugi_placeholder.c], [$ext_shared])
PHP_ADD_EXTENSION_DEP([pdo_tsurugi], [date])
PHP_ADD_EXTENSION_DEP([pdo_tsurugi], [pdo])

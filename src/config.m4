PHP_ARG_WITH([pdo-tsurugi],
  [for TsurugiDB support for PDO],
  [AS_HELP_STRING([--without-pdo-tsurugi],
    [PDO: tsurugi support.])],
  [$PHP_PDO])

if test "$PHP_PDO_TSURUGI" != "no"; then
  PHP_CHECK_PDO_INCLUDES

  if test "$PHP_PDO_TSURUGI" = "yes"; then
    PDO_TSURUGI_INCLUDE_DIR="/opt/tsubakuro-rust/tsubakuro-rust-ffi"
  else
    PDO_TSURUGI_INCLUDE_DIR="$PHP_PDO_TSURUGI/tsubakuro-rust-ffi"
  fi


  PDO_TSURUGI_LIB_DIR="$PDO_TSURUGI_INCLUDE_DIR/target/release"

  if test ! -f "$PDO_TSURUGI_INCLUDE_DIR/tsubakuro-rust-ffi.h"; then
    AC_MSG_ERROR([Header file not found: $PDO_TSURUGI_INCLUDE_DIR/tsubakuro-rust-ffi.h])
  fi

  if test ! -f "$PDO_TSURUGI_LIB_DIR/libtsubakuro_rust_ffi.so"; then
    AC_MSG_ERROR([Executable file not found: $PDO_TSURUGI_LIB_DIR/libtsubakuro_rust_ffi.so])
  fi

  PHP_ADD_INCLUDE([$PDO_TSURUGI_INCLUDE_DIR])
  PHP_ADD_LIBRARY_WITH_PATH([tsubakuro_rust_ffi], [$PDO_TSURUGI_LIB_DIR], [PDO_TSURUGI_SHARED_LIBADD])
  PHP_SUBST([PDO_TSURUGI_SHARED_LIBADD])

  PHP_NEW_EXTENSION([pdo_tsurugi], [pdo_tsurugi.c tsurugi_driver.c tsurugi_stmt.c pdo_tsurugi_placeholder.c], [$ext_shared])
  PHP_ADD_EXTENSION_DEP([pdo_tsurugi], [pdo])
  PHP_ADD_MAKEFILE_FRAGMENT
fi

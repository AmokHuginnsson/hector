pushdef([MY_PACKAGE_VERSION],[0.0.0-esyscmd(date +%Y%m%d|tr -d '\n')])
AC_INIT([[hector]],[MY_PACKAGE_VERSION],[[amok@codestation.org]])
dnl It is expected that client applications have link to yaal's _aux.
dnl This macro has to be here due to primitivism of libtoolize.
AC_CONFIG_AUX_DIR([_aux])
m4_include([_aux/client-configure.m4])
YAAL_CONFIGURE_CLIENT_INIT(hector,MY_PACKAGE_VERSION)
YAAL_CONFIGURE_CLIENT_FINALIZE


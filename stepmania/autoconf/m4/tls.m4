# See if we have working TLS.  We only check to see if it compiles; we test
# to see if it works at runtime, since we can run binaries compiled with __thread
# on systems without TLS.
AC_DEFUN(SM_TLS,
[
    AC_MSG_CHECKING(for TLS)
    AC_TRY_COMPILE( [], [ static __thread int val; ], have_tls=yes,have_tls=no,have_tls=no )
    AC_MSG_RESULT($have_tls)
    if test "$have_tls" = "yes"; then
        AC_DEFINE([HAVE_TLS],[1],[Define if the complier supports __thread])
        AC_DEFINE([thread_local],[__thread],[Define to the compiler TLS keyword])
    fi
])


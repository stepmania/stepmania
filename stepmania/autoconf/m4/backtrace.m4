# See if we have a system backtrace().
AC_DEFUN(SM_FUNC_BACKTRACE,
[
    AC_MSG_CHECKING(for working backtrace)
    AC_TRY_RUN(
    [
	    #include <execinfo.h>
	    int main()
	    {
		    void *BacktracePointers[128];
		    return backtrace (BacktracePointers, 128) <= 0? 1:0;
	    }
    ], have_sys_backtrace=yes,have_sys_backtrace=no,have_sys_backtrace=no
    )
    AC_MSG_RESULT($have_sys_backtrace)
])


# See if we have a working backtrace_symbols.
AC_DEFUN(SM_FUNC_BACKTRACE_SYMBOLS,
[
    AC_MSG_CHECKING(for working backtrace_symbols())
    AC_TRY_RUN(
    [
	    #include <execinfo.h>
	    int main()
	    {
		    void *BacktracePointer=main;
		    return backtrace_symbols (&BacktracePointer, 1) == 0? 1:0;
	    }
    ], have_backtrace_symbols=yes,have_backtrace_symbols=no,have_backtrace_symbols=no
    )
    AC_MSG_RESULT($have_backtrace_symbols)
])


# To support backtraces in the crash handler, we need two things: a call to
# get backtrace pointers, and a way to convert them to symbols.
AC_DEFUN(SM_CHECK_CRASH_HANDLER,
[
    # See if we have a custom backtrace():
    case $host in
	*86-pc-linux*)
	    AC_DEFINE([BACKTRACE_METHOD],[X86_LINUX],[Define backtrace type])
	    have_backtrace=yes
	    ;;
    esac


    if test "$have_backtrace" != "yes"; then
	    # See if we have a system backtrace():
	    SM_FUNC_BACKTRACE

	    if test "$have_sys_backtrace" = "yes"; then
		    AC_DEFINE([BACKTRACE_METHOD],[BACKTRACE],[Define backtrace type])
		    have_backtrace=yes
	    fi
    fi

    # Do we have a libdl with dladdr?
    AC_SEARCH_LIBS(dladdr, [dl],
		   [AC_DEFINE([BACKTRACE_LOOKUP_METHOD],[DLADDR],[Define symbol lookup type])
		   have_symbol_lookup=yes])

    if test "$have_symbol_lookup" != "yes"; then
	    SM_FUNC_BACKTRACE_SYMBOLS

	    if test "$have_backtrace_symbols" = "yes"; then
		    AC_DEFINE([BACKTRACE_LOOKUP_METHOD],[BACKTRACE_SYMBOLS],[Define symbol lookup type])
		    have_symbol_lookup=yes
	    fi
    fi


    AC_MSG_CHECKING(for crash handler components)
    if test "$have_backtrace" = "yes" -a "$have_symbol_lookup" = "yes"; then
	    use_crash_handler=yes
	    AC_MSG_RESULT(ok)
    else
	    AC_MSG_RESULT(incomplete; crash handler will not be used)
    fi

    if test "$use_crash_handler" = "yes"; then
	    # All current symbol lookup methods need this flag.
	    LDFLAGS="$LDFLAGS -rdynamic"

	    # Check for libiberty.  This makes backtraces much nicer.
	    AC_CHECK_HEADER(libiberty.h, have_iberty_header=yes, have_iberty_header=no)
	    AC_SEARCH_LIBS(cplus_demangle, [iberty], have_iberty_lib=yes, have_iberty_lib=no)

	    if test $have_iberty_lib = "yes" -a $have_iberty_header = "yes"; then
		    AC_DEFINE(HAVE_LIBIBERTY, 1, [Liberty available])
	    else
		    echo
		    echo "*** Backtrace support has been detected, but libiberty is not installed."
		    echo "*** Libiberty will make crash reports for developers much easier to read,"
		    echo "*** and is strongly recommended."
		    echo
	    fi
    fi
])


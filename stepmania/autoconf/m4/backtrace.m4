# See if we have a working backtrace_symbols.
AC_DEFUN([SM_FUNC_BACKTRACE_SYMBOLS],
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

AC_DEFUN([SM_FUNC_CXA_DEMANGLE],
[
    # Check for abi::__cxa_demangle (gcc 3.1+)
    AC_MSG_CHECKING(for working cxa_demangle)
    AC_LANG_PUSH(C++)
    AC_TRY_RUN(
    [
	    #include <cxxabi.h>
	    #include <stdlib.h>
	    #include <typeinfo>

	    int main()
	    {
		    struct foo { } fum;
		    int status;
		    char *realname = abi::__cxa_demangle(typeid(fum).name(), 0, 0, &status);
		    free( realname );
		    return 0;
	    }
    ], have_cxa_demangle=yes,have_cxa_demangle=no,have_cxa_demangle=no
    )
    AC_LANG_POP(C++)
    AC_MSG_RESULT($have_cxa_demangle)
    if test "$have_cxa_demangle" = "yes"; then
	AC_DEFINE(HAVE_CXA_DEMANGLE, 1, [abi::__cxa_demangle available])
    fi
])    
	
# To support backtraces in the crash handler, we need two things: a call to
# get backtrace pointers, and a way to convert them to symbols.
AC_DEFUN([SM_CHECK_CRASH_HANDLER],
[
    # See if we have a custom backtrace():
    case $host in
    	# RedHat kindly changes the standard "pc-linux-gnu" to "redhat-linux-gnu",
	# soley for the purpose of breaking lots of scripts.  Ugh.
	*86-*-linux* | x86_64-*-linux* )
	    AC_DEFINE([BACKTRACE_METHOD_X86_LINUX],[1],[Define backtrace type])
	    AC_DEFINE([BACKTRACE_METHOD_TEXT],["x86 custom backtrace"],[Define backtrace type])
	    have_backtrace=yes
	    ;;
    esac

    # Do we have a libdl with dladdr?
    AC_SEARCH_LIBS(dladdr, [dl],
		   [AC_DEFINE([BACKTRACE_LOOKUP_METHOD_DLADDR],[1],[Define symbol lookup type])
		    AC_DEFINE([BACKTRACE_LOOKUP_METHOD_TEXT],["dladdr"],[Define backtrace type])
		   have_symbol_lookup=yes])

    if test "$have_symbol_lookup" != "yes"; then
	    SM_FUNC_BACKTRACE_SYMBOLS

	    if test "$have_backtrace_symbols" = "yes"; then
		    AC_DEFINE([BACKTRACE_LOOKUP_METHOD_BACKTRACE_SYMBOLS],[1],[Define symbol lookup type])
		    AC_DEFINE([BACKTRACE_LOOKUP_METHOD_TEXT],["backtrace_symbols"],[Define backtrace type])
		    have_symbol_lookup=yes
	    fi
    fi


    AC_MSG_CHECKING(for crash handler components)
    if test "$have_backtrace" = "yes" -a "$have_symbol_lookup" = "yes"; then
	    use_crash_handler=yes
	    AC_DEFINE([CRASH_HANDLER],1,[Define if using crash handler])
	    AC_MSG_RESULT(ok)
    else
	    AC_MSG_RESULT(incomplete; crash handler will not be used)
    fi

    if test "$use_crash_handler" = "yes"; then
	    # All current symbol lookup methods need this flag.
	    LDFLAGS="$LDFLAGS -rdynamic"

	    SM_FUNC_CXA_DEMANGLE
	    if test "$have_cxa_demangle" = "yes"; then
	    	have_demangle=yes
		AC_DEFINE([BACKTRACE_DEMANGLE_METHOD_TEXT],["cxa_demangle"],[Define demangle type])
   	    fi 
    
	    if test "$have_demangle" != "yes"; then
		    # Check for libiberty.
		    AC_CHECK_HEADER(libiberty.h, have_iberty_header=yes, have_iberty_header=no)
		    AC_SEARCH_LIBS(cplus_demangle, [iberty], have_iberty_lib=yes, have_iberty_lib=no)

		    if test $have_iberty_lib = "yes" -a $have_iberty_header = "yes"; then
			    AC_DEFINE(HAVE_LIBIBERTY, 1, [Liberty available])
			    have_demangle=yes
			    AC_DEFINE([BACKTRACE_DEMANGLE_METHOD_TEXT],["libiberty"],[Define demangle type])
		    fi
	    fi

	    if test "$have_demangle" != "yes"; then
		    echo
		    echo "*** Backtrace support has been detected, but libiberty is not installed."
		    echo "*** Libiberty will make crash reports for developers much easier to read,"
		    echo "*** and is strongly recommended."
		    echo
		    AC_DEFINE([BACKTRACE_DEMANGLE_METHOD_TEXT],["none"],[Define demangle type])
	    fi
    fi
])


AC_DEFUN([SM_X_WITH_OPENGL],
[
    AC_PATH_X
    
    XCFLAGS=
    XLIBS=

    if test "$no_x" != "yes"; then
	if test -n "$x_includes"; then
	    XCFLAGS="-I$x_includes"
	fi

	if test -n "$x_libraries"; then
	    XLIBS="-L$x_libraries -lX11"
	fi

	if test -n "$x_includes"; then
	    # See if we can compile X applications without using $XCFLAGS.
	    AC_MSG_CHECKING(if $XCFLAGS is really necessary)
	    AC_TRY_COMPILE( [#include <X11/Xos.h>], [],
		[XCFLAGS=
		AC_MSG_RESULT(no)],
		[AC_MSG_RESULT(yes)])
	fi

	# Check for libXtst.
	AC_CHECK_LIB(Xtst, XTestQueryExtension, 
	    XLIBS="$XLIBS -lXtst"
	    [AC_DEFINE(HAVE_LIBXTST, 1, [libXtst available])],
	    ,
	    [$XLIBS])
    fi

    AC_SUBST(XCFLAGS)
    AC_SUBST(XLIBS)

    # Check for libGL and libGLU.
    AC_CHECK_LIB(GL, glPushMatrix, XLIBS="$XLIBS -lGL",
	AC_MSG_ERROR([No OpenGL library could be found.]), [$XLIBS])
    AC_CHECK_LIB(GLU, gluGetString, XLIBS="$XLIBS -lGLU",
	AC_MSG_ERROR([No GLU library could be found.]), [$XLIBS])

    AC_SUBST(XLIBS)
])


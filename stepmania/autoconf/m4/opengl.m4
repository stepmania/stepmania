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
	AC_DEFINE(HAVE_X11, 1, [X11 libraries present])
    fi

	# Check for Xrandr
	# Can someone fix this for me? This is producing bizarre warnings from
	# configure... I have no clue what I'm doing -Ben
	AC_CHECK_LIB(Xrandr, XRRSizes, have_xrandr=yes, have_xrandr=no)
	AC_CHECK_HEADER(X11/extensions/Xrandr.h, have_xrandr_header=yes, have_xrandr_header=no)

	if test "$have_xrandr_header" = "no"; then
		have_xrandr=no
	fi

	if test "$have_xrandr" = "no"; then
		echo "*** Direct X11 support needs Xrandr libraries and headers."
		echo "*** Couldn't find needed headers. Continuing without X11 backend."
		$no_x = yes
	else
		LIBS="$LIBS -lXrandr"
	fi

	AM_CONDITIONAL(HAVE_X11, test "$no_x" != "yes")

    AC_SUBST(XCFLAGS)
    AC_SUBST(XLIBS)

    # Check for libGL and libGLU.
    AC_CHECK_LIB(GL, glPushMatrix, XLIBS="$XLIBS -lGL",
	AC_MSG_ERROR([No OpenGL library could be found.]), [$XLIBS])
    AC_CHECK_LIB(GLU, gluGetString, XLIBS="$XLIBS -lGLU",
	AC_MSG_ERROR([No GLU library could be found.]), [$XLIBS])

    AC_SUBST(XLIBS)
])


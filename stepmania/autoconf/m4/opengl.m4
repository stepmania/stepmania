AC_DEFUN(SM_X_WITH_OPENGL,
[
    AC_PATH_X
    AC_PATH_XTRA
    
    # See if we can compile X applications without using X_CFLAGS.
    AC_MSG_CHECKING(if $X_CFLAGS is really necessary)
    AC_TRY_COMPILE( [#include <X11/Xos.h>], [],
	[X_CFLAGS=
	AC_MSG_RESULT(no)],
	[AC_MSG_RESULT(yes)])

    XLIBS="$X_LIBS $X_EXTRA_LIBS $X_PRE_LIBS -lX11"

    # Check for libXtst.
    AC_CHECK_LIB(Xtst, XTestQueryExtension, 
	XLIBS="$XLIBS -lXtst"
	[AC_DEFINE(HAVE_LIBXTST, 1, [libXtst available])],
	,
	[$XLIBS])

    # Check for libGL and libGLU.
    AC_CHECK_LIB(GL, glPushMatrix, XLIBS="$XLIBS -lGL",
	AC_MSG_ERROR([No OpenGL library could be found.]), [$XLIBS])
    AC_CHECK_LIB(GLU, gluGetString, XLIBS="$XLIBS -lGLU",
	AC_MSG_ERROR([No GLU library could be found.]), [$XLIBS])

    AC_SUBST(XLIBS)
])


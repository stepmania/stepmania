AC_DEFUN(SM_OPENGL,
[
    GL_LIBS=
    if test "$GL_LIBS" = ""; then
	AC_CHECK_LIB(GL, glPushMatrix, have_gl=yes)
	AC_CHECK_LIB(GLU, gluGetString, have_glu=yes, , [-lGL])
	if test "$have_gl" = "yes" -a "$have_glu" = "yes"; then
	    GL_LIBS="-lGL -lGLU"
	fi
    fi

    # Some broken systems need us to link to X explicitly because
    # the GL library needs it, instead of doing it automatically.
    if test "$GL_LIBS" = ""; then
	AC_PATH_X
	AC_PATH_XTRA

	oldLIBS=$LIBS
	EXTRALIBS="$X_LIBS $X_EXTRA_LIBS $X_PRE_LIBS -lX11"
	LIBS="$LIBS $EXTRALIBS"

	if test "$GL_LIBS" = ""; then
	    AC_CHECK_LIB(GL, glPushMatrix, have_gl=yes)
	    AC_CHECK_LIB(GLU, gluGetString, have_glu=yes, , [-lGL])
	    if test "$have_gl" = "yes" -a "$have_glu" = "yes"; then
		GL_LIBS="-lGL -lGLU"
	    fi
	fi

	if test "$GL_LIBS" != ""; then
	    GL_LIBS="$GL_LIBS $EXTRALIBS"
	fi
	LIBS=$oldLIBS
	oldLIBS=
	EXTRALIBS=
    fi

    if test "$GL_LIBS" = ""; then
    	if test "$have_gl" != "yes"; then
	    AC_MSG_ERROR([No OpenGL library could be found.])
	else
	    AC_MSG_ERROR([No GLU library could be found.])
	fi
    fi
    AC_SUBST(GL_LIBS)
])


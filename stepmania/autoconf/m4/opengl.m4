AC_DEFUN(SM_OPENGL,
[
    AC_ARG_ENABLE(mesa, AC_HELP_STRING([--enable-mesa], [Use libMesaGL]), enable_mesa=$enableval, enable_mesa=no)

    GL_LIBS=
    if test "$enable_mesa" = "yes"; then
	AC_CHECK_LIB(MesaGL, glPushMatrix, have_mesagl=yes)
	AC_CHECK_LIB(MesaGLU, gluGetString, have_mesaglu=yes)
	if test "$have_mesagl" = "yes" -a "$have_mesaglu" = "yes"; then
	    GL_LIBS="-lMesaGL -lMesaGLU"
	fi
    fi

    if test "$GL_LIBS" = ""; then
	AC_CHECK_LIB(GL, glPushMatrix, have_gl=yes)
	AC_CHECK_LIB(GLU, gluGetString, have_glu=yes)
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

	if test "$enable_mesa" = "yes"; then
	    AC_CHECK_LIB(MesaGL, glPushMatrix, have_mesagl=yes)
	    AC_CHECK_LIB(MesaGLU, gluGetString, have_mesaglu=yes)
	    if test "$have_mesagl" = "yes" -a "$have_mesaglu" = "yes"; then
		GL_LIBS="-lMesaGL -lMesaGLU"
	    fi
	fi

	if test "$GL_LIBS" = ""; then
	    AC_CHECK_LIB(GL, glPushMatrix, have_gl=yes)
	    AC_CHECK_LIB(GLU, gluGetString, have_glu=yes)
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
	AC_MSG_ERROR([No OpenGL libraries could be found.  If you want
to search for MesaGL, pass the "--enable-mesa" flag to configure.])
    fi
    AC_SUBST(GL_LIBS)
])




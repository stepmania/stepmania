AC_DEFUN(SM_OPENGL,
[
    AC_ARG_ENABLE(mesa, AC_HELP_STRING([--enable-mesa], [Use libMesaGL]), enable_mesa=$enableval, enable_mesa=no)

    GL_LIBS=
    if test "$enable_mesa" = "yes"; then
	AC_CHECK_LIB(MesaGL, glPushMatrix, GL_LIBS=-lMesaGL)
    fi

    if test "$GL_LIBS" = ""; then
	AC_CHECK_LIB(GL, glPushMatrix, GL_LIBS="-lGL")
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
	    AC_CHECK_LIB(MesaGL, glPushMatrix, GL_LIBS="-lMesaGL $EXTRALIBS")
	fi
	if test "$GL_LIBS" = ""; then
	    AC_CHECK_LIB(GL, glPushMatrix, GL_LIBS="-lGL $EXTRALIBS")
	fi
	LIBS=$oldLIBS
	oldLIBS=
	EXTRALIBS=
    fi

    AC_SUBST(GL_LIBS)
    if test "$GL_LIBS" = ""; then
	AC_MSG_ERROR(["No OpenGL libraries could be found.  If you want
to search for MesaGL, pass the "--enable-mesa" flag to configure.])
    fi
])




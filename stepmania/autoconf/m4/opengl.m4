AC_DEFUN(SM_OPENGL,
[
    AC_ARG_ENABLE(mesa, AC_HELP_STRING([--enable-mesa], [Use libMesaGL]), enable_mesa=$enableval, enable_mesa=no)

    GL_LIBS=
    if test "$enable_mesa" = "yes"; then
	AC_CHECK_LIB(MesaGL, glPushMatrix, GL_LIBS=-lMesaGL)
    fi

    if test -z "$GL_LIBS"; then
	AC_CHECK_LIB(GL, glPushMatrix, GL_LIBS="-lGL")
    fi

    AC_SUBST(GL_LIBS)
    if test -z $GL_LIBS; then
	AC_MSG_ERROR(["No OpenGL libraries could be found.  If you want
to search for MesaGL, pass the "--enable-mesa" flag to configure.])
    fi
])




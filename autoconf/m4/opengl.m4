
AC_DEFUN([SM_OPENGL],
[
	AC_LANG_PUSH(C)

	AC_CHECK_HEADER("GL/gl.h", ,
	AC_MSG_ERROR("OpenGL headers not found."), [$GL_INCLUDES])
	
	GL_LIBS=""
	
	AX_CHECK_LIB_USING_HEADER(GL, 'glBegin(GL_POINTS)', GL/gl.h, [GL_LIBS="-lGL"])
	
	if test "$GL_LIBS" = ""; then
	AX_CHECK_LIB_USING_HEADER(opengl32, 'glBegin(GL_POINTS)', GL/gl.h, [GL_LIBS="-lopengl32"])
	fi
	if test "$GL_LIBS" = ""; then
			AC_MSG_ERROR("No usable OpenGL library found.")
	fi
	
	AC_SUBST(GL_LIBS)
])
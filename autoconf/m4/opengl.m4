
AC_DEFUN([SM_OPENGL],
[
	AC_LANG_PUSH(C)

	AC_CHECK_HEADER("GL/gl.h", ,
	AC_MSG_ERROR("OpenGL headers not found."), [$GL_INCLUDES])
	
	GL_LIBS=""
	
	AX_CHECK_LIB_USING_HEADER(GL, glBegin(GL_POINTS), GL/gl.h, [GL_LIBS="-lGL"])
	
	if test "$GL_LIBS" = ""; then
		AX_CHECK_LIB_USING_HEADER(opengl32, glBegin(GL_POINTS), GL/gl.h, [GL_LIBS="-lopengl32"])
	fi
	
	if test "$GL_LIBS" = ""; then
		AC_MSG_ERROR("No usable OpenGL library found.")
	fi
	
	AX_CHECK_LIB_USING_HEADER(GLU, gluGetString(GLU_VERSION), GL/glu.h, [GLU_LIBS="-lGLU"])
	
	if test "$GLU_LIBS" = ""; then
		AX_CHECK_LIB_USING_HEADER(glu32, gluGetString(GLU_VERSION), GL/glu.h, [GLU_LIBS="-lglu32"])
	fi
	
	if test "$GLU_LIBS" = ""; then
		AC_MSG_ERROR("No usable GLU library found.")
	fi
	
	AX_CHECK_LIB_USING_HEADER(GLEW, glewInit(), GL/glew.h, [GLEW_LIBS="$LIBS -lGLEW"])

	if test "$GLEW_LIBS" = ""; then
		AX_CHECK_LIB_USING_HEADER(glew32, glewInit(), GL/glew.h, [GLEW_LIBS="$LIBS -lglew32"], , [$GL_LIBS])
	fi
	
	if test "$GLEW_LIBS" = ""; then
		AC_DEFINE([GLEW_STATIC], [], [Tell GLEW it is a static library.])
		AX_CHECK_LIB_USING_HEADER(glew32, glewInit(), GL/glew.h, [GLEW_LIBS="$LIBS -lglew32"], , [$GL_LIBS])
	fi
	
	if test "$GLEW_LIBS" = ""; then
		AC_MSG_ERROR("No usable version of GLEW found.")
	fi

	AC_CHECK_DECL(GLEW_VERSION_2_1, , AC_MSG_ERROR("GLEW 1.3.5 or newer is required."), [#include <GL/glew.h>])
	
	GL_LIBS="$GL_LIBS $GLU_LIBS $GLEW_LIBS"
])
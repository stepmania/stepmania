/*
 * This is a demonstration 3d object.  I created this to give some basic
 * code that sets up normal 3d object rendering.  (The Space code sets
 * up perspective and then renders 2d, which is different.)  This actor
 * should be positioned just like any other actor, calling SetX and SetY
 * in the parent; that position will become the center of the frustum.
 */
#include "stdafx.h"

#include "Sample3dObject.h"

#include "RageDisplay.h"
#include <math.h>

#include "SDL.h"
#include "SDL_opengl.h"

Sample3dObject::Sample3dObject()
{
	rot = 0;
}

void Sample3dObject::Update( float fDeltaTime )
{
	rot += fDeltaTime * 360.f;
	rot = float(fmod(rot, 360.f));
}

/* One note about rendering: you get a 640x480 viewport to render in, centered
 * on the location the actor is set to.  That means that if the center isn't
 * the center of the real screen, part of the view is offscreen and there's
 * a portion of the screen you can't render to.  If you want to render things
 * like fullscreen background animations (characters), call SetX(CENTER_X) on
 * and SetY(CENTER_Y) on the object so it gets the whole screen.
 */
void Sample3dObject::DrawPrimitives()
{
	DISPLAY->FlushQueue(); /* do this before rendering directly */

	/* If this is a sub-object (3d object within a 3d object), this won't
	 * actually do anything: */
	DISPLAY->EnterPerspective(60);

	DISPLAY->SetTexture(NULL);
	DISPLAY->EnableZBuffer();

	glColor4f(1,1,1,1);
	glPushMatrix();

	/* By default, 0,0,0 is the center of the object, at the location the actor
	 * is set to.  We have to translate away from the viewpoint (negative Z)
	 * to get away from the viewer (and in front of the near clip plane). */
	glTranslatef (0,0,-5);

	{ /* Standard ugly OpenGL lighting stuff. */
		GLfloat mat_ambient[] = { 1.0, 1.0, 1.0, 1.0 };
		GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
		GLfloat light_position[] = { 0.0, 0.0, 0.0, 1.0 };
		GLfloat lm_ambient[] = { 0.3f, 0.3f, 0.3f, 1.0f };

		glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
		glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
		glMaterialf(GL_FRONT, GL_SHININESS, 100.0);
		glLightfv(GL_LIGHT0, GL_POSITION, light_position);
		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lm_ambient);

		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
 	}
	
	GLfloat sphere_diffuse[] = { 0.7f, 0.0f, 0.7f, 1.0f };
	glMaterialfv(GL_FRONT, GL_DIFFUSE, sphere_diffuse);
	GLUquadricObj *quadObj = gluNewQuadric();
	gluQuadricDrawStyle(quadObj, GLU_FILL);
	gluQuadricNormals(quadObj, GLU_SMOOTH);

	glRotatef (rot, 0.0, 1.0, 0.0);

	glPushMatrix();
	glTranslatef (1,0,0);
	gluSphere(quadObj, .25, 8, 16);
	glPopMatrix();

	glPushMatrix();
	glTranslatef (-1,0,0);
	gluSphere(quadObj, .35, 8, 16);
	glPopMatrix();

	gluDeleteQuadric(quadObj);
	glPopMatrix();

	DISPLAY->ExitPerspective();
	DISPLAY->DisableZBuffer();

	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);
}

/*
 * Copyright (c) 2002 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard
 */

#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: ScreenSandbox.h

 Desc: Area for testing.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
	Glenn Maynard (OpenGL Code)
	Lance Gilbert (OpenGL/Usability Modifications)
-----------------------------------------------------------------------------
*/

#include "stdafx.h"

#include "ScreenSandbox.h"

#include "RageDisplay.h"
#include <math.h>

#include "SDL.h"
#include "SDL_opengl.h"
#include "ScreenSandbox.h"
#include "ScreenManager.h"
#include "RageMusic.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "Quad.h"
#include "ThemeManager.h"
#include "RageNetwork.h"


ScreenSandbox::ScreenSandbox()
{	
	rot = 0;
	tX = 0;
	tY = 0;
	tZ = 0;
}


void ScreenSandbox::Update( float fDeltaTime )
{
	rot += fDeltaTime * 360.f;
	rot = float(fmod(rot, 360.f));
}

void ScreenSandbox::DrawPrimitives()
{
	DISPLAY->FlushQueue(); /* do this before rendering directly */

	/* If this is a sub-object (3d object within a 3d object), this won't
	 * actually do anything: */
	DISPLAY->EnterPerspective(60);

	DISPLAY->SetTexture(NULL);
	DISPLAY->EnableZBuffer();

	glColor4f(1,1,1,1);
	glPushMatrix();
	SetX(CENTER_X);
	SetY(CENTER_Y);

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
	gluSphere(quadObj, (tX+.25), (tY+8), (tZ+16));
	glPopMatrix();

	glPushMatrix();
	glTranslatef (-1,0,0);
	gluSphere(quadObj, (tX+.35), (tY+8), (tZ+16));
	glPopMatrix();

	gluDeleteQuadric(quadObj);
	glPopMatrix();

	DISPLAY->ExitPerspective();
	DISPLAY->DisableZBuffer();

	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);
}

void ScreenSandbox::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	if( type != IET_FIRST_PRESS )
		return;	// ignore

	switch( DeviceI.device)
	{
	case DEVICE_KEYBOARD:
		switch( DeviceI.button )
		{
		case DIK_LEFT:
			glTranslatef ((tX-.1),0,0);
			break;
		case DIK_RIGHT:
			glTranslatef ((tX+.1),0,0);
			break;
		case DIK_UP:
			glTranslatef (0,(tY-.1),0);
			break;
		case DIK_DOWN:
			glTranslatef (0,(tY+.1),0);
			break;
		case DIK_T: 
			{
				SDL_Event *event;
				event = (SDL_Event *) malloc(sizeof(event));
				event->type = SDL_QUIT;
				SDL_PushEvent(event);
			}
		case DIK_ESCAPE: 
			{
			SCREENMAN->SetNewScreen( "ScreenTitleMenu" );
			}
		}

	}

}
void ScreenSandbox::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_DoneClosingWipingLeft:
		break;
	case SM_DoneClosingWipingRight:
		break;
	case SM_DoneOpeningWipingLeft:
		break;
	case SM_DoneOpeningWipingRight:
		break;
	}
}

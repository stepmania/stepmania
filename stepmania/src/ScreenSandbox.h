#ifndef SCREENSANDBOX_H
#define SCREENSANDBOX_H
/*
-----------------------------------------------------------------------------
 Class: ScreenSandbox

 Desc: Area for testing.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Screen.h"
#include "Model.h"
#include "Quad.h"
#include "Transition.h"

class ScreenSandbox : public Screen
{
public:
	ScreenSandbox();

	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	virtual void HandleScreenMessage( const ScreenMessage SM );

	void MenuLeft( PlayerNumber pn );
	void MenuRight( PlayerNumber pn );

	void Update(float f);
	Model m_model;
	Quad m_quad;
	Transition	m_In;
	Transition	m_Out;
};


#endif

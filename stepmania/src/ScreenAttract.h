#ifndef ScreenAttract_H
#define ScreenAttract_H
/*
-----------------------------------------------------------------------------
 Class: ScreenAttract

 Desc: Base class for all attraction screens.  This class handles input
	and coin logic

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Screen.h"
#include "Sprite.h"
#include "BitmapText.h"
#include "TransitionFade.h"
#include "RandomSample.h"
#include "BGAnimation.h"
#include "RageTimer.h"


class ScreenAttract : public Screen
{
public:
	ScreenAttract();
	virtual ~ScreenAttract();

	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	virtual void FirstUpdate();
	virtual void Update( float fDelta );
	virtual void HandleScreenMessage( const ScreenMessage SM );

protected:

	virtual CString	GetMetricName() = 0;	// used to loop up theme metrics
	virtual CString	GetElementName() = 0;	// used to loop up theme elements 

	BGAnimation			m_Background;
	TransitionFade		m_Fade;
	RandomSample		m_soundStart;
};



#endif
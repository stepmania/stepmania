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
#include "RageSound.h"


class ScreenAttract : public Screen
{
public:
	ScreenAttract( CString sMetricName, CString sElementName );
	virtual ~ScreenAttract();

	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	virtual void Update( float fDelta );
	virtual void HandleScreenMessage( const ScreenMessage SM );

protected:

	CString		m_sMetricName;	// used to loop up theme metrics
	CString		m_sElementName;	// used to loop up theme elements

	BGAnimation			m_Background;
	TransitionFade		m_Fade;
	RandomSample		m_soundStart;
};



#endif
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
#include "TransitionBGAnimation.h"
#include "RandomSample.h"
#include "BGAnimation.h"
#include "RageTimer.h"
#include "RageSound.h"


class ScreenAttract : public Screen
{
public:
	ScreenAttract( CString sClassName );
	virtual ~ScreenAttract();

	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	static void AttractInput( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI, bool bTransitioning );
	virtual void Update( float fDelta );
	virtual void HandleScreenMessage( const ScreenMessage SM );

protected:

	BGAnimation				m_Background;
	TransitionBGAnimation	m_In;
	TransitionBGAnimation	m_Out;
	RandomSample			m_soundStart;
};



#endif

/*
-----------------------------------------------------------------------------
 Class: ScreenDemonstration

 Desc: Base class for all attraction screens.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenGameplay.h"
#include "Sprite.h"


class ScreenDemonstration : public ScreenGameplay
{
public:
	ScreenDemonstration();
	~ScreenDemonstration();

	virtual void FirstUpdate();
	virtual void Update( float fDeltaTime );
	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	virtual void HandleScreenMessage( const ScreenMessage SM );

protected:
	Sprite		m_sprDemonstrationOverlay;
	Sprite		m_sprDemonstrationBlink;


};




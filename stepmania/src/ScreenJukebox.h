/*
-----------------------------------------------------------------------------
 Class: ScreenJukebox

 Desc: Like ScreenDemonstration, Plays whole songs continuously.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenGameplay.h"
#include "Sprite.h"


class ScreenJukebox : public ScreenGameplay
{
public:
	ScreenJukebox();
	~ScreenJukebox();

	virtual void Update( float fDeltaTime );
	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	virtual void HandleScreenMessage( const ScreenMessage SM );

protected:

};




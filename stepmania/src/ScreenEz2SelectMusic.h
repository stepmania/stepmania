#ifndef SCREENEZ2SELECTMUSIC_H
#define SCREENEZ2SELECTMUSIC_H
/*
-----------------------------------------------------------------------------
 Class: ScreenSandbox

 Desc: Area for testing.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Screen.h"
#include "Sprite.h"
#include "BitmapText.h"
#include "TransitionStarWipe.h"
#include "MenuElements.h"
#include "TipDisplay.h"
#include "RageSoundStream.h"
#include "MusicBannerWheel.h"
#include "MenuElements.h"

class ScreenEz2SelectMusic : public Screen
{
public:
	ScreenEz2SelectMusic();
	virtual void DrawPrimitives();

	virtual void Update( float fDeltaTime );
	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	virtual void HandleScreenMessage( const ScreenMessage SM );
protected:
	MusicBannerWheel			m_MusicBannerWheel;
	MenuElements		m_Menu;

};


#endif

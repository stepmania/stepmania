#pragma once
/*
-----------------------------------------------------------------------------
 Class: MusicStatusDisplay

 Desc: A little graphic to the left of the song's text banner in the MusicWheel.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


#include "Sprite.h"


enum MusicStatusDisplayType { TYPE_NEW, TYPE_NONE, TYPE_CROWN1, TYPE_CROWN2, TYPE_CROWN3 };

class MusicStatusDisplay : public Sprite
{
public:
	MusicStatusDisplay();

	void SetType( MusicStatusDisplayType msdt );

	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();

protected:

	enum MusicStatusDisplayType m_MusicStatusDisplayType;
};

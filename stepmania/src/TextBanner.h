#pragma once
/*
-----------------------------------------------------------------------------
 Class: TextBanner

 Desc: Shows song title, subtitle, and artist/.  Displayed on the MusicWheel.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ActorFrame.h"
#include "BitmapText.h"
class Song;

class TextBanner : public ActorFrame
{
public:
	TextBanner();
	bool LoadFromSong( Song* pSong );

private:
	BitmapText	m_textTitle, m_textSubTitle, m_textArtist;
};

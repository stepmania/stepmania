#ifndef TEXTBANNER_H
#define TEXTBANNER_H
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
	void LoadFromSong( const Song* pSong );
	void LoadFromString( 
		CString sDisplayTitle, CString sTranslitTitle, 
		CString sDisplaySubTitle, CString sTranslitSubTitle, 
		CString sDisplayArtist, CString sTranslitArtist );

private:

	BitmapText	m_textTitle, m_textSubTitle, m_textArtist;
	//Sprite	m_textTitle, m_textSubTitle, m_textArtist;
};

#endif

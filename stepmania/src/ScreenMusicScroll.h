/*
-----------------------------------------------------------------------------
 File: ScreenMusicScroll.h

 Desc: Music plays and song names scroll across the screen.

 Copyright (c) 2001-2003 by the person(s) listed below.  All rights reserved.
	Chris Danford
	Glenn Maynard
-----------------------------------------------------------------------------
*/

#include "BitmapText.h"
#include "ScreenAttract.h"

class ScreenMusicScroll : public ScreenAttract
{
public:
	ScreenMusicScroll( CString sName );
	virtual ~ScreenMusicScroll();

	virtual void Update( float fDeltaTime );

	virtual void HandleScreenMessage( const ScreenMessage SM );

private:
	vector<BitmapText *> m_textLines;
};



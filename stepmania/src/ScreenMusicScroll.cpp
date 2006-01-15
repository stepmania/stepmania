#include "global.h"
#include "ScreenMusicScroll.h"
#include "SongManager.h"
#include "GameSoundManager.h"
#include "ThemeManager.h"
#include "AnnouncerManager.h"
#include "song.h"
#include "SongUtil.h"
#include "ScreenDimensions.h"


#define SCROLL_DELAY		THEME->GetMetricF("ScreenMusicScroll","ScrollDelay")
#define SCROLL_SPEED		THEME->GetMetricF("ScreenMusicScroll","ScrollSpeed")
#define TEXT_ZOOM			THEME->GetMetricF("ScreenMusicScroll","TextZoom")


const CString CREDIT_LINES[] = 
{
""
};
const unsigned NUM_CREDIT_LINES = sizeof(CREDIT_LINES) / sizeof(CString);


REGISTER_SCREEN_CLASS_NEW( ScreenMusicScroll );

void ScreenMusicScroll::Init()
{
	ScreenAttract::Init();

	vector<Song*> arraySongs;
	SONGMAN->GetSongs( arraySongs );
	SongUtil::SortSongPointerArrayByTitle( arraySongs );
	
	for( unsigned i=0; i < arraySongs.size(); i++ )
	{
		BitmapText *bt = new BitmapText;
		m_textLines.push_back(bt);
		
		Song* pSong = arraySongs[i];
		bt->LoadFromFont( THEME->GetPathF("ScreenMusicScroll","titles") );
		bt->SetText( pSong->GetDisplayFullTitle(), pSong->GetTranslitFullTitle() );
		bt->SetDiffuse( SONGMAN->GetSongColor(pSong) );
		bt->SetZoom( TEXT_ZOOM );

		this->AddChild( bt );
	}

//	for( i=0; i<min(NUM_CREDIT_LINES, MAX_CREDIT_LINES); i++ )
//	{
//		m_textLines[m_iNumLines].LoadFromFont( THEME->GetPathF("ScreenMusicScroll, "titles") );
//		m_textLines[m_iNumLines].SetText( CREDIT_LINES[i] );
//		m_textLines[m_iNumLines].SetZoom( TEXT_ZOOM );
//
//		m_iNumLines++;
//	}

	for( unsigned i=0; i<m_textLines.size(); i++ )
	{
		m_textLines[i]->SetXY( SCREEN_CENTER_X, SCREEN_BOTTOM + 40 );
		m_textLines[i]->BeginTweening( SCROLL_DELAY * i );
		m_textLines[i]->BeginTweening( 2.0f*SCROLL_SPEED );
		m_textLines[i]->SetXY( SCREEN_CENTER_X, SCREEN_TOP - 40 );	
	}
	
	this->MoveToTail( &m_In );		// put it in the back so it covers up the stuff we just added
	this->MoveToTail( &m_Out );		// put it in the back so it covers up the stuff we just added

	this->ClearMessageQueue( SM_BeginFadingOut );	// ignore ScreenAttract's SecsToShow
	this->PostScreenMessage( SM_BeginFadingOut, 0.2f * m_textLines.size() + 3.0f );

	SOUND->PlayOnceFromDir( ANNOUNCER->GetPathTo("music scroll") );
}

ScreenMusicScroll::~ScreenMusicScroll()
{
	for( unsigned i=0; i<m_textLines.size(); i++ )
		delete m_textLines[i];
}

void ScreenMusicScroll::Update( float fDeltaTime )
{
	ScreenAttract::Update( fDeltaTime );

	for( unsigned i=0; i<m_textLines.size(); i++ )
	{
		const bool shown = ( m_textLines[i]->GetY() > SCREEN_TOP-20  &&
					   m_textLines[i]->GetY() < SCREEN_BOTTOM+20 );
		m_textLines[i]->SetHidden( !shown );
	}
}	

/*
 * (c) 2001-2004 Chris Danford, Glenn Maynard
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

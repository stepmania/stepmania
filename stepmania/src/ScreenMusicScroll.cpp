#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenMusicScroll

 Desc: See header.

 Copyright (c) 2001-2003 by the person(s) listed below.  All rights reserved.
	Chris Danford
	Glenn Maynard
-----------------------------------------------------------------------------
*/


#include "ScreenMusicScroll.h"
#include "SongManager.h"
#include "RageSounds.h"
#include "ThemeManager.h"
#include "AnnouncerManager.h"
#include "song.h"
#include "SongUtil.h"


#define SCROLL_DELAY		THEME->GetMetricF("ScreenMusicScroll","ScrollDelay")
#define SCROLL_SPEED		THEME->GetMetricF("ScreenMusicScroll","ScrollSpeed")
#define TEXT_ZOOM			THEME->GetMetricF("ScreenMusicScroll","TextZoom")


const CString CREDIT_LINES[] = 
{
""
};
const unsigned NUM_CREDIT_LINES = sizeof(CREDIT_LINES) / sizeof(CString);


ScreenMusicScroll::ScreenMusicScroll( CString sClassName ) : ScreenAttract( sClassName )
{
	vector<Song*> arraySongs;
	SONGMAN->GetSongs( arraySongs );
	SongUtil::SortSongPointerArrayByTitle( arraySongs );
	
	unsigned i;
	for( i=0; i < arraySongs.size(); i++ )
	{
		BitmapText *bt = new BitmapText;
		m_textLines.push_back(bt);
		
		Song* pSong = arraySongs[i];
		bt->LoadFromFont( THEME->GetPathToF("ScreenMusicScroll titles") );
		bt->SetText( pSong->GetFullDisplayTitle(), pSong->GetFullTranslitTitle() );
		bt->SetDiffuse( SONGMAN->GetSongColor(pSong) );
		bt->SetZoom( TEXT_ZOOM );

		this->AddChild( bt );
	}

//	for( i=0; i<min(NUM_CREDIT_LINES, MAX_CREDIT_LINES); i++ )
//	{
//		m_textLines[m_iNumLines].LoadFromFont( THEME->GetPathToF("ScreenMusicScroll titles") );
//		m_textLines[m_iNumLines].SetText( CREDIT_LINES[i] );
//		m_textLines[m_iNumLines].SetZoom( TEXT_ZOOM );
//
//		m_iNumLines++;
//	}

	for( i=0; i<m_textLines.size(); i++ )
	{
		m_textLines[i]->SetXY( CENTER_X, SCREEN_BOTTOM + 40 );
		m_textLines[i]->BeginTweening( SCROLL_DELAY * i );
		m_textLines[i]->BeginTweening( 2.0f*SCROLL_SPEED );
		m_textLines[i]->SetXY( CENTER_X, SCREEN_TOP - 40 );	
	}
	
	this->MoveToTail( &m_In );		// put it in the back so it covers up the stuff we just added
	this->MoveToTail( &m_Out );		// put it in the back so it covers up the stuff we just added

	this->ClearMessageQueue( SM_BeginFadingOut );	// ignore ScreenAttract's SecsToShow
	this->PostScreenMessage( SM_BeginFadingOut, 0.2f * i + 3.0f );

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


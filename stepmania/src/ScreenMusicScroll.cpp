#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenMusicScroll

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


#include "ScreenMusicScroll.h"
#include "ScreenManager.h"
#include "PrefsManager.h"
#include "ThemeManager.h"
#include "ScreenSelectMusic.h"
#include "ScreenTitleMenu.h"
#include "GameManager.h"
#include "RageLog.h"
#include "GameConstantsAndTypes.h"
#include "SongManager.h"


const ScreenMessage SM_StartFadingOut	=	ScreenMessage(SM_User + 1);
const ScreenMessage SM_GoToNextState	=	ScreenMessage(SM_User + 2);

const int LINE_GAP = 40;

const CString CREDIT_LINES[] = 
{
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"GRAPHICS:",
	"TofuBoy (Lucas Tang)",
	"DJ McFox (Ryan McKanna)",
	"SPiGuMuS",
	"",
	"",
	"",
	"WEB DESIGN:",
	"Brian 'Bork' Bugh",
	"",
	"",
	"",
	"SOUND:",
	"Kyle 'Keel' Ward",
	"",
	"",
	"",
	"PROGRAMMING:",
	"Chris Danford",
	"Ben Nordstrom",
	"Lord Frieza (Andrew Livy)",
	"Parasyte (Chris Gomez)",
	"angedelamort (Sauleil Lamarre)",
	"Edwin Evans",
	"Brian 'Bork' Bugh",
	"Jared Roberts",
	"Elvis314 (Joel Maher)",
	"Kefabi (Garth Smith)",
	"Pkillah (Playah Killah)",
	"DJ McFox (Ryan McKanna)",
	"Robert Kemmetmueller",
	"Shabach (Ben Andersen)",
	"SlinkyWizard (Will Valladao)",
	"TheChip (The Chip)",
	"WarriorBob (David H)",
	"Mike Waltson",
	"",
	"",
	"",
	"SPECIAL THANKS TO:",
	"SimWolf",
	"Dance With Intensity",
	"DDR Llama",
	"DDRManiaX",
	"Lagged",
	"The Melting Pot",
	"DDRJamz Global BBS",
	"BemaniRuler",
	"Brendan Walker",
	"Eric 'WaffleKing' Webster",
	"Mark 'Foobly' Verrey",
	"Mandarin Blue",
	"Anne Kiel",
	"BeMaNiFiNiNaTiC",
	"Garett Sakamoto",
	"Illusionz - Issaquah, WA",
	"Quarters - Kirkland, WA",
	"Segapark - Bournemouth, UK",
	"Pier Amusements - Bournemouth, UK",
	"Westcliff Amusements - Bournemouth, UK",
	"Naoki",
	"Konami Computer Entertainment Japan",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"If your name is missing from this list,",
	"                      send me an e-mail!",
	"                                -Chris"
};
const int NUM_CREDIT_LINES = sizeof(CREDIT_LINES) / sizeof(CString);


ScreenMusicScroll::ScreenMusicScroll()
{
	LOG->WriteLine( "ScreenMusicScroll::ScreenMusicScroll()" );

	int i;

	m_sprBackground.Load( THEME->GetPathTo(GRAPHIC_MUSIC_SCROLL_BACKGROUND) );
	m_sprBackground.StretchTo( CRect(SCREEN_LEFT,SCREEN_TOP,SCREEN_RIGHT,SCREEN_BOTTOM) );
	this->AddActor( &m_sprBackground );


	CArray<Song*, Song*> arraySongs;
	arraySongs.Copy( SONGMAN->m_pSongs );
	SortSongPointerArrayByTitle( arraySongs );
	
	m_iNumLines = 0;

	for( i=0; i<min(arraySongs.GetSize(), MAX_TOTAL_LINES); i++ )
	{
		Song* pSong = arraySongs[i];
		m_textLines[m_iNumLines].Load( THEME->GetPathTo(FONT_NORMAL) );
		m_textLines[m_iNumLines].SetText( pSong->GetFullTitle() );
		m_textLines[m_iNumLines].SetDiffuseColor( SONGMAN->GetGroupColor(pSong->m_sGroupName) );

		m_iNumLines++;
	}

	for( i=0; i<min(NUM_CREDIT_LINES, MAX_CREDIT_LINES); i++ )
	{
		m_textLines[m_iNumLines].Load( THEME->GetPathTo(FONT_NORMAL) );
		m_textLines[m_iNumLines].SetText( CREDIT_LINES[i] );
//		this->AddActor( &m_textLines[m_iNumLines] );

		m_iNumLines++;
	}

	for( i=0; i<m_iNumLines; i++ )
	{
		m_textLines[i].SetZoom( 0.7f );
		m_textLines[i].SetXY( CENTER_X, SCREEN_BOTTOM + 40 );
		m_textLines[i].BeginTweeningQueued( 0.30f * i );
		m_textLines[i].BeginTweeningQueued( 3.0f );
		m_textLines[i].SetTweenXY( CENTER_X, SCREEN_TOP - 40 );	
	}
	
	this->SendScreenMessage( SM_StartFadingOut, 0.3f * i + 3.0f );

	this->AddActor( &m_Fade );

	if ( GAMEMAN->m_CurGame != GAME_EZ2 )	
	{
		m_soundMusic.Load( THEME->GetPathTo(SOUND_MUSIC_SCROLL_MUSIC) );
	}
	else
	{
		m_soundMusic.Load( THEME->GetPathTo(SOUND_MENU_PROMPT) ); // yes, yes, hack hack hack. What can I do huh? re-write thememanager? hahaha.
	}

	m_Fade.OpenWipingRight();
	if ( GAMEMAN->m_CurGame != GAME_EZ2 )
	{
		m_soundMusic.Play( true );
	}
	else
	{
		m_soundMusic.Play( false );
	}
}


void ScreenMusicScroll::Update( float fDeltaTime )
{
	Screen::Update( fDeltaTime );
	
	for( int i=0; i<m_iNumLines; i++ )
	{
		m_textLines[i].Update( fDeltaTime );
	}
		
}	


void ScreenMusicScroll::DrawPrimitives()
{
	Screen::DrawPrimitives();

	for( int i=0; i<m_iNumLines; i++ )
	{
		if( m_textLines[i].GetY() > SCREEN_TOP-20  &&
			m_textLines[i].GetY() < SCREEN_BOTTOM+20 )
			m_textLines[i].Draw();
	}

	m_Fade.Draw();	// render it again so it shows over the text
}	


void ScreenMusicScroll::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	LOG->WriteLine( "ScreenMusicScroll::Input()" );

	if( m_Fade.IsClosing() )
		return;

	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );	// default input handler
}

void ScreenMusicScroll::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_StartFadingOut:
		m_Fade.CloseWipingRight( SM_GoToNextState );
		break;
	case SM_GoToNextState:
		SCREENMAN->SetNewScreen( new ScreenTitleMenu );
		break;
	}
}

void ScreenMusicScroll::MenuStart( const PlayerNumber p )
{
	m_Fade.CloseWipingRight( SM_GoToNextState );
}

void ScreenMusicScroll::MenuBack( const PlayerNumber p )
{
	MenuStart( p );
}


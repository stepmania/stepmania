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
#include "GameManager.h"
#include "RageLog.h"
#include "GameConstantsAndTypes.h"
#include "SongManager.h"
#include "GameState.h"


const ScreenMessage SM_StartFadingOut	=	ScreenMessage(SM_User + 1);
const ScreenMessage SM_GoToNextScreen	=	ScreenMessage(SM_User + 2);

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
	"SPiGuMuS",
	"DJ McFox (Ryan McKanna)",
	"Cloud34 (Lamden Travis)",
	"",
	"",
	"",
	"WEB DESIGN:",
	"Brian 'Bork' Bugh",
	"",
	"",
	"",
	"SOUND:",
	"Kyle 'KeeL' Ward",
	"",
	"",
	"",
	"PROGRAMMING:",
	"Chris Danford",
	"Lord Frieza (Andrew Livy)",
	"Glenn Maynard",
	"Bruno Figueiredo",
	"Dro Kulix (Peter S. May)",
	"Mantis (Ben Nordstrom)",
	"Parasyte (Chris Gomez)",
	"dirkthedaring (Michael Patterson)",
	"angedelamort (Sauleil Lamarre)",
	"Edwin Evans",
	"Brian 'Bork' Bugh",
	"nmspaz (Jared Roberts)",
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
	"DJ DraftHorse",
	"Dance With Intensity",
	"BeMaNiRuler",
	"DDRLlama",
	"DDRManiaX",
	"Lagged",
	"The Melting Pot",
	"DDRJamz Global BBS",
	"Brendan Walker",
	"Eric 'WaffleKing' Webster",
	"Mark 'Foobly' Verrey",
	"Mandarin Blue",
	"Anne Kiel",
	"BeMaNiFiNiNaTiC (Jeremy Hine)",
	"Garett Sakamoto",
	"SailorBob",
	"AngelTK {Kenny Lai}",
	"Gotetsu",
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
	"Please, join the StepMania team and help us out!",
};
const int NUM_CREDIT_LINES = sizeof(CREDIT_LINES) / sizeof(CString);


ScreenMusicScroll::ScreenMusicScroll()
{
	LOG->Trace( "ScreenMusicScroll::ScreenMusicScroll()" );

	 int i;

	m_sprBackground.Load( THEME->GetPathTo("Graphics","music scroll background") );
	m_sprBackground.StretchTo( CRect(SCREEN_LEFT,SCREEN_TOP,SCREEN_RIGHT,SCREEN_BOTTOM) );
	this->AddChild( &m_sprBackground );


	CArray<Song*, Song*> arraySongs;
	arraySongs.Copy( SONGMAN->m_pSongs );
	SortSongPointerArrayByTitle( arraySongs );
	
	m_iNumLines = 0;

	for( i=0; i<min(arraySongs.GetSize(), MAX_TOTAL_LINES); i++ )
	{
		Song* pSong = arraySongs[i];
		m_textLines[m_iNumLines].LoadFromFont( THEME->GetPathTo("Fonts","normal") );
		m_textLines[m_iNumLines].SetText( pSong->GetFullTitle() );
		m_textLines[m_iNumLines].SetDiffuse( SONGMAN->GetSongColor(pSong) );

		m_iNumLines++;
	}

	for( i=0; i<min(NUM_CREDIT_LINES, MAX_CREDIT_LINES); i++ )
	{
		m_textLines[m_iNumLines].LoadFromFont( THEME->GetPathTo("Fonts","normal") );
		m_textLines[m_iNumLines].SetText( CREDIT_LINES[i] );
//		this->AddChild( &m_textLines[m_iNumLines] );

		m_iNumLines++;
	}

	for( i=0; i<m_iNumLines; i++ )
	{
		m_textLines[i].SetZoom( 0.7f );
		m_textLines[i].SetXY( CENTER_X, SCREEN_BOTTOM + 40 );
		m_textLines[i].BeginTweening( 0.20f * i );
		m_textLines[i].BeginTweening( 2.0f );
		m_textLines[i].SetTweenXY( CENTER_X, SCREEN_TOP - 40 );	
	}
	
	this->SendScreenMessage( SM_StartFadingOut, 0.2f * i + 3.0f );

	this->AddChild( &m_Fade );

	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo("music scroll") );

	MUSIC->LoadAndPlayIfNotAlready( THEME->GetPathTo("Sounds","music scroll music") );

	m_Fade.OpenWipingRight();
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
	LOG->Trace( "ScreenMusicScroll::Input()" );

	if( m_Fade.IsClosing() )
		return;

	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );	// default input handler
}

void ScreenMusicScroll::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_StartFadingOut:
		m_Fade.CloseWipingRight( SM_GoToNextScreen );
		break;
	case SM_GoToNextScreen:
		SCREENMAN->SetNewScreen( "ScreenTitleMenu" );
		break;
	}
}

void ScreenMusicScroll::MenuStart( PlayerNumber p )
{
	m_Fade.CloseWipingRight( SM_GoToNextScreen );
}

void ScreenMusicScroll::MenuBack( PlayerNumber p )
{
	MenuStart( p );
}


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
#include "ThemeManager.h"
#include "AnnouncerManager.h"


#define SCROLL_DELAY		THEME->GetMetricF("ScreenMusicScroll","ScrollDelay")
#define SCROLL_SPEED		THEME->GetMetricF("ScreenMusicScroll","ScrollSpeed")
#define TEXT_ZOOM			THEME->GetMetricF("ScreenMusicScroll","TextZoom")

const ScreenMessage SM_StartFadingOut	=	ScreenMessage(SM_User + 1);
const ScreenMessage SM_GoToNextScreen	=	ScreenMessage(SM_User + 2);


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
	"v1ral (Lucas Tang)",
	"SPiGuMuS",
	"Visage",
	"DJ McFox (Ryan McKanna)",
	"Cloud34 (Lamden Travis)",
	"Redcrusher (Michael Curry)",
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
	"Jim 'Animechanic' Cabeen"
	"",
	"",
	"",
	"TESTING:",
	"Gotetsu",
	"spds (James Sanders)",
	"",
	"",
	"",
	"PROGRAMMING:",
	"Chris Danford",
	"Andrew 'Frieza' Livy",
	"Glenn Maynard",
	"Bruno Figueiredo",
	"Dro Kulix (Peter S. May)",
	"nmspaz (Jared Roberts)",
	"binarys (Brendan Walker)",
	"Lance Gilbert (Neovanglist)",
	"Michel Donais",
	"Mantis (Ben Nordstrom)",
	"Parasyte (Chris Gomez)",
	"dirkthedaring (Michael Patterson)",
	"angedelamort (Sauleil Lamarre)",
	"Edwin Evans",
	"Brian 'Bork' Bugh",
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
	"NMR",
	"Random Hajile",
	"Lagged",
	"The Melting Pot",
	"DDRJamz Global BBS",
	"Eric 'WaffleKing' Webster",
	"Mark 'Foobly' Verrey",
	"Mandarin Blue",
	"Anne Kiel",
	"BeMaNiFiNiNaTiC (Jeremy Hine)",
	"Garett Sakamoto",
	"SailorBob",
	"AngelTK (Kenny Lai)",
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
const unsigned NUM_CREDIT_LINES = sizeof(CREDIT_LINES) / sizeof(CString);


ScreenMusicScroll::ScreenMusicScroll()
{
	LOG->Trace( "ScreenMusicScroll::ScreenMusicScroll()" );

	unsigned i;

	GAMESTATE->Reset();		// so that credits message for both players will show

	m_Background.LoadFromAniDir( THEME->GetPathTo("BGAnimations","music scroll") );
	this->AddChild( &m_Background );


	vector<Song*> arraySongs = SONGMAN->m_pSongs;
	SortSongPointerArrayByTitle( arraySongs );
	
	m_iNumLines = 0;

	for( i=0; i<min(arraySongs.size(), MAX_MUSIC_LINES); i++ )
	{
		Song* pSong = arraySongs[i];
		m_textLines[m_iNumLines].LoadFromFont( THEME->GetPathTo("Fonts","music scroll") );
		m_textLines[m_iNumLines].SetText( pSong->GetFullTitle() );
		m_textLines[m_iNumLines].SetDiffuse( SONGMAN->GetSongColor(pSong) );
		m_textLines[m_iNumLines].SetZoom( TEXT_ZOOM );

		m_iNumLines++;
	}

	for( i=0; i<min(NUM_CREDIT_LINES, MAX_CREDIT_LINES); i++ )
	{
		m_textLines[m_iNumLines].LoadFromFont( THEME->GetPathTo("Fonts","music scroll") );
		m_textLines[m_iNumLines].SetText( CREDIT_LINES[i] );
		m_textLines[m_iNumLines].SetZoom( TEXT_ZOOM );

		m_iNumLines++;
	}

	for( i=0; i<m_iNumLines; i++ )
	{
		m_textLines[i].SetXY( CENTER_X, SCREEN_BOTTOM + 40 );
		m_textLines[i].BeginTweening( SCROLL_DELAY * i );
		m_textLines[i].BeginTweening( 2.0f*SCROLL_SPEED );
		m_textLines[i].SetTweenXY( CENTER_X, SCREEN_TOP - 40 );	
	}
	
	this->SendScreenMessage( SM_StartFadingOut, 0.2f * i + 3.0f );

	this->AddChild( &m_Fade );

	SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo("music scroll") );

	SOUNDMAN->PlayMusic( THEME->GetPathTo("Sounds","music scroll music") );

	m_Fade.OpenWipingRight();
}


void ScreenMusicScroll::Update( float fDeltaTime )
{
	Screen::Update( fDeltaTime );
	
	for( unsigned i=0; i<m_iNumLines; i++ )
		m_textLines[i].Update( fDeltaTime );
}	


void ScreenMusicScroll::DrawPrimitives()
{
	Screen::DrawPrimitives();

	for( unsigned i=0; i<m_iNumLines; i++ )
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
		SCREENMAN->SetNewScreen( "ScreenCompany" );
		break;
	}
}

void ScreenMusicScroll::MenuStart( PlayerNumber pn )
{
	m_Fade.CloseWipingRight( SM_GoToNextScreen );
}

void ScreenMusicScroll::MenuBack( PlayerNumber pn )
{
	MenuStart( pn );
}


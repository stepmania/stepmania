#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: ScreenEz2SelectMusic.cpp

 Desc: See Header

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.

  Andrew Livy
-----------------------------------------------------------------------------
*/

#include "stdafx.h"

#include "ScreenEz2SelectMusic.h"

#include "RageDisplay.h"
#include <math.h>

#include "SDL.h"
#include "SDL_opengl.h"
#include "ScreenSandbox.h"
#include "ScreenManager.h"
#include "RageMusic.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "ThemeManager.h"
#include "GameState.h"
#include "StyleDef.h"
#include "InputMapper.h"
#include "CodeDetector.h"

#define SCROLLING_LIST_X				THEME->GetMetricF("ScreenEz2SelectMusic","ScrollingListX")
#define SCROLLING_LIST_Y				THEME->GetMetricF("ScreenEz2SelectMusic","ScrollingListY")
#define PUMP_DIFF_X	THEME->GetMetricI("ScreenEz2SelectMusic","PumpDifficultyX")
#define PUMP_DIFF_Y	THEME->GetMetricI("ScreenEz2SelectMusic","PumpDifficultyY")
#define HELP_TEXT				THEME->GetMetric("ScreenSelectMusic","HelpText")
#define TIMER_SECONDS			THEME->GetMetricI("ScreenSelectMusic","TimerSeconds")
#define METER_X( p )			THEME->GetMetricF("ScreenEz2SelectMusic",ssprintf("MeterP%dX",p+1))
#define METER_Y( p )			THEME->GetMetricF("ScreenEz2SelectMusic",ssprintf("MeterP%dY",p+1))
#define GUIDE_X							THEME->GetMetricF("ScreenSelectMode","GuideX")
#define GUIDE_Y							THEME->GetMetricF("ScreenSelectMode","GuideY")

const ScreenMessage SM_GoToPrevScreen		=	ScreenMessage(SM_User+1);

ScreenEz2SelectMusic::ScreenEz2SelectMusic()
{	
	CodeDetector::RefreshCacheItems();
	m_Menu.Load(
		THEME->GetPathTo("BGAnimations","select music"), 
		THEME->GetPathTo("Graphics","select music top edge"),
		HELP_TEXT, true, true, TIMER_SECONDS 
		);
	this->AddChild( &m_Menu );


	m_ChoiceListFrame.Load( THEME->GetPathTo("Graphics","select mode list frame"));
	m_ChoiceListFrame.SetXY( SCROLLING_LIST_X, SCROLLING_LIST_Y);
	this->AddChild( &m_ChoiceListFrame );

	m_Guide.Load( THEME->GetPathTo("Graphics","select mode guide"));
	m_Guide.SetXY( GUIDE_X, GUIDE_Y );
	this->AddChild( &m_Guide );

	m_MusicBannerWheel.SetX(SCROLLING_LIST_X);
	m_MusicBannerWheel.SetY(SCROLLING_LIST_Y);
	this->AddChild( &m_MusicBannerWheel );

	m_ChoiceListHighlight.Load( THEME->GetPathTo("Graphics","select mode list highlight"));
	m_ChoiceListHighlight.SetXY( SCROLLING_LIST_X, SCROLLING_LIST_Y);
	this->AddChild( &m_ChoiceListHighlight );

	for(int p=0; p<NUM_PLAYERS; p++ )
	{
		m_FootMeter[p].SetXY( METER_X(p), METER_Y(p) );
		m_FootMeter[p].SetShadowLength( 2 );
		this->AddChild( &m_FootMeter[p] );

		m_iSelection[p] = 0;
	}

	m_PumpDifficultyRating.LoadFromFont( THEME->GetPathTo("Fonts","pump songselect difficulty") );
	m_PumpDifficultyRating.SetXY( PUMP_DIFF_X, PUMP_DIFF_Y );
	this->AddChild(&m_PumpDifficultyRating);

	MusicChanged();

	m_Menu.TweenOnScreenFromMenu( SM_None );
}

void ScreenEz2SelectMusic::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
//	if( type != IET_FIRST_PRESS )
//		return;	// ignore

	PlayerNumber pn = GAMESTATE->GetCurrentStyleDef()->ControllerToPlayerNumber( GameI.controller );

	if( CodeDetector::EnteredEasierDifficulty(GameI.controller) )
	{
		EasierDifficulty( pn );
		return;
	}
	if( CodeDetector::EnteredHarderDifficulty(GameI.controller) )
	{
		HarderDifficulty( pn );
		return;
	}

	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );
}

void ScreenEz2SelectMusic::HandleScreenMessage( const ScreenMessage SM )
{
	Screen::HandleScreenMessage( SM );

	switch( SM )
	{
	case SM_GoToPrevScreen:
		SCREENMAN->SetNewScreen( "ScreenTitleMenu" );
		break;
	}
}


void ScreenEz2SelectMusic::MenuRight( PlayerNumber pn, const InputEventType type )
{
		m_MusicBannerWheel.BannersRight();
		MusicChanged();
}

void ScreenEz2SelectMusic::MenuBack( PlayerNumber pn )
{
	MUSIC->Stop();

	m_Menu.TweenOffScreenToBlack( SM_GoToPrevScreen, true );
}


void ScreenEz2SelectMusic::MenuLeft( PlayerNumber pn, const InputEventType type )
{
	m_MusicBannerWheel.BannersLeft();
	MusicChanged();
}

void ScreenEz2SelectMusic::MenuStart( PlayerNumber pn )
{
	if( !m_MusicBannerWheel.GetSelectedSong()->HasMusic() )
	{
		/* TODO: gray these out. 
			*
			* XXX: also, make sure they're not selected by roulette */
		SCREENMAN->Prompt( SM_None, "ERROR:\n \nThis song does not have a music file\n and cannot be played." );
		return;
	}
	MUSIC->Stop();
	SCREENMAN->SetNewScreen( "ScreenStage" );
}


void ScreenEz2SelectMusic::Update( float fDeltaTime )
{
	Screen::Update( fDeltaTime );
}

void ScreenEz2SelectMusic::DrawPrimitives()
{
	m_Menu.DrawBottomLayer();
	Screen::DrawPrimitives();
	m_Menu.DrawTopLayer();
}

void ScreenEz2SelectMusic::EasierDifficulty( PlayerNumber pn )
{
	if( !GAMESTATE->IsPlayerEnabled(pn) )
		return;
	if( m_arrayNotes[pn].empty() )
		return;
	if( m_iSelection[pn] == 0 )
		return;

	m_iSelection[pn]--;
	// the user explicity switched difficulties.  Update the preferred difficulty
	GAMESTATE->m_PreferredDifficulty[pn] = m_arrayNotes[pn][ m_iSelection[pn] ]->m_Difficulty;

//	m_soundChangeNotes.Play();

	AfterNotesChange( pn );
}



void ScreenEz2SelectMusic::HarderDifficulty( PlayerNumber pn )
{
	if( !GAMESTATE->IsPlayerEnabled(pn) )
		return;
	if( m_arrayNotes[pn].empty() )
		return;
	if( m_iSelection[pn] == int(m_arrayNotes[pn].size()-1) )
		return;

	m_iSelection[pn]++;
	// the user explicity switched difficulties.  Update the preferred difficulty
	GAMESTATE->m_PreferredDifficulty[pn] = m_arrayNotes[pn][ m_iSelection[pn] ]->m_Difficulty;

//	m_soundChangeNotes.Play();

	AfterNotesChange( pn );
}



void ScreenEz2SelectMusic::MusicChanged()
{
	Song* pSong = m_MusicBannerWheel.GetSelectedSong();
	GAMESTATE->m_pCurSong = pSong;

	int pn;
	for( pn = 0; pn < NUM_PLAYERS; ++pn)
		m_arrayNotes[pn].clear();


	for( pn = 0; pn < NUM_PLAYERS; ++pn) 
	{
		pSong->GetNotesThatMatch( GAMESTATE->GetCurrentStyleDef()->m_NotesType, m_arrayNotes[pn] );
		SortNotesArrayByDifficulty( m_arrayNotes[pn] );
	}

	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		AfterNotesChange( (PlayerNumber)p );
	}
}

void ScreenEz2SelectMusic::AfterNotesChange( PlayerNumber pn )
{
	if( !GAMESTATE->IsPlayerEnabled(pn) )
		return;
	Notes* pNotes = m_arrayNotes[pn].empty()? NULL: m_arrayNotes[pn][m_iSelection[pn]];

	if( pNotes != NULL )
		m_PumpDifficultyRating.SetText(ssprintf("Lv.%d",pNotes->m_iMeter));

	GAMESTATE->m_pCurNotes[pn] = pNotes;

//	Notes* m_pNotes = GAMESTATE->m_pCurNotes[pn];

	m_FootMeter[pn].SetFromNotes( pNotes );
}


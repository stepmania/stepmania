#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenNameEntry

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenNameEntry.h"
#include "SongManager.h"
#include "ScreenManager.h"
#include "GameConstantsAndTypes.h"
#include "RageUtil.h"
#include "PrefsManager.h"
#include "GameManager.h"
#include "RageLog.h"
#include "GameState.h"
#include "RageSoundManager.h"
#include "ThemeManager.h"
#include "ScreenRanking.h"
#include <math.h>


//
// Defines specific to ScreenNameEntry
//
#define TIMER_X						THEME->GetMetricF("ScreenNameEntry","TimerX")
#define TIMER_Y						THEME->GetMetricF("ScreenNameEntry","TimerY")
#define CATEGORY_Y					THEME->GetMetricF("ScreenNameEntry","CategoryY")
#define CHARS_ZOOM_SMALL			THEME->GetMetricF("ScreenNameEntry","CharsZoomSmall")
#define CHARS_ZOOM_LARGE			THEME->GetMetricF("ScreenNameEntry","CharsZoomLarge")
#define CHARS_SPACING_Y				THEME->GetMetricF("ScreenNameEntry","CharsSpacingY")
#define SCROLLING_CHARS_COLOR		THEME->GetMetricC("ScreenNameEntry","ScrollingCharsColor")
#define SELECTED_CHARS_COLOR		THEME->GetMetricC("ScreenNameEntry","SelectedCharsColor")
#define GRAY_ARROWS_Y				THEME->GetMetricF("ScreenNameEntry","GrayArrowsY")
#define NUM_CHARS_TO_DRAW_BEHIND	THEME->GetMetricI("ScreenNameEntry","NumCharsToDrawBehind")
#define NUM_CHARS_TO_DRAW_TOTAL		THEME->GetMetricI("ScreenNameEntry","NumCharsToDrawTotal")
#define FAKE_BEATS_PER_SEC			THEME->GetMetricF("ScreenNameEntry","FakeBeatsPerSec")
#define TIMER_SECONDS				THEME->GetMetricI("ScreenNameEntry","TimerSeconds")


// cache for frequently used metrics
float	g_fCharsZoomSmall;
float	g_fCharsZoomLarge; 
float	g_fCharsSpacingY;
RageColor	g_ScrollingCharsColor; 
RageColor	g_SelectedCharsColor; 
float	g_fGrayArrowsY;
int		g_iNumCharsToDrawBehind;
int		g_iNumCharsToDrawTotal;
float	g_fFakeBeatsPerSec;


const char NAME_CHARS[] =
{
	' ',' ',' ',' ','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z'
};
#define NUM_NAME_CHARS (sizeof(NAME_CHARS)/sizeof(char))
#define HEIGHT_OF_ALL_CHARS		(NUM_NAME_CHARS * g_fCharsSpacingY)



int GetClosestCharIndex( float fFakeBeat )
{
	return ((int)roundf(fFakeBeat)) % NUM_NAME_CHARS;
};

// return value is relative to gray arrows
float GetClosestCharYOffset( float fFakeBeat )
{
	float f = fmodf(fFakeBeat, 1.0f);
	if( f > 0.5f )
		f -= 1;
	ASSERT( f>=-0.5f && f<=0.5f );
	return -f;	
}

// return value is relative to gray arrows
float GetClosestCharYPos( float fFakeBeat )
{
	return roundf( GetClosestCharYOffset(fFakeBeat)*g_fCharsSpacingY );
}


ScreenNameEntry::ScreenNameEntry()
{
	LOG->Trace( "ScreenNameEntry::ScreenNameEntry()" );

	// update cache
	g_fCharsZoomSmall = CHARS_ZOOM_SMALL;
	g_fCharsZoomLarge = CHARS_ZOOM_LARGE;
	g_fCharsSpacingY = CHARS_SPACING_Y;
	g_ScrollingCharsColor = SCROLLING_CHARS_COLOR;
	g_SelectedCharsColor = SELECTED_CHARS_COLOR;
	g_fGrayArrowsY = GRAY_ARROWS_Y;
	g_iNumCharsToDrawBehind = NUM_CHARS_TO_DRAW_BEHIND;
	g_iNumCharsToDrawTotal = NUM_CHARS_TO_DRAW_TOTAL;
	g_fFakeBeatsPerSec = FAKE_BEATS_PER_SEC;


	
	// DEBUGGING STUFF
//	GAMESTATE->m_CurGame = GAME_DANCE;
//	GAMESTATE->m_CurStyle = STYLE_DANCE_SINGLE;
//	GAMESTATE->m_PlayMode = PLAY_MODE_ARCADE;
//	GAMESTATE->m_bSideIsJoined[PLAYER_1] = true;
//	GAMESTATE->m_MasterPlayerNumber = PLAYER_1;
//	GAMESTATE->m_LastRankingCategory[PLAYER_1] = RANKING_A;
//	GAMESTATE->m_iLastHighScoreIndex[PLAYER_1] = 0;




	GAMESTATE->m_bPastHereWeGo = true;	// enable the gray arrows

	m_Background.LoadFromAniDir( THEME->GetPathTo("BGAnimations","name entry") );
	this->AddChild( &m_Background );

	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		bool bNewHighScore = GAMESTATE->m_iLastHighScoreIndex[p] != -1;
		m_bConfirmedName[p] = !bNewHighScore;	// false if they made a new high score

		if( !bNewHighScore )
			continue;	// skip

		ASSERT( GAMESTATE->IsPlayerEnabled(p) );	// they better be enabled if they made a high score!

		m_GrayArrowRow[p].Load( (PlayerNumber)p );
		m_GrayArrowRow[p].SetX( (float)GAMESTATE->GetCurrentStyleDef()->m_iCenterX[p] );
		m_GrayArrowRow[p].SetY( SCREEN_TOP + 100 );
		this->AddChild( &m_GrayArrowRow[p] );


		const StyleDef* pStyleDef = GAMESTATE->GetCurrentStyleDef();
		for( int t=0; t<pStyleDef->m_iColsPerPlayer; t++ )
		{
			float ColX = pStyleDef->m_iCenterX[p] + pStyleDef->m_ColumnInfo[p][t].fXOffset;

			m_textSelectedChars[p][t].LoadFromFont( THEME->GetPathTo("Fonts","ranking") );
			m_textSelectedChars[p][t].SetX( ColX );
			m_textSelectedChars[p][t].SetY( GRAY_ARROWS_Y );
			m_textSelectedChars[p][t].SetDiffuse( g_SelectedCharsColor );
			m_textSelectedChars[p][t].SetZoom( CHARS_ZOOM_LARGE );
			this->AddChild( &m_textSelectedChars[p][t] );		// draw these manually
			
			m_textScrollingChars[p][t].LoadFromFont( THEME->GetPathTo("Fonts","ranking") );
			m_textScrollingChars[p][t].SetX( ColX );
			m_textScrollingChars[p][t].SetY( GRAY_ARROWS_Y );
			m_textScrollingChars[p][t].SetDiffuse( g_ScrollingCharsColor );
			//this->AddChild( &m_textScrollingChars[p][t] );	// draw these manually
		}

		m_textCategory[p].LoadFromFont( THEME->GetPathTo("Fonts","header2") );
		m_textCategory[p].SetX( (float)GAMESTATE->GetCurrentStyleDef()->m_iCenterX[p] );
		m_textCategory[p].SetY( CATEGORY_Y );
		CString sCategoryText = ssprintf("No. %d", GAMESTATE->m_iLastHighScoreIndex[p]+1);
		switch( GAMESTATE->m_PlayMode )
		{
		case PLAY_MODE_ARCADE:
			sCategoryText += ssprintf(" in Type %c", 'A'+GAMESTATE->m_LastRankingCategory[p]);
			break;
		case PLAY_MODE_NONSTOP:
		case PLAY_MODE_ONI:
		case PLAY_MODE_ENDLESS:
			sCategoryText += ssprintf(" in %s", GAMESTATE->m_pCurCourse->m_sName.c_str());
			break;
		default:
			ASSERT(0);
		}
		m_textCategory[p].SetText( sCategoryText );
		this->AddChild( &m_textCategory[p] );
	}

	m_Timer.SetTimer(TIMER_SECONDS);
	m_Timer.SetXY( TIMER_X, TIMER_Y );
	this->AddChild( &m_Timer );

	m_Fade.OpenWipingRight();
//	this->AddChild( &m_Fade );	// draw and update this manually too

	m_soundStep.Load( THEME->GetPathTo("Sounds","name entry step") );

	SOUNDMAN->PlayMusic( THEME->GetPathTo("Sounds","name entry music") );

	m_fFakeBeat = 0;
}


ScreenNameEntry::~ScreenNameEntry()
{
	LOG->Trace( "ScreenNameEntry::~ScreenNameEntry()" );
}

void ScreenNameEntry::Update( float fDelta )
{
	m_fFakeBeat += fDelta * FAKE_BEATS_PER_SEC;
	GAMESTATE->m_fSongBeat = m_fFakeBeat;
	Screen::Update(fDelta);

	m_Fade.Update( fDelta );
}

void ScreenNameEntry::DrawPrimitives()
{
	Screen::DrawPrimitives();

	int iClosestIndex = GetClosestCharIndex( m_fFakeBeat );
	int iStartDrawingIndex = iClosestIndex - NUM_CHARS_TO_DRAW_BEHIND;
	iStartDrawingIndex += NUM_NAME_CHARS;	// make positive

	const StyleDef* pStyleDef = GAMESTATE->GetCurrentStyleDef();

	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		if( m_bConfirmedName[p] )
			continue;	// don't draw scrolling arrows.  They already confirmed.

		float fY = GRAY_ARROWS_Y + GetClosestCharYPos(m_fFakeBeat) - g_iNumCharsToDrawBehind*g_fCharsSpacingY;
		int iCharIndex = iStartDrawingIndex % NUM_NAME_CHARS;
		for( int i=0; i<NUM_CHARS_TO_DRAW_TOTAL; i++ )
		{
			char c = NAME_CHARS[iCharIndex];
			for( int t=0; t<pStyleDef->m_iColsPerPlayer; t++ )
			{
				m_textScrollingChars[p][t].SetText( ssprintf("%c",c) );	// why doens't CStdStr have a contructor that takes a char?
				m_textScrollingChars[p][t].SetY( fY );
				float fZoom = g_fCharsZoomSmall;
				if( iCharIndex==iClosestIndex )
					fZoom = SCALE(fabsf(GetClosestCharYOffset(m_fFakeBeat)),0,0.5f,g_fCharsZoomLarge,g_fCharsZoomSmall);
				m_textScrollingChars[p][t].SetZoom(fZoom);
				RageColor color = g_ScrollingCharsColor;
				if( i==0 )
					color.a *= SCALE(GetClosestCharYOffset(m_fFakeBeat),-0.5f,0.f,0.f,1.f);
				if( i==g_iNumCharsToDrawTotal-1 )
					color.a *= SCALE(GetClosestCharYOffset(m_fFakeBeat),0.f,0.5f,1.f,0.f);
				m_textScrollingChars[p][t].SetDiffuse( color );
				m_textScrollingChars[p][t].Draw();
			}
			fY += g_fCharsSpacingY;
			iCharIndex = (iCharIndex+1) % NUM_NAME_CHARS;
		}


		for( int t=0; t<pStyleDef->m_iColsPerPlayer; t++ )
		{
			m_textSelectedChars[p][t].Draw();
		}
	}


	m_Fade.Draw();
}

void ScreenNameEntry::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	LOG->Trace( "ScreenNameEntry::Input()" );

	if( type != IET_FIRST_PRESS )
		return;		// ignore

	if( m_Fade.IsClosing() )
		return;

	if( StyleI.IsValid() )
	{
		m_GrayArrowRow[StyleI.player].Step( StyleI.col );
		m_soundStep.Play();
		char c = NAME_CHARS[GetClosestCharIndex(m_fFakeBeat)];
		m_textSelectedChars[StyleI.player][StyleI.col].SetText( ssprintf("%c",c) );
	}

	if( MenuI.IsValid() )
	{
		if( MenuI.button == MENU_BUTTON_START )
		{
			m_soundStep.Play();

			m_bConfirmedName[MenuI.player] = true;

			bool bAllConfirmed = true;
			for( int p=0; p<NUM_PLAYERS; p++ )
				bAllConfirmed &= m_bConfirmedName[p];
			if( bAllConfirmed )
			{
				if( !m_Fade.IsClosing() )
					m_Fade.CloseWipingRight( SM_GoToNextScreen );
			}
		}
	}

	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );
}

void ScreenNameEntry::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_MenuTimer:
		if( !m_Fade.IsClosing() )
			m_Fade.CloseWipingRight( SM_GoToNextScreen );
		break;
	case SM_GoToNextScreen:
		SCREENMAN->SetNewScreen( "ScreenMusicScroll" );
		break;
	}
}

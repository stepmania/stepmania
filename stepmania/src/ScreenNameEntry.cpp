#include "global.h"
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
#include "RageSounds.h"
#include "ThemeManager.h"
#include "ScreenRanking.h"
#include "Course.h"
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
	int iCharIndex = (int)roundf(fFakeBeat) % NUM_NAME_CHARS;
	ASSERT( iCharIndex >= 0 );
	return iCharIndex;
}

// return value is relative to gray arrows
float GetClosestCharYOffset( float fFakeBeat )
{
	float f = fmodf(fFakeBeat, 1.0f);
	if( f > 0.5f )
		f -= 1;
	ASSERT( f>-0.5f && f<=0.5f );
	return -f;	
}

// return value is relative to gray arrows
float GetClosestCharYPos( float fFakeBeat )
{
	return GetClosestCharYOffset(fFakeBeat)*g_fCharsSpacingY;
}


ScreenNameEntry::ScreenNameEntry() : Screen("ScreenNameEntry")
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
//	GAMESTATE->m_RankingCategory[PLAYER_1] = RANKING_A;
//	GAMESTATE->m_iRankingIndex[PLAYER_1] = 0;



	// reset Player and Song Options
	{
		for( int p=0; p<NUM_PLAYERS; p++ )
			GAMESTATE->m_PlayerOptions[p] = PlayerOptions();
		GAMESTATE->m_SongOptions = SongOptions();
	}


	GAMESTATE->m_bPastHereWeGo = true;	// enable the gray arrows

	m_Background.LoadFromAniDir( THEME->GetPathToB("ScreenNameEntry background") );
	this->AddChild( &m_Background );

	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		// Find out if player deserves to enter their name
		bool bNewHighScore = GAMESTATE->m_iRankingIndex[p] != -1;
		m_bStillEnteringName[p] = bNewHighScore;	// false if they made a new high score
		for( int i=0; i<MAX_RANKING_NAME_LENGTH; i++ )
			m_sSelectedName[p] += ' ';

		if( !bNewHighScore )
			continue;	// skip

		// remove modifiers that may have been on the last song
		GAMESTATE->m_PlayerOptions[p] = PlayerOptions();

		ASSERT( GAMESTATE->IsHumanPlayer(p) );	// they better be enabled if they made a high score!

		m_GrayArrowRow[p].Load( (PlayerNumber)p );
		m_GrayArrowRow[p].SetX( (float)GAMESTATE->GetCurrentStyleDef()->m_iCenterX[p] );
		m_GrayArrowRow[p].SetY( SCREEN_TOP + 100 );
		this->AddChild( &m_GrayArrowRow[p] );


		const StyleDef* pStyleDef = GAMESTATE->GetCurrentStyleDef();

		m_ColToStringIndex[p].insert(m_ColToStringIndex[p].begin(), pStyleDef->m_iColsPerPlayer, -1);
		int CurrentStringIndex = 0;

		for( int t=0; t<pStyleDef->m_iColsPerPlayer; t++ )
		{
			if(CurrentStringIndex == MAX_RANKING_NAME_LENGTH)
				continue; /* We have enough columns. */

			/* Find out if this column is associated with the START menu button. */
			StyleInput si((PlayerNumber)p, t);
			GameInput gi=GAMESTATE->GetCurrentStyleDef()->StyleInputToGameInput(si);
			MenuInput m=GAMESTATE->GetCurrentGameDef()->GameInputToMenuInput(gi);
			if(m.button == MENU_BUTTON_START)
				continue;
			m_ColToStringIndex[p][t] = CurrentStringIndex++;

			float ColX = pStyleDef->m_iCenterX[p] + pStyleDef->m_ColumnInfo[p][t].fXOffset;

			m_textSelectedChars[p][t].LoadFromFont( THEME->GetPathToF("ScreenNameEntry letters") );
			m_textSelectedChars[p][t].SetX( ColX );
			m_textSelectedChars[p][t].SetY( GRAY_ARROWS_Y );
			m_textSelectedChars[p][t].SetDiffuse( g_SelectedCharsColor );
			m_textSelectedChars[p][t].SetZoom( CHARS_ZOOM_LARGE );
			this->AddChild( &m_textSelectedChars[p][t] );		// draw these manually
			
			m_textScrollingChars[p][t].LoadFromFont( THEME->GetPathToF("ScreenNameEntry letters") );
			m_textScrollingChars[p][t].SetX( ColX );
			m_textScrollingChars[p][t].SetY( GRAY_ARROWS_Y );
			m_textScrollingChars[p][t].SetDiffuse( g_ScrollingCharsColor );
			//this->AddChild( &m_textScrollingChars[p][t] );	// draw these manually
		}

		m_textCategory[p].LoadFromFont( THEME->GetPathToF("ScreenNameEntry category") );
		m_textCategory[p].SetX( (float)GAMESTATE->GetCurrentStyleDef()->m_iCenterX[p] );
		m_textCategory[p].SetY( CATEGORY_Y );
		CString sCategoryText = ssprintf("No. %d in\n", GAMESTATE->m_iRankingIndex[p]+1);
		switch( GAMESTATE->m_PlayMode )
		{
		case PLAY_MODE_ARCADE:
		case PLAY_MODE_HUMAN_BATTLE:
		case PLAY_MODE_CPU_BATTLE:
		case PLAY_MODE_RAVE:
			{
				StageStats SS;
				vector<Song*> vSongs;
				GAMESTATE->GetFinalEvalStatsAndSongs( SS, vSongs );
				sCategoryText += ssprintf("Type %c (%d)", 'A'+GAMESTATE->m_RankingCategory[p], SS.iMeter[p] );
			}
			break;
		case PLAY_MODE_NONSTOP:
		case PLAY_MODE_ONI:
		case PLAY_MODE_ENDLESS:
			sCategoryText += ssprintf("%s", GAMESTATE->m_pCurCourse->m_sName.c_str());
			break;
		default:
			ASSERT(0);
		}
		m_textCategory[p].SetText( sCategoryText );
		this->AddChild( &m_textCategory[p] );
	}



	bool bAnyStillEntering = false;
	
	{
		for( int p=0; p<NUM_PLAYERS; p++ )
			bAnyStillEntering |= m_bStillEnteringName[p];
		if( !bAnyStillEntering )
		{
			HandleScreenMessage( SM_GoToNextScreen );
			return;
		}
	}




	if( !PREFSMAN->m_bMenuTimer )
		m_Timer.Disable();
	else
		m_Timer.SetSeconds(TIMER_SECONDS);
	m_Timer.SetXY( TIMER_X, TIMER_Y );
	this->AddChild( &m_Timer );

	m_In.Load( THEME->GetPathToB("ScreenNameEntry in") );
	m_In.StartTransitioning();
//	this->AddChild( &m_In );	// draw and update this manually too

	m_Out.Load( THEME->GetPathToB("ScreenNameEntry out") );
//	this->AddChild( &m_Out );	// draw and update this manually too

	m_soundStep.Load( THEME->GetPathToS("ScreenNameEntry step") );

	SOUND->PlayMusic( THEME->GetPathToS("ScreenNameEntry music") );

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

	m_In.Update( fDelta );
	m_Out.Update( fDelta );
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
		if( !m_bStillEnteringName[p] )
			continue;	// don't draw scrolling arrows

		float fY = GRAY_ARROWS_Y + GetClosestCharYPos(m_fFakeBeat) - g_iNumCharsToDrawBehind*g_fCharsSpacingY;
		int iCharIndex = iStartDrawingIndex % NUM_NAME_CHARS;
		
		for( int i=0; i<NUM_CHARS_TO_DRAW_TOTAL; i++ )
		{
			char c = NAME_CHARS[iCharIndex];
			for( int t=0; t<pStyleDef->m_iColsPerPlayer; t++ )
			{
				if(m_ColToStringIndex[p][t] == -1)
					continue;

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


	m_In.Draw();
	m_Out.Draw();
}

void ScreenNameEntry::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	LOG->Trace( "ScreenNameEntry::Input()" );

	if( m_In.IsTransitioning() || m_Out.IsTransitioning() )
		return;	

	if( type != IET_FIRST_PRESS )
		return;		// ignore

	if( StyleI.IsValid() && m_bStillEnteringName[StyleI.player])
	{
		int StringIndex = m_ColToStringIndex[StyleI.player][StyleI.col];
		if(StringIndex != -1)
		{
			m_GrayArrowRow[StyleI.player].Step( StyleI.col );
			m_soundStep.Play();
			char c = NAME_CHARS[GetClosestCharIndex(m_fFakeBeat)];
			m_textSelectedChars[StyleI.player][StyleI.col].SetText( ssprintf("%c",c) );
			m_sSelectedName[StyleI.player][StringIndex] = c;
		}
	}

	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );
}

void ScreenNameEntry::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_MenuTimer:
		if( !m_Out.IsTransitioning() )
		{
			for( int p=0; p<NUM_PLAYERS; p++ )
				this->MenuStart( (PlayerNumber)p );
		}
		break;
	case SM_GoToNextScreen:
		{
			Grade max_grade = GRADE_E;
			vector<Song*> vSongs;
			StageStats stats;
			GAMESTATE->GetFinalEvalStatsAndSongs( stats, vSongs );
			/* Hack: go back to the select course screen in event mode. */
			if( PREFSMAN->m_bEventMode && GAMESTATE->IsCourseMode() )
			{
				SCREENMAN->SetNewScreen( "ScreenSelectCourse" );
				break;
			}

			for( int p=0; p<NUM_PLAYERS; p++ )
				if( GAMESTATE->IsHumanPlayer(p) )
					max_grade = max( max_grade, stats.GetGrade((PlayerNumber)p) );
			if( max_grade >= GRADE_AA )
				SCREENMAN->SetNewScreen( "ScreenCredits" );
			else
				SCREENMAN->SetNewScreen( "ScreenMusicScroll" );
		}
		break;
	}
}


void ScreenNameEntry::MenuStart( PlayerNumber pn )
{
	if( !m_bStillEnteringName[pn] )
		return;
	m_bStillEnteringName[pn] = false;

	m_soundStep.Play();

	TrimRight( m_sSelectedName[pn], " " );
	TrimLeft( m_sSelectedName[pn], " " );

	if( m_sSelectedName[pn] == "" )
		m_sSelectedName[pn] = DEFAULT_RANKING_NAME;
	
	switch( GAMESTATE->m_PlayMode )
	{
	case PLAY_MODE_ARCADE:
	case PLAY_MODE_HUMAN_BATTLE:
	case PLAY_MODE_CPU_BATTLE:
	case PLAY_MODE_RAVE:
		SONGMAN->m_MachineScores[GAMESTATE->m_RankingNotesType][GAMESTATE->m_RankingCategory[pn]][GAMESTATE->m_iRankingIndex[pn]].sName = m_sSelectedName[pn];
		break;
	case PLAY_MODE_NONSTOP:
	case PLAY_MODE_ONI:
	case PLAY_MODE_ENDLESS:
		GAMESTATE->m_pRankingCourse->m_RankingScores[GAMESTATE->m_RankingNotesType][GAMESTATE->m_iRankingIndex[pn]].sName = m_sSelectedName[pn];
		break;
	default:
		ASSERT(0);
	}

	bool bAnyStillEntering = false;
	for( int p=0; p<NUM_PLAYERS; p++ )
		bAnyStillEntering |= m_bStillEnteringName[p];
	if( !bAnyStillEntering && !m_Out.IsTransitioning() )
		m_Out.StartTransitioning( SM_GoToNextScreen );
}

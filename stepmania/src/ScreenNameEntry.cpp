#include "global.h"
#include "ScreenNameEntry.h"
#include "SongManager.h"
#include "GameConstantsAndTypes.h"
#include "RageUtil.h"
#include "PrefsManager.h"
#include "GameManager.h"
#include "RageLog.h"
#include "GameState.h"
#include "GameSoundManager.h"
#include "ThemeManager.h"
#include "Course.h"
#include "AnnouncerManager.h"
#include "ProfileManager.h"
#include "Profile.h"
#include "StageStats.h"
#include "Game.h"
#include "ScreenDimensions.h"
#include "PlayerState.h"
#include "Style.h"
#include "NoteSkinManager.h"
#include "InputEventPlus.h"


//
// Defines specific to ScreenNameEntry
//
#define TIMER_X				THEME->GetMetricF("ScreenNameEntry","TimerX")
#define TIMER_Y				THEME->GetMetricF("ScreenNameEntry","TimerY")
#define CATEGORY_Y			THEME->GetMetricF("ScreenNameEntry","CategoryY")
#define CATEGORY_ZOOM			THEME->GetMetricF("ScreenNameEntry","CategoryZoom")
#define CHARS_ZOOM_SMALL		THEME->GetMetricF("ScreenNameEntry","CharsZoomSmall")
#define CHARS_ZOOM_LARGE		THEME->GetMetricF("ScreenNameEntry","CharsZoomLarge")
#define CHARS_SPACING_Y			THEME->GetMetricF("ScreenNameEntry","CharsSpacingY")
#define SCROLLING_CHARS_COMMAND		THEME->GetMetricA("ScreenNameEntry","ScrollingCharsCommand")
#define SELECTED_CHARS_COMMAND		THEME->GetMetricA("ScreenNameEntry","SelectedCharsCommand")
#define GRAY_ARROWS_Y			THEME->GetMetricF("ScreenNameEntry","ReceptorArrowsY")
#define NUM_CHARS_TO_DRAW_BEHIND	THEME->GetMetricI("ScreenNameEntry","NumCharsToDrawBehind")
#define NUM_CHARS_TO_DRAW_TOTAL		THEME->GetMetricI("ScreenNameEntry","NumCharsToDrawTotal")
#define FAKE_BEATS_PER_SEC		THEME->GetMetricF("ScreenNameEntry","FakeBeatsPerSec")
#define TIMER_SECONDS			THEME->GetMetricF("ScreenNameEntry","TimerSeconds")
#define MAX_RANKING_NAME_LENGTH		THEME->GetMetricI(m_sName,"MaxRankingNameLength")
#define PLAYER_X( p, styleType )	THEME->GetMetricF(m_sName,ssprintf("PlayerP%d%sX",p+1,StyleTypeToString(styleType).c_str()))


// cache for frequently used metrics
float	g_fCharsZoomSmall;
float	g_fCharsZoomLarge; 
float	g_fCharsSpacingY;
float	g_fReceptorArrowsY;
int	g_iNumCharsToDrawBehind;
int	g_iNumCharsToDrawTotal;
float	g_fFakeBeatsPerSec;


const char NAME_CHARS[] =
{
	' ',' ',' ',' ','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z'
};
#define NUM_NAME_CHARS (ARRAYLEN(NAME_CHARS))
#define HEIGHT_OF_ALL_CHARS (NUM_NAME_CHARS * g_fCharsSpacingY)


static int GetClosestCharIndex( float fFakeBeat )
{
	int iCharIndex = (int)roundf(fFakeBeat) % NUM_NAME_CHARS;
	ASSERT( iCharIndex >= 0 );
	return iCharIndex;
}

// return value is relative to gray arrows
static float GetClosestCharYOffset( float fFakeBeat )
{
	float f = fmodf(fFakeBeat, 1.0f);
	if( f > 0.5f )
		f -= 1;
	ASSERT( f>-0.5f && f<=0.5f );
	return -f;	
}

// return value is relative to gray arrows
static float GetClosestCharYPos( float fFakeBeat )
{
	return GetClosestCharYOffset(fFakeBeat)*g_fCharsSpacingY;
}


REGISTER_SCREEN_CLASS( ScreenNameEntry );
ScreenNameEntry::ScreenNameEntry()
{
		// DEBUGGING STUFF
//	GAMESTATE->m_CurGame = GAME_DANCE;
//	GAMESTATE->m_CurStyle = STYLE_DANCE_SINGLE;
//	GAMESTATE->m_PlayMode = PLAY_MODE_REGULAR;
//	GAMESTATE->m_bSideIsJoined[PLAYER_1] = true;
//	GAMESTATE->m_MasterPlayerNumber = PLAYER_1;
//	GAMESTATE->m_RankingCategory[PLAYER_1] = RANKING_A;
//	GAMESTATE->m_iRankingIndex[PLAYER_1] = 0;
}

void ScreenNameEntry::Init()
{
	Screen::Init();

	// update cache
	g_fCharsZoomSmall = CHARS_ZOOM_SMALL;
	g_fCharsZoomLarge = CHARS_ZOOM_LARGE;
	g_fCharsSpacingY = CHARS_SPACING_Y;
	g_fReceptorArrowsY = GRAY_ARROWS_Y;
	g_iNumCharsToDrawBehind = NUM_CHARS_TO_DRAW_BEHIND;
	g_iNumCharsToDrawTotal = NUM_CHARS_TO_DRAW_TOTAL;
	g_fFakeBeatsPerSec = FAKE_BEATS_PER_SEC;

	// reset Player and Song Options
	{
		FOREACH_PlayerNumber( p )
			PO_GROUP_CALL( GAMESTATE->m_pPlayerState[p]->m_PlayerOptions, ModsLevel_Stage, Init );
		SO_GROUP_CALL( GAMESTATE->m_SongOptions, ModsLevel_Stage, Init );
	}

	vector<GameState::RankingFeat> aFeats[NUM_PLAYERS];

	// Find out if players deserve to enter their name
	FOREACH_PlayerNumber( p )
	{
		GAMESTATE->GetRankingFeats( p, aFeats[p] );
		GAMESTATE->JoinPlayer( p );
		m_bStillEnteringName[p] = aFeats[p].size()>0;
	}

	if( !AnyStillEntering() )
	{
		/* Nobody made a high score. */
		PostScreenMessage( SM_GoToNextScreen, 0 );
		return;
	}

	bool IsOnRanking = ( (GAMESTATE->m_PlayMode == PLAY_MODE_NONSTOP || GAMESTATE->m_PlayMode == PLAY_MODE_ONI)
		&& !(GAMESTATE->m_pCurCourse->IsRanking()) );

	if( PREFSMAN->m_GetRankingName == RANKING_OFF || 
		(PREFSMAN->m_GetRankingName == RANKING_LIST && !IsOnRanking) )
	{
		// don't collect score due to ranking setting
		PostScreenMessage( SM_GoToNextScreen, 0 );
		return;
	}


	GAMESTATE->m_bGameplayLeadIn.Set( false );	// enable the gray arrows

	FOREACH_PlayerNumber( p )
	{
		// load last used ranking name if any
		Profile* pProfile = PROFILEMAN->GetProfile(p);
		if( pProfile && !pProfile->m_sLastUsedHighScoreName.empty() )
			 m_sSelectedName[p] = pProfile->m_sLastUsedHighScoreName;

		// resize string to MAX_RANKING_NAME_LENGTH
		m_sSelectedName[p] = ssprintf( "%*.*s", MAX_RANKING_NAME_LENGTH, MAX_RANKING_NAME_LENGTH, m_sSelectedName[p].c_str() );
		ASSERT( (int) m_sSelectedName[p].length() == MAX_RANKING_NAME_LENGTH );

		// don't load player if they aren't going to enter their name
		if( !m_bStillEnteringName[p] )
			continue;	// skip

		// remove modifiers that may have been on the last song
		PlayerOptions po;
		GAMESTATE->GetDefaultPlayerOptions( po );
		GAMESTATE->m_pPlayerState[p]->m_PlayerOptions.Assign( ModsLevel_Stage, po );

		ASSERT( GAMESTATE->IsHumanPlayer(p) );	// they better be enabled if they made a high score!

		float fPlayerX = PLAYER_X(p,GAMESTATE->GetCurrentStyle()->m_StyleType);

		{
			LockNoteSkin l( GAMESTATE->m_pPlayerState[p]->m_PlayerOptions.GetCurrent().m_sNoteSkin );

			m_ReceptorArrowRow[p].Load( GAMESTATE->m_pPlayerState[p], 0 );
			m_ReceptorArrowRow[p].SetX( fPlayerX );
			m_ReceptorArrowRow[p].SetY( SCREEN_TOP + 100 );
			this->AddChild( &m_ReceptorArrowRow[p] );
		}


		const Style* pStyle = GAMESTATE->GetCurrentStyle();

		m_ColToStringIndex[p].insert(m_ColToStringIndex[p].begin(), pStyle->m_iColsPerPlayer, -1);
		int CurrentStringIndex = 0;

		for( int iCol=0; iCol<pStyle->m_iColsPerPlayer; iCol++ )
		{
			if( CurrentStringIndex == MAX_RANKING_NAME_LENGTH )
				continue; /* We have enough columns. */

			/* Find out if this column is associated with the START menu button. */
			GameInput gi = GAMESTATE->GetCurrentStyle()->StyleInputToGameInput( iCol, p );
			MenuButton mb = INPUTMAPPER->GameToMenu( gi );
			if( mb == MENU_BUTTON_START )
				continue;
			m_ColToStringIndex[p][iCol] = CurrentStringIndex++;

			float ColX = fPlayerX + pStyle->m_ColumnInfo[p][iCol].fXOffset;

			m_textSelectedChars[p][iCol].LoadFromFont( THEME->GetPathF("ScreenNameEntry","letters") );
			m_textSelectedChars[p][iCol].SetX( ColX );
			m_textSelectedChars[p][iCol].SetY( GRAY_ARROWS_Y );
			m_textSelectedChars[p][iCol].RunCommands( SELECTED_CHARS_COMMAND );
			m_textSelectedChars[p][iCol].SetZoom( CHARS_ZOOM_LARGE );
			if( iCol < (int)m_sSelectedName[p].length() )
				m_textSelectedChars[p][iCol].SetText( m_sSelectedName[p].substr(iCol,1) );
			this->AddChild( &m_textSelectedChars[p][iCol] );		// draw these manually
			
			m_textScrollingChars[p][iCol].LoadFromFont( THEME->GetPathF("ScreenNameEntry","letters") );
			m_textScrollingChars[p][iCol].SetX( ColX );
			m_textScrollingChars[p][iCol].SetY( GRAY_ARROWS_Y );
			m_textScrollingChars[p][iCol].RunCommands( SCROLLING_CHARS_COMMAND );
			//this->AddChild( &m_textScrollingChars[p][iCol] );	// draw these manually
		}

		m_textCategory[p].LoadFromFont( THEME->GetPathF("ScreenNameEntry","category") );
		m_textCategory[p].SetX( fPlayerX );
		m_textCategory[p].SetY( CATEGORY_Y );
		m_textCategory[p].SetZoom( CATEGORY_ZOOM );
		RString joined;
		for( unsigned j = 0; j < aFeats[p].size(); ++j )
		{
			if( j )
				joined += "\n";
			joined += aFeats[p][j].Feat;
		}

		m_textCategory[p].SetText( joined );
		this->AddChild( &m_textCategory[p] );
	}


	m_Timer.Load();
	if( !PREFSMAN->m_bMenuTimer )
		m_Timer.Disable();
	else
		m_Timer.SetSeconds(TIMER_SECONDS);
	m_Timer.SetXY( TIMER_X, TIMER_Y );
	this->AddChild( &m_Timer );

	m_In.Load( THEME->GetPathB("ScreenNameEntry","in") );
	m_In.StartTransitioning();
	m_In.SetDrawOrder( DRAW_ORDER_TRANSITIONS );
	this->AddChild( &m_In );

	m_Out.Load( THEME->GetPathB("ScreenNameEntry","out") );
	m_Out.SetDrawOrder( DRAW_ORDER_TRANSITIONS );
	this->AddChild( &m_Out );

	this->SortByDrawOrder();

	m_soundStep.Load( THEME->GetPathS("ScreenNameEntry","step") );

	m_fFakeBeat = 0;
}

bool ScreenNameEntry::AnyStillEntering() const
{
	FOREACH_PlayerNumber( p )
		if( m_bStillEnteringName[p] )
			return true;
	return false;
}

ScreenNameEntry::~ScreenNameEntry()
{
	LOG->Trace( "ScreenNameEntry::~ScreenNameEntry()" );
}

void ScreenNameEntry::Update( float fDelta )
{
	if( m_bFirstUpdate )
		SOUND->PlayOnceFromDir( ANNOUNCER->GetPathTo("name entry") );

	m_fFakeBeat += fDelta * FAKE_BEATS_PER_SEC;
	GAMESTATE->m_fSongBeat = m_fFakeBeat;

	Screen::Update(fDelta);
}

void ScreenNameEntry::DrawPrimitives()
{
	Screen::DrawPrimitives();

	int iClosestIndex = GetClosestCharIndex( m_fFakeBeat );
	int iStartDrawingIndex = iClosestIndex - NUM_CHARS_TO_DRAW_BEHIND;
	iStartDrawingIndex += NUM_NAME_CHARS;	// make positive

	const Style* pStyle = GAMESTATE->GetCurrentStyle();

	FOREACH_PlayerNumber( p )
	{
		if( !m_bStillEnteringName[p] )
			continue;	// don't draw scrolling arrows

		float fY = GRAY_ARROWS_Y + GetClosestCharYPos(m_fFakeBeat) - g_iNumCharsToDrawBehind*g_fCharsSpacingY;
		int iCharIndex = iStartDrawingIndex % NUM_NAME_CHARS;
		
		for( int i=0; i<NUM_CHARS_TO_DRAW_TOTAL; i++ )
		{
			char c = NAME_CHARS[iCharIndex];
			for( int t=0; t<pStyle->m_iColsPerPlayer; t++ )
			{
				if( m_ColToStringIndex[p][t] == -1 )
					continue;

				m_textScrollingChars[p][t].SetText( ssprintf("%c",c) );	// why doens't CStdStr have a contructor that takes a char?
				m_textScrollingChars[p][t].SetY( fY );
				float fZoom = g_fCharsZoomSmall;
				if( iCharIndex==iClosestIndex )
					fZoom = SCALE(fabsf(GetClosestCharYOffset(m_fFakeBeat)),0,0.5f,g_fCharsZoomLarge,g_fCharsZoomSmall);
				m_textScrollingChars[p][t].SetZoom(fZoom);
				float fAlpha = 1;
				if( i==0 )
					fAlpha *= SCALE(GetClosestCharYOffset(m_fFakeBeat),-0.5f,0.f,0.f,1.f);
				if( i==g_iNumCharsToDrawTotal-1 )
					fAlpha *= SCALE(GetClosestCharYOffset(m_fFakeBeat),0.f,0.5f,1.f,0.f);
				m_textScrollingChars[p][t].SetDiffuseAlpha( fAlpha  );
				m_textScrollingChars[p][t].Draw();
			}
			fY += g_fCharsSpacingY;
			iCharIndex = (iCharIndex+1) % NUM_NAME_CHARS;
		}


		for( int t=0; t<pStyle->m_iColsPerPlayer; t++ )
		{
			m_textSelectedChars[p][t].Draw();
		}
	}
}

void ScreenNameEntry::Input( const InputEventPlus &input )
{
	LOG->Trace( "ScreenNameEntry::Input()" );

	if( m_In.IsTransitioning() || m_Out.IsTransitioning() )
		return;	

	if( input.type != IET_FIRST_PRESS )
		return;		// ignore

	const int iCol = GAMESTATE->GetCurrentStyle()->GameInputToColumn( input.GameI );
	if( iCol != Column_Invalid && m_bStillEnteringName[input.pn] )
	{
		int iStringIndex = m_ColToStringIndex[input.pn][iCol];
		if( iStringIndex != -1 )
		{
			m_ReceptorArrowRow[input.pn].Step( iCol, TNS_W1 );
			m_soundStep.Play();
			char c = NAME_CHARS[GetClosestCharIndex(m_fFakeBeat)];
			m_textSelectedChars[input.pn][iCol].SetText( ssprintf("%c",c) );
			m_sSelectedName[input.pn][iStringIndex] = c;
		}
	}

	Screen::Input( input );
}

void ScreenNameEntry::HandleScreenMessage( const ScreenMessage SM )
{
	if( SM == SM_MenuTimer )
	{
		if( !m_Out.IsTransitioning() )
		{
			InputEventPlus iep;
			FOREACH_PlayerNumber( p )
			{
				iep.pn = p;
				this->MenuStart( iep );
			}
		}
	}
	else if( SM == SM_GoToNextScreen )
	{
		// There shouldn't be NameEntry in event mode.  -Chris
//		/* Hack: go back to the select course screen in event mode. */
//		if( GAMESTATE->GetEventMode() && GAMESTATE->IsCourseMode() )
//		{
//			SCREENMAN->SetNewScreen( "ScreenSelectCourse" );
//			break;
//		}
	}

	Screen::HandleScreenMessage( SM );
}


void ScreenNameEntry::MenuStart( const InputEventPlus &input )
{
	PlayerNumber pn = input.pn;
	
	if( !m_bStillEnteringName[pn] )
		return;
	m_bStillEnteringName[pn] = false;

	m_soundStep.Play();

	// save last used ranking name
	Profile* pProfile = PROFILEMAN->GetProfile(pn);
	pProfile->m_sLastUsedHighScoreName = m_sSelectedName[pn];

	TrimRight( m_sSelectedName[pn], " " );
	TrimLeft( m_sSelectedName[pn], " " );

	GAMESTATE->StoreRankingName( pn, m_sSelectedName[pn] );

	if( !AnyStillEntering() && !m_Out.IsTransitioning() )
		m_Out.StartTransitioning( SM_GoToNextScreen );
}

/*
 * (c) 2001-2004 Chris Danford
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

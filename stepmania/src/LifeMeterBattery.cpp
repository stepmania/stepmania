#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: LifeMeterBattery

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "LifeMeterBattery.h"
#include "PrefsManager.h"
#include "GameState.h"
#include "ThemeManager.h"
#include "Notes.h"


const float	BATTERY_X[NUM_PLAYERS]	=	{ -92, +92 };

const float NUM_X[NUM_PLAYERS]		=	{ BATTERY_X[0], BATTERY_X[1] };
const float NUM_Y					=	+2;

const float PERCENT_X[NUM_PLAYERS]	=	{ +20, -20 };
const float PERCENT_Y				=	0;

const float BATTERY_BLINK_TIME		= 1.2f;


LifeMeterBattery::LifeMeterBattery()
{
	m_iLivesLeft = GAMESTATE->m_SongOptions.m_iBatteryLives;
	m_iTrailingLivesLeft = m_iLivesLeft;
	m_bFailedEarlier = false;

	m_fBatteryBlinkTime = 0;

	
	m_soundGainLife.Load( THEME->GetPathTo("Sounds","gameplay oni gain life") );
	m_soundLoseLife.Load( THEME->GetPathTo("Sounds","gameplay oni lose life") );
}

void LifeMeterBattery::Load( PlayerNumber pn )
{
	LifeMeter::Load( pn );

	bool bPlayerEnabled = GAMESTATE->IsPlayerEnabled(pn);

	m_sprFrame.Load( THEME->GetPathTo("Graphics","gameplay lifemeter oni") );
	this->AddChild( &m_sprFrame );

	m_sprBattery.Load( THEME->GetPathTo("Graphics","gameplay lifemeter battery") );
	m_sprBattery.StopAnimating();
	if( bPlayerEnabled )
		this->AddChild( &m_sprBattery );

	m_textNumLives.LoadFromNumbers( THEME->GetPathTo("Numbers","gameplay battery life numbers") );
	m_textNumLives.SetDiffuse( RageColor(1,1,1,1) );
	m_textNumLives.TurnShadowOff();
	if( bPlayerEnabled )
		this->AddChild( &m_textNumLives );

	m_textPercent.LoadFromNumbers( THEME->GetPathTo("Numbers","gameplay battery percent numbers") );
	m_textPercent.TurnShadowOff();
	m_textPercent.SetZoom( 0.7f );
	m_textPercent.SetText( "00.0%" );
	if( bPlayerEnabled )
		this->AddChild( &m_textPercent );



	m_sprFrame.SetZoomX( pn==PLAYER_1 ? 1.0f : -1.0f );
	m_sprBattery.SetZoomX( pn==PLAYER_1 ? 1.0f : -1.0f );
	m_sprBattery.SetX( BATTERY_X[pn] );
	m_textNumLives.SetX( NUM_X[pn] );
	m_textNumLives.SetY( NUM_Y );
	m_textPercent.SetX( PERCENT_X[pn] );
	m_textPercent.SetY( PERCENT_Y );

	m_textPercent.SetDiffuse( PlayerToColor(pn) );	// light blue

	Refresh();
}

void LifeMeterBattery::OnSongEnded()
{
	if( m_bFailedEarlier )
		return;

	if( m_iLivesLeft < GAMESTATE->m_SongOptions.m_iBatteryLives )
	{
		m_iTrailingLivesLeft = m_iLivesLeft;
		m_iLivesLeft += ( GAMESTATE->m_pCurNotes[m_PlayerNumber]->GetMeter()>=8 ? 2 : 1 );
		m_iLivesLeft = min( m_iLivesLeft, GAMESTATE->m_SongOptions.m_iBatteryLives );
		m_soundGainLife.Play();
	}

	Refresh();
}


void LifeMeterBattery::ChangeLife( TapNoteScore score )
{
	if( m_bFailedEarlier )
		return;

	switch( score )
	{
	case TNS_MARVELOUS:
	case TNS_PERFECT:
	case TNS_GREAT:
		break;
	case TNS_GOOD:
	case TNS_BOO:
	case TNS_MISS:
		m_iTrailingLivesLeft = m_iLivesLeft;
		m_iLivesLeft--;
		m_soundLoseLife.Play();

		m_textNumLives.SetZoom( 1.5f );
		m_textNumLives.BeginTweening( 0.15f );
		m_textNumLives.SetTweenZoom( 1.0f );

		Refresh();
		m_fBatteryBlinkTime = BATTERY_BLINK_TIME;
		break;
	default:
		ASSERT(0);
	}
	if( m_iLivesLeft == 0 )
		m_bFailedEarlier = true;
}

void LifeMeterBattery::ChangeLife( HoldNoteScore score, TapNoteScore tscore )
{
	switch( score )
	{
	case HNS_OK:
		break;
	case HNS_NG:
		ChangeLife( TNS_MISS );		// NG is the same as a miss
		break;
	default:
		ASSERT(0);
	}
}

void LifeMeterBattery::OnDancePointsChange()
{
	int iActualDancePoints = GAMESTATE->m_CurStageStats.iActualDancePoints[m_PlayerNumber];
	int iPossibleDancePoints = GAMESTATE->m_CurStageStats.iPossibleDancePoints[m_PlayerNumber];
	iPossibleDancePoints = max( 1, iPossibleDancePoints );
	float fPercentDancePoints =  iActualDancePoints / (float)iPossibleDancePoints + 0.00001f;	// correct for rounding errors

//	LOG->Trace( "Actual %d, Possible %d, Percent %f\n", iActualDancePoints, iPossibleDancePoints, fPercentDancePoints );

	float fNumToDisplay = MAX( 0, fPercentDancePoints*100 );
	CString sNumToDisplay = ssprintf("%03.1f%%", fNumToDisplay);
	if( sNumToDisplay.GetLength() == 4 )
		sNumToDisplay = "0" + sNumToDisplay;
	m_textPercent.SetText( sNumToDisplay );
}


bool LifeMeterBattery::IsInDanger()
{
	return false;
}

bool LifeMeterBattery::IsHot()
{
	return false;
}

bool LifeMeterBattery::IsFailing()
{
	return m_bFailedEarlier;
}

bool LifeMeterBattery::FailedEarlier()
{
	return m_bFailedEarlier;
}

void LifeMeterBattery::Refresh()
{
	if( m_iLivesLeft <= 4 )
	{
		m_textNumLives.SetText( "" );
		m_sprBattery.SetState( max(m_iLivesLeft-1,0) );
	}
	else
	{
		m_textNumLives.SetText( ssprintf("x%d", m_iLivesLeft) );
		m_sprBattery.SetState( 3 );
	}
}

void LifeMeterBattery::Update( float fDeltaTime )
{
	LifeMeter::Update( fDeltaTime );

	if( m_fBatteryBlinkTime > 0 )
	{
		m_fBatteryBlinkTime -= fDeltaTime;
		int iFrame1 = m_iLivesLeft-1;
		int iFrame2 = m_iTrailingLivesLeft-1;
		
		int iFrameNo = (int(m_fBatteryBlinkTime*15)%2) ? iFrame1 : iFrame2;
		CLAMP( iFrameNo, 0, 3 );
		m_sprBattery.SetState( iFrameNo );

	}
	else
	{
		m_fBatteryBlinkTime = 0;
		int iFrameNo = m_iLivesLeft-1;
		CLAMP( iFrameNo, 0, 3 );
		m_sprBattery.SetState( iFrameNo );
	}
}

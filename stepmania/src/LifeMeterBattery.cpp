#include "stdafx.h"
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
	this->AddSubActor( &m_sprFrame );

	m_sprBattery.Load( THEME->GetPathTo("Graphics","gameplay lifemeter battery") );
	m_sprBattery.StopAnimating();
	if( bPlayerEnabled )
		this->AddSubActor( &m_sprBattery );

	m_textNumLives.LoadFromFont( THEME->GetPathTo("Fonts","header1") );
	m_textNumLives.SetDiffuseColor( D3DXCOLOR(1,1,1,1) );		// pink
	m_textNumLives.SetZoom( 1.1f );
	m_textNumLives.TurnShadowOff();
	if( bPlayerEnabled )
		this->AddSubActor( &m_textNumLives );

	m_textPercent.LoadFromFont( THEME->GetPathTo("Fonts","score numbers") );
	m_textPercent.TurnShadowOff();
	m_textPercent.SetZoom( 0.7f );
	m_textPercent.SetText( "00.0" );
	if( bPlayerEnabled )
		this->AddSubActor( &m_textPercent );



	m_sprFrame.SetZoomX( pn==PLAYER_1 ? 1.0f : -1.0f );
	m_sprBattery.SetZoomX( pn==PLAYER_1 ? 1.0f : -1.0f );
	m_sprBattery.SetX( BATTERY_X[pn] );
	m_textNumLives.SetX( NUM_X[pn] );
	m_textNumLives.SetY( NUM_Y );
	m_textPercent.SetX( PERCENT_X[pn] );
	m_textPercent.SetY( PERCENT_Y );

	m_textPercent.SetDiffuseColor( PlayerToColor(pn) );	// light blue

	Refresh();
}

void LifeMeterBattery::SongEnded()
{
	if( m_bFailedEarlier )
		return;

	m_iTrailingLivesLeft = m_iLivesLeft;
	m_iLivesLeft += ( GAMESTATE->m_pCurNotes[m_PlayerNumber]->m_iMeter>=8 ? 2 : 1 );
	m_iLivesLeft = min( m_iLivesLeft, GAMESTATE->m_SongOptions.m_iBatteryLives );
	m_soundGainLife.Play();

	Refresh();
}


void LifeMeterBattery::ChangeLife( TapNoteScore score )
{
	if( m_bFailedEarlier )
		return;

	switch( score )
	{
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
		m_textNumLives.SetTweenZoom( 1.1f );

		Refresh();
		m_fBatteryBlinkTime = BATTERY_BLINK_TIME;
		break;
	}
	if( m_iLivesLeft == 0 )
		m_bFailedEarlier = true;
}

void LifeMeterBattery::OnDancePointsChange()
{
	int iActualDancePoints = GAMESTATE->m_iActualDancePoints[m_PlayerNumber];
	int iPossibleDancePoints = GAMESTATE->m_iPossibleDancePoints[m_PlayerNumber];
	iPossibleDancePoints = max( 1, iPossibleDancePoints );
	float fPercentDancePoints =  iActualDancePoints / (float)iPossibleDancePoints + 0.00001f;	// correct for rounding errors

	printf( "Actual %d, Possible %d, Percent %f\n", iActualDancePoints, iPossibleDancePoints, fPercentDancePoints );

	float fNumToDisplay = MAX( 0, fPercentDancePoints*100 );
	CString sNumToDisplay = ssprintf("%03.1f", fNumToDisplay);
	if( sNumToDisplay.GetLength() == 3 )
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
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

const float PERCENT_X[NUM_PLAYERS]	=	{ +28, -28 };
const float PERCENT_Y				=	0;

const float BATTERY_BLINK_TIME		= 1.2f;


LifeMeterBattery::LifeMeterBattery()
{
	m_iMaxLives = 3;
	m_iLivesLeft = m_iMaxLives;
	m_bFailedEarlier = false;

	m_fBatteryBlinkTime = 0;

	m_sprFrame.Load( THEME->GetPathTo(GRAPHIC_GAMEPLAY_LIFEMETER_ONI) );
	this->AddSubActor( &m_sprFrame );

	m_sprBattery.Load( THEME->GetPathTo(GRAPHIC_GAMEPLAY_LIFEMETER_BATTERY) );
	m_sprBattery.StopAnimating();
	this->AddSubActor( &m_sprBattery );

	m_textNumLives.Load( THEME->GetPathTo(FONT_HEADER1) );
	m_textNumLives.SetDiffuseColor( D3DXCOLOR(1,1,1,1) );		// pink
	m_textNumLives.SetZoom( 1.1f );
	m_textNumLives.TurnShadowOff();
	this->AddSubActor( &m_textNumLives );

	m_textPercent.Load( THEME->GetPathTo(FONT_SCORE_NUMBERS) );
	m_textPercent.TurnShadowOff();
	m_textPercent.SetZoom( 0.7f );
	m_textPercent.SetText( "0.00" );
	this->AddSubActor( &m_textPercent );
	
	m_soundGainLife.Load( THEME->GetPathTo(SOUND_GAMEPLAY_ONI_GAIN_LIFE) );
	m_soundLoseLife.Load( THEME->GetPathTo(SOUND_GAMEPLAY_ONI_LOSE_LIFE) );

	Refresh();
}

void LifeMeterBattery::Load( PlayerNumber p, const PlayerOptions &po )
{
	LifeMeter::Load( p, po );

	m_sprFrame.SetZoomX( p==PLAYER_1 ? 1.0f : -1.0f );
	m_sprBattery.SetZoomX( p==PLAYER_1 ? 1.0f : -1.0f );
	m_sprBattery.SetX( BATTERY_X[p] );
	m_textNumLives.SetX( NUM_X[p] );
	m_textNumLives.SetY( NUM_Y );
	m_textPercent.SetX( PERCENT_X[p] );
	m_textPercent.SetY( PERCENT_Y );

	m_textPercent.SetDiffuseColor( PlayerToColor(p) );	// light blue

}

void LifeMeterBattery::NextSong( Song* pSong )
{
	if( m_bFailedEarlier )
		return;

	m_iLivesLeft++;
	m_soundGainLife.Play();

	Refresh();
}

void LifeMeterBattery::ChangeLife( TapNoteScore score )
{
	if( m_bFailedEarlier )
		return;

	if( GAMESTATE->m_aGameplayStatistics.GetSize() > 0 )
	{
		int iActualDancePoints = 0;
		for( int i=0; i<GAMESTATE->m_aGameplayStatistics.GetSize(); i++ )
			iActualDancePoints += GAMESTATE->m_aGameplayStatistics[i].iActualDancePoints[m_PlayerNumber];
		int iPossibleDancePoints = GAMESTATE->m_iCoursePossibleDancePoints;
		float fPercentDancePoints =  iActualDancePoints / (float)iPossibleDancePoints + 0.0001f;	// correct for rounding errors
		m_textPercent.SetText( ssprintf("%1.2f", fPercentDancePoints) );
	}

	switch( score )
	{
	case TNS_PERFECT:
	case TNS_GREAT:
		break;
	case TNS_GOOD:
	case TNS_BOO:
	case TNS_MISS:
		m_iLivesLeft--;
		m_soundLoseLife.Play();
		Refresh();
		m_fBatteryBlinkTime = BATTERY_BLINK_TIME;
		break;
	}
	if( m_iLivesLeft == -1 )
		m_bFailedEarlier = true;
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
	if( m_iLivesLeft <= 3 )
	{
		m_textNumLives.SetText( "" );
		m_sprBattery.SetState( max(m_iLivesLeft,0) );
	}
	else
	{
		m_textNumLives.SetText( ssprintf("x%d", m_iLivesLeft) );
		m_sprBattery.SetState( 3 );
	}
}

void LifeMeterBattery::Update( float fDeltaTime )
{
	if( m_iLivesLeft == -1 )
	{
		m_sprBattery.SetState( 0 );
	}
	else if( m_fBatteryBlinkTime > 0 )
	{
		m_fBatteryBlinkTime -= fDeltaTime;
		int iFrameNo = m_iLivesLeft + int(m_fBatteryBlinkTime*15)%2;
		CLAMP( iFrameNo, 0, 3 );
		m_sprBattery.SetState( iFrameNo );


		if( m_fBatteryBlinkTime < 0 )
		{
			m_fBatteryBlinkTime = 0;
			m_sprBattery.SetState( max(m_iLivesLeft,0) );
		}
	}
}
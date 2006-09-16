#include "global.h"
#include "LifeMeterBattery.h"
#include "GameState.h"
#include "ThemeManager.h"
#include "Steps.h"
#include "PlayerState.h"
#include "Course.h"

const float	BATTERY_X[NUM_PLAYERS]	=	{ -92, +92 };

const float NUM_X[NUM_PLAYERS]		=	{ BATTERY_X[0], BATTERY_X[1] };
const float NUM_Y					=	+2;

const float BATTERY_BLINK_TIME		= 1.2f;


LifeMeterBattery::LifeMeterBattery()
{
	m_iLivesLeft = GAMESTATE->m_SongOptions.GetStage().m_iBatteryLives;
	m_iTrailingLivesLeft = m_iLivesLeft;

	m_fBatteryBlinkTime = 0;

	m_soundGainLife.Load( THEME->GetPathS("LifeMeterBattery","gain") );
	m_soundLoseLife.Load( THEME->GetPathS("LifeMeterBattery","lose"),true );
}

void LifeMeterBattery::Load( const PlayerState *pPlayerState, PlayerStageStats *pPlayerStageStats )
{
	LifeMeter::Load( pPlayerState, pPlayerStageStats );

	bool bPlayerEnabled = GAMESTATE->IsPlayerEnabled( pPlayerState );

	m_sprFrame.Load( THEME->GetPathG("LifeMeterBattery","frame") );
	this->AddChild( &m_sprFrame );

	m_sprBattery.Load( THEME->GetPathG("LifeMeterBattery","lives 1x4") );
	m_sprBattery.StopAnimating();
	if( bPlayerEnabled )
		this->AddChild( &m_sprBattery );

	m_textNumLives.LoadFromFont( THEME->GetPathF("LifeMeterBattery", "lives") );
	m_textNumLives.SetDiffuse( RageColor(1,1,1,1) );
	m_textNumLives.SetShadowLength( 0 );
	if( bPlayerEnabled )
		this->AddChild( &m_textNumLives );

	PlayerNumber pn = pPlayerState->m_PlayerNumber;

	m_sprFrame.SetZoomX( pn==PLAYER_1 ? 1.0f : -1.0f );
	m_sprBattery.SetZoomX( pn==PLAYER_1 ? 1.0f : -1.0f );
	m_sprBattery.SetX( BATTERY_X[pn] );
	m_textNumLives.SetX( NUM_X[pn] );
	m_textNumLives.SetY( NUM_Y );

	if( bPlayerEnabled )
	{
		m_Percent.Load( pPlayerState, pPlayerStageStats, "LifeMeterBattery Percent", true );
		this->AddChild( &m_Percent );
	}

	Refresh();
}

void LifeMeterBattery::OnSongEnded()
{
	if( m_pPlayerStageStats->bFailedEarlier )
		return;

	if( m_iLivesLeft < GAMESTATE->m_SongOptions.GetSong().m_iBatteryLives )
	{
		m_iTrailingLivesLeft = m_iLivesLeft;
		PlayerNumber pn = m_pPlayerState->m_PlayerNumber;
		const Course *pCourse = GAMESTATE->m_pCurCourse;
		
		if( pCourse && pCourse->m_vEntries[GAMESTATE->GetCourseSongIndex()].iGainLives > -1 )
			m_iLivesLeft += pCourse->m_vEntries[GAMESTATE->GetCourseSongIndex()].iGainLives;
		else
			m_iLivesLeft += ( GAMESTATE->m_pCurSteps[pn]->GetMeter()>=8 ? 2 : 1 );
		m_iLivesLeft = min( m_iLivesLeft, GAMESTATE->m_SongOptions.GetSong().m_iBatteryLives );

		if( m_iTrailingLivesLeft < m_iLivesLeft )
			m_soundGainLife.Play();
	}

	Refresh();
}


void LifeMeterBattery::ChangeLife( TapNoteScore score )
{
	if( m_pPlayerStageStats->bFailedEarlier )
		return;

	switch( score )
	{
	case TNS_W1:
	case TNS_W2:
	case TNS_W3:
		break;
	case TNS_W4:
	case TNS_W5:
	case TNS_Miss:
	case TNS_HitMine:
		m_iTrailingLivesLeft = m_iLivesLeft;
		m_iLivesLeft--;
		m_soundLoseLife.Play();

		m_textNumLives.SetZoom( 1.5f );
		m_textNumLives.BeginTweening( 0.15f );
		m_textNumLives.SetZoom( 1.0f );

		Refresh();
		m_fBatteryBlinkTime = BATTERY_BLINK_TIME;
		break;
	default:
		ASSERT(0);
	}
	if( m_iLivesLeft == 0 )
		m_pPlayerStageStats->bFailedEarlier = true;
}

void LifeMeterBattery::ChangeLife( HoldNoteScore score, TapNoteScore tscore )
{
	switch( score )
	{
	case HNS_Held:
		break;
	case HNS_LetGo:
		ChangeLife( TNS_Miss );		// LetGo is the same as a miss
		break;
	default:
		ASSERT(0);
	}
}

void LifeMeterBattery::ChangeLife( float fDeltaLifePercent )
{
}

void LifeMeterBattery::OnDancePointsChange()
{
}


bool LifeMeterBattery::IsInDanger() const
{
	return false;
}

bool LifeMeterBattery::IsHot() const
{
	return false;
}

bool LifeMeterBattery::IsFailing() const
{
	return m_pPlayerStageStats->bFailedEarlier;
}

float LifeMeterBattery::GetLife() const
{
	if( !GAMESTATE->m_SongOptions.GetSong().m_iBatteryLives )
		return 1;

	return float(m_iLivesLeft) / GAMESTATE->m_SongOptions.GetSong().m_iBatteryLives;
}

void LifeMeterBattery::ForceFail()
{
	m_iLivesLeft = 0;
	Refresh();
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
		m_textNumLives.SetText( ssprintf("x%d", m_iLivesLeft-1) );
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

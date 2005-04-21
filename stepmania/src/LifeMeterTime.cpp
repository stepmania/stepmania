#include "global.h"
#include "LifeMeterTime.h"
#include "GameState.h"
#include "ThemeManager.h"
#include "Steps.h"
#include "StatsManager.h"
#include "ActorUtil.h"
#include "Course.h"
#include "PrefsManager.h"

const float MAX_LIFE_SECONDS = 3*60;

LifeMeterTime::LifeMeterTime()
{
	m_fLifeRemainingSeconds = 0;
}

void LifeMeterTime::Load( PlayerNumber pn )
{
	LifeMeter::Load( pn );

	const CString sType = "LifeMeterTime";

	bool bPlayerEnabled = GAMESTATE->IsPlayerEnabled(pn);

	m_sprFrame.Load( THEME->GetPathG(sType,"frame") );
	m_sprFrame->SetName( "Frame" );
	this->AddChild( m_sprFrame );
	ActorUtil::OnCommand( m_sprFrame, sType );

	m_textTimeRemaining.LoadFromFont( THEME->GetPathF(sType, "TimeRemaining") );
	m_textTimeRemaining.SetName( "TimeRemaining" );
	this->AddChild( &m_textTimeRemaining );
	ActorUtil::OnCommand( m_textTimeRemaining, sType );

	m_Meter.SetName( "Meter" );
	m_Meter.Load( THEME->GetPathG(sType,"Stream"), THEME->GetMetricF(sType,"StreamWidth"), THEME->GetPathG(sType,"Tip") );
	this->AddChild( &m_Meter );
	ActorUtil::OnCommand( m_Meter, sType );

	FOREACH_TapNoteScore( tns )
		m_TapCommands[tns] = THEME->GetMetricA( sType, TapNoteScoreToString(tns)+"Command" );
	m_GainLife = THEME->GetMetricA( sType, "GainLifeCommand" );

	m_soundGainLife.Load( THEME->GetPathS(sType,"GainLife") );
}

void LifeMeterTime::OnLoadSong()
{
	if( STATSMAN->m_CurStageStats.m_player[m_PlayerNumber].bFailedEarlier )
		return;

	Course* pCourse = GAMESTATE->m_pCurCourse;
	ASSERT( pCourse );
	const CourseEntry *pEntry = &pCourse->m_entries[GAMESTATE->GetCourseSongIndex()];
	m_fLifeRemainingSeconds += pEntry->fGainSeconds;
	CLAMP( m_fLifeRemainingSeconds, 0, MAX_LIFE_SECONDS );

	m_soundGainLife.Play();

	m_textTimeRemaining.RunCommands( m_GainLife );
}


void LifeMeterTime::ChangeLife( TapNoteScore score )
{
	if( STATSMAN->m_CurStageStats.m_player[m_PlayerNumber].bFailedEarlier )
		return;

	switch( score )
	{
	case TNS_MARVELOUS:	m_fLifeRemainingSeconds += PREFSMAN->m_fTimeMeterSecondsChangeMarvelous;	break;
	case TNS_PERFECT:	m_fLifeRemainingSeconds += PREFSMAN->m_fTimeMeterSecondsChangePerfect;		break;
	case TNS_GREAT:		m_fLifeRemainingSeconds += PREFSMAN->m_fTimeMeterSecondsChangeGreat;		break;
	case TNS_GOOD:		m_fLifeRemainingSeconds += PREFSMAN->m_fTimeMeterSecondsChangeGood;			break;
	case TNS_BOO:		m_fLifeRemainingSeconds += PREFSMAN->m_fTimeMeterSecondsChangeBoo;			break;
	case TNS_MISS:		m_fLifeRemainingSeconds += PREFSMAN->m_fTimeMeterSecondsChangeMiss;			break;
	default:	ASSERT(0);
	}
	CLAMP( m_fLifeRemainingSeconds, 0, MAX_LIFE_SECONDS );

	m_textTimeRemaining.RunCommands( m_TapCommands[score] );

	if( m_fLifeRemainingSeconds == 0 )
		STATSMAN->m_CurStageStats.m_player[m_PlayerNumber].bFailedEarlier = true;
}

void LifeMeterTime::ChangeLife( HoldNoteScore score, TapNoteScore tscore )
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

void LifeMeterTime::ChangeLife( float fDeltaLifePercent )
{
}

void LifeMeterTime::OnDancePointsChange()
{
}


bool LifeMeterTime::IsInDanger() const
{
	return false;
}

bool LifeMeterTime::IsHot() const
{
	return false;
}

bool LifeMeterTime::IsFailing() const
{
	return STATSMAN->m_CurStageStats.m_player[m_PlayerNumber].bFailedEarlier;
}

void LifeMeterTime::Update( float fDeltaTime )
{
	LifeMeter::Update( fDeltaTime );
	
	m_textTimeRemaining.SetText( SecondsToMMSSMsMs(m_fLifeRemainingSeconds) );
	m_Meter.SetPercent( m_fLifeRemainingSeconds/MAX_LIFE_SECONDS );
}

float LifeMeterTime::GetLife() const
{
	return m_fLifeRemainingSeconds/MAX_LIFE_SECONDS;
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

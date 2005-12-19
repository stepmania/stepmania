#include "global.h"
#include "BPMDisplay.h"
#include "RageUtil.h"
#include "GameConstantsAndTypes.h"
#include "GameState.h"
#include "Course.h"
#include "Style.h"
#include "ActorUtil.h"
#include "CommonMetrics.h"
#include "song.h"

#include <limits.h>

BPMDisplay::BPMDisplay()
{
	m_fBPMFrom = m_fBPMTo = 0;
	m_iCurrentBPM = 0;
	m_BPMS.push_back(0);
	m_fPercentInState = 0;
	m_fCycleTime = 1.0f;
}

void BPMDisplay::Load()
{
	NORMAL_COLOR.Load( m_sName, "NormalColor");
	CHANGE_COLOR.Load( m_sName, "ChangeColor" );
	EXTRA_COLOR.Load( m_sName, "ExtraColor" );
	CYCLE.Load( m_sName, "Cycle" );
	SEPARATOR.Load( m_sName, "Separator" );
	NO_BPM_TEXT.Load( m_sName, "NoBPMText" );

	m_textBPM.SetName( "Text" );
	m_textBPM.LoadFromFont( THEME->GetPathF(m_sName,"bpm") );
	SET_XY_AND_ON_COMMAND( m_textBPM );
	m_textBPM.SetDiffuse( NORMAL_COLOR );
	this->AddChild( &m_textBPM );

	m_sprLabel.Load( THEME->GetPathG(m_sName,"label") );
	m_sprLabel->SetName( "Label" );
	SET_XY_AND_ON_COMMAND( m_sprLabel );
	m_sprLabel->SetDiffuse( NORMAL_COLOR );
	this->AddChild( m_sprLabel );
}

float BPMDisplay::GetActiveBPM() const
{
	return m_fBPMTo + (m_fBPMFrom-m_fBPMTo)*m_fPercentInState;
}

void BPMDisplay::Update( float fDeltaTime ) 
{ 
	ActorFrame::Update( fDeltaTime ); 

	if( !(bool)CYCLE )
		return;
	if( m_BPMS.size() == 0 )
		return; /* no bpm */

	m_fPercentInState -= fDeltaTime / m_fCycleTime;
	if( m_fPercentInState < 0 )
	{
		// go to next state
		m_fPercentInState = 1;		// reset timer

		m_iCurrentBPM = (m_iCurrentBPM + 1) % m_BPMS.size();
		m_fBPMFrom = m_fBPMTo;
		m_fBPMTo = m_BPMS[m_iCurrentBPM];

		if(m_fBPMTo == -1)
		{
			m_fBPMFrom = -1;
			m_textBPM.SetText( (RandomFloat(0,1)>0.90f) ? CString("xxx") : ssprintf("%03.0f",RandomFloat(0,600)) ); 
		}
		else if(m_fBPMFrom == -1)
		{
			m_fBPMFrom = m_fBPMTo;
		}
	}

	// update m_textBPM
	if( m_fBPMTo != -1)
	{
		const float fActualBPM = GetActiveBPM();
		m_textBPM.SetText( ssprintf("%03.0f", fActualBPM) );
	}
}


void BPMDisplay::SetBPMRange( const DisplayBpms &bpms )
{
	ASSERT( !bpms.vfBpms.empty() );

	m_BPMS.clear();

	const vector<float> &BPMS = bpms.vfBpms;

	bool AllIdentical = true;
	for( unsigned i = 0; i < BPMS.size(); ++i )
	{
		if( i > 0 && BPMS[i] != BPMS[i-1] )
			AllIdentical = false;
	}

	if( !(bool)CYCLE )
	{
		int MinBPM = INT_MAX;
		int MaxBPM = INT_MIN;
		for( unsigned i = 0; i < BPMS.size(); ++i )
		{
			MinBPM = min( MinBPM, (int) roundf(BPMS[i]) );
			MaxBPM = max( MaxBPM, (int) roundf(BPMS[i]) );
		}
		if( MinBPM == MaxBPM )
		{
			if( MinBPM == -1 )
				m_textBPM.SetText( "..." ); // random
			else
				m_textBPM.SetText( ssprintf("%i", MinBPM) );
		}
		else
		{
			m_textBPM.SetText( ssprintf("%i%s%i", MinBPM, SEPARATOR.GetValue().c_str(), MaxBPM) );
		}
	}
	else
	{
		for( unsigned i = 0; i < BPMS.size(); ++i )
		{
			m_BPMS.push_back(BPMS[i]);
			if( BPMS[i] != -1 )
				m_BPMS.push_back(BPMS[i]); /* hold */
		}

		m_iCurrentBPM = min(1u, m_BPMS.size()); /* start on the first hold */
		m_fBPMFrom = BPMS[0];
		m_fBPMTo = BPMS[0];
		m_fPercentInState = 1;
	}

	m_textBPM.StopTweening();
	m_sprLabel->StopTweening();
	m_textBPM.BeginTweening(0.5f);
	m_sprLabel->BeginTweening(0.5f);
	if( GAMESTATE->IsExtraStage() || GAMESTATE->IsExtraStage2() )
	{
		m_textBPM.SetDiffuse( EXTRA_COLOR );
		m_sprLabel->SetDiffuse( EXTRA_COLOR );		
	}
	else if( !AllIdentical )
	{
		m_textBPM.SetDiffuse( CHANGE_COLOR );
		m_sprLabel->SetDiffuse( CHANGE_COLOR );
	}
	else
	{
		m_textBPM.SetDiffuse( NORMAL_COLOR );
		m_sprLabel->SetDiffuse( NORMAL_COLOR );
	}
}

void BPMDisplay::CycleRandomly()
{
	DisplayBpms bpms;
	bpms.Add(-1);
	SetBPMRange( bpms );

	m_fCycleTime = 0.2f;
}

void BPMDisplay::NoBPM()
{
	m_BPMS.clear();
	m_textBPM.SetText( NO_BPM_TEXT ); 

	m_textBPM.SetDiffuse( NORMAL_COLOR );
	m_sprLabel->SetDiffuse( NORMAL_COLOR );
}

void BPMDisplay::SetBpmFromSong( const Song* pSong )
{
	ASSERT( pSong );
	switch( pSong->m_DisplayBPMType )
	{
	case Song::DISPLAY_ACTUAL:
	case Song::DISPLAY_SPECIFIED:
		{
			DisplayBpms bpms;
			pSong->GetDisplayBpms( bpms );
			SetBPMRange( bpms );
			m_fCycleTime = 1.0f;
		}
		break;
	case Song::DISPLAY_RANDOM:
		CycleRandomly();
		break;
	default:
		ASSERT(0);
	}
}

void BPMDisplay::SetBpmFromCourse( const Course* pCourse )
{
	ASSERT( pCourse );

	StepsType st = GAMESTATE->GetCurrentStyle()->m_StepsType;
	Trail *pTrail = pCourse->GetTrail( st );
	ASSERT( pTrail );

	DisplayBpms bpms;
	pTrail->GetDisplayBpms( bpms );
	
	SetBPMRange( bpms );
	m_fCycleTime = 0.2f;
}

void BPMDisplay::SetConstantBpm( float fBPM )
{
	DisplayBpms bpms;
	bpms.Add( fBPM );
	SetBPMRange( bpms );
}

void BPMDisplay::SetVarious()
{
	m_BPMS.clear();
	m_BPMS.push_back( -1 );
	m_textBPM.SetText( "Various" );
}

/*
 * (c) 2001-2002 Chris Danford
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

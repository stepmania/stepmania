#include "global.h"
/*
-----------------------------------------------------------------------------
 File: BPMDisplay.h

 Desc: A graphic displayed in the BPMDisplay during Dancing.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/

#include "BPMDisplay.h"
#include "RageUtil.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "GameState.h"
#include "ThemeManager.h"
#include "Course.h"
#include "StyleDef.h"


#define NORMAL_COLOR		THEME->GetMetricC("BPMDisplay","NormalColor")
#define CHANGE_COLOR		THEME->GetMetricC("BPMDisplay","ChangeColor")
#define EXTRA_COLOR			THEME->GetMetricC("BPMDisplay","ExtraColor")


BPMDisplay::BPMDisplay()
{
	m_fBPMFrom = m_fBPMTo = 0;
	m_iCurrentBPM = 0;
	m_BPMS.push_back(0);
	m_fPercentInState = 0;
	m_fCycleTime = 1.0f;

	m_textBPM.LoadFromNumbers( THEME->GetPathToN("BPMDisplay") );
	m_textBPM.EnableShadow( false );
	m_textBPM.SetHorizAlign( Actor::align_right );
	m_textBPM.SetDiffuse( NORMAL_COLOR );
	m_textBPM.SetXY( 0, 0 );

	m_sprLabel.Load( THEME->GetPathToG("BPMDisplay label") );
	m_sprLabel.EnableShadow( false );
	m_sprLabel.SetDiffuse( NORMAL_COLOR );
	m_sprLabel.SetHorizAlign( Actor::align_left );
	m_sprLabel.SetXY( 0, 0 );

	this->AddChild( &m_textBPM );
	this->AddChild( &m_sprLabel );
}

float BPMDisplay::GetActiveBPM() const
{
	return m_fBPMTo + (m_fBPMFrom-m_fBPMTo)*m_fPercentInState;
}

void BPMDisplay::Update( float fDeltaTime ) 
{ 
	ActorFrame::Update( fDeltaTime ); 

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
			m_textBPM.SetText( (RandomFloat(0,1)>0.90) ? "xxx" : ssprintf("%03.0f",RandomFloat(0,600)) ); 
		} else if(m_fBPMFrom == -1)
			m_fBPMFrom = m_fBPMTo;
	}

	// update m_textBPM
	if( m_fBPMTo != -1)
	{
		const float fActualBPM = GetActiveBPM();
		m_textBPM.SetText( ssprintf("%03.0f", fActualBPM) );
	}
}


void BPMDisplay::SetBPMRange( const vector<float> &BPMS )
{
	ASSERT( BPMS.size() );

	m_BPMS.clear();
	unsigned i;
	bool AllIdentical = true;
	for( i = 0; i < BPMS.size(); ++i )
	{
		if( i > 0 && m_BPMS[i] != m_BPMS[i-1] )
			AllIdentical = false;

		m_BPMS.push_back(BPMS[i]);
		if( BPMS[i] != -1 )
			m_BPMS.push_back(BPMS[i]); /* hold */
	}

	m_iCurrentBPM = min(1u, sizeof(m_BPMS)); /* start on the first hold */
	m_fBPMFrom = BPMS[0];
	m_fBPMTo = BPMS[0];
	m_fPercentInState = 1;

	m_textBPM.StopTweening();
	m_sprLabel.StopTweening();
	m_textBPM.BeginTweening(0.5f);
	m_sprLabel.BeginTweening(0.5f);

	if( GAMESTATE->IsExtraStage() || GAMESTATE->IsExtraStage2() )
	{
		m_textBPM.SetDiffuse( EXTRA_COLOR );
		m_sprLabel.SetDiffuse( EXTRA_COLOR );		
	}
	else if( !AllIdentical )
	{
		m_textBPM.SetDiffuse( CHANGE_COLOR );
		m_sprLabel.SetDiffuse( CHANGE_COLOR );
	}
	else
	{
		m_textBPM.SetDiffuse( NORMAL_COLOR );
		m_sprLabel.SetDiffuse( NORMAL_COLOR );
	}

}

void BPMDisplay::CycleRandomly()
{
	vector<float> BPMS;
	BPMS.push_back(-1);
	SetBPMRange( BPMS );

	m_fCycleTime = 0.2f;
}

void BPMDisplay::NoBPM()
{
	m_BPMS.clear();
	m_textBPM.SetText( "..." ); 

	m_textBPM.SetDiffuse( NORMAL_COLOR );
	m_sprLabel.SetDiffuse( NORMAL_COLOR );
}

void BPMDisplay::SetBPM( const Song* pSong )
{
	ASSERT( pSong );
	switch( pSong->m_DisplayBPMType )
	{
	case Song::DISPLAY_ACTUAL:
	case Song::DISPLAY_SPECIFIED:
		{
			float fMinBPM, fMaxBPM;
			pSong->GetDisplayBPM( fMinBPM, fMaxBPM );
			vector<float> BPMS;
			BPMS.push_back(fMinBPM);
			BPMS.push_back(fMaxBPM);
			SetBPMRange( BPMS );
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

void BPMDisplay::SetBPM( const Course* pCourse )
{
	vector<Song*> vSongs;
	vector<Notes*> vNotes;
	vector<CString> vsModifiers;
	pCourse->GetStageInfo( vSongs, vNotes, vsModifiers, GAMESTATE->GetCurrentStyleDef()->m_NotesType );

	ASSERT( vSongs.size() );

	vector<float> BPMS;
	for( unsigned i = 0; i < vSongs.size(); ++i )
	{
		if( pCourse->IsMysterySong(i) )
		{
			BPMS.push_back( -1 );
			continue;
		}

		Song *pSong = vSongs[i];
		ASSERT( pSong );
		switch( pSong->m_DisplayBPMType )
		{
		case Song::DISPLAY_ACTUAL:
		case Song::DISPLAY_SPECIFIED:
			{
				float fMinBPM, fMaxBPM;
				pSong->GetDisplayBPM( fMinBPM, fMaxBPM );
				BPMS.push_back( fMinBPM );
				if( fMinBPM != fMaxBPM )
					BPMS.push_back( fMaxBPM );
			}
			break;
		case Song::DISPLAY_RANDOM:
			BPMS.push_back( -1 );
			break;
		default:
			ASSERT(0);
		}
	}
	
	SetBPMRange( BPMS );
	m_fCycleTime = 0.2f;
}

void BPMDisplay::DrawPrimitives()
{
	ActorFrame::DrawPrimitives();
}

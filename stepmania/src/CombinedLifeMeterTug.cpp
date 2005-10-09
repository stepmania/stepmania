#include "global.h"
#include "CombinedLifeMeterTug.h"
#include "ThemeManager.h"
#include "GameState.h"
#include "PrefsManager.h"
#include "ThemeMetric.h"


ThemeMetric<float> METER_WIDTH		("CombinedLifeMeterTug","MeterWidth");


CombinedLifeMeterTug::CombinedLifeMeterTug() 
{
	FOREACH_PlayerNumber( p )
	{
		CString sStreamPath = THEME->GetPathG("CombinedLifeMeterTug",ssprintf("stream p%d",p+1));
		CString sTipPath = THEME->GetPathG("CombinedLifeMeterTug",ssprintf("tip p%d",p+1));
		m_Stream[p].Load( sStreamPath, METER_WIDTH, sTipPath );
		this->AddChild( &m_Stream[p] );
	}
	m_Stream[PLAYER_2].SetZoomX( -1 );

	m_sprSeparator.Load( THEME->GetPathG("CombinedLifeMeterTug","separator") );
	this->AddChild( m_sprSeparator );

	m_sprFrame.Load( THEME->GetPathG("CombinedLifeMeterTug","frame") );
	this->AddChild( m_sprFrame );
}

void CombinedLifeMeterTug::Update( float fDelta )
{
	float fPercentToShow = GAMESTATE->m_fTugLifePercentP1;
	CLAMP( fPercentToShow, 0.f, 1.f );

	m_Stream[PLAYER_1].SetPercent( fPercentToShow );
	m_Stream[PLAYER_2].SetPercent( 1-fPercentToShow );

	float fSeparatorX = SCALE( fPercentToShow, 0.f, 1.f, -METER_WIDTH/2.f, +METER_WIDTH/2.f );

	m_sprSeparator->SetX( fSeparatorX );

	ActorFrame::Update( fDelta );
}

void CombinedLifeMeterTug::ChangeLife( PlayerNumber pn, TapNoteScore score )
{
	float fPercentToMove = 0;
	switch( score )
	{
	case TNS_W1:			fPercentToMove = PREFSMAN->m_fTugMeterPercentChange.Get(SE_W1);		break;
	case TNS_W2:			fPercentToMove = PREFSMAN->m_fTugMeterPercentChange.Get(SE_W2);		break;
	case TNS_W3:			fPercentToMove = PREFSMAN->m_fTugMeterPercentChange.Get(SE_W3);		break;
	case TNS_W4:			fPercentToMove = PREFSMAN->m_fTugMeterPercentChange.Get(SE_W4);		break;
	case TNS_W5:			fPercentToMove = PREFSMAN->m_fTugMeterPercentChange.Get(SE_W5);		break;
	case TNS_Miss:		fPercentToMove = PREFSMAN->m_fTugMeterPercentChange.Get(SE_Miss);	break;
	case TNS_HitMine:	fPercentToMove = PREFSMAN->m_fTugMeterPercentChange.Get(SE_HitMine); break;
	default:	ASSERT(0);	break;
	}

	ChangeLife( pn, fPercentToMove );
}

void CombinedLifeMeterTug::ChangeLife( PlayerNumber pn, HoldNoteScore score, TapNoteScore tscore )
{
	float fPercentToMove = 0;
	switch( score )
	{
	case HNS_Held:			fPercentToMove = PREFSMAN->m_fTugMeterPercentChange.Get(SE_Held);	break;
	case HNS_LetGo:			fPercentToMove = PREFSMAN->m_fTugMeterPercentChange.Get(SE_LetGo);	break;
	default:	ASSERT(0);	break;
	}

	ChangeLife( pn, fPercentToMove );
}

void CombinedLifeMeterTug::ChangeLife( PlayerNumber pn, float fPercentToMove )
{
	if( PREFSMAN->m_bMercifulDrain  &&  fPercentToMove < 0 )
	{
		float fLifePercentage = 0;
		switch( pn )
		{
		case PLAYER_1:	fLifePercentage = GAMESTATE->m_fTugLifePercentP1;		break;
		case PLAYER_2:	fLifePercentage = 1 - GAMESTATE->m_fTugLifePercentP1;	break;
		default:	ASSERT(0);
		}

		/* Clamp the life meter only for calculating the multiplier. */
		fLifePercentage = clamp( fLifePercentage, 0.0f, 1.0f );
		fPercentToMove *= SCALE( fLifePercentage, 0.f, 1.f, 0.2f, 1.f);
	}

	switch( pn )
	{
	case PLAYER_1:	GAMESTATE->m_fTugLifePercentP1 += fPercentToMove;	break;
	case PLAYER_2:	GAMESTATE->m_fTugLifePercentP1 -= fPercentToMove;	break;
	default:	ASSERT(0);
	}
}

/*
 * (c) 2003-2004 Chris Danford
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

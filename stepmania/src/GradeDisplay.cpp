#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: GradeDisplay

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "GradeDisplay.h"
#include "RageUtil.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "ThemeManager.h"


const float SCROLL_TIME = 5.0f;
const float QUICK_SCROLL_TIME = .25f;
const int NUM_GRADE_FRAMES = 8;
const float GRADE_FRAME_HEIGHT = 1/(float)NUM_GRADE_FRAMES;
const float GRADES_TO_SCROLL = NUM_GRADE_FRAMES*4;


GradeDisplay::GradeDisplay()
{
	m_fTimeLeftInScroll = 0;
	m_bDoScrolling = 0;

	SetGrade( PLAYER_1, GRADE_NO_DATA );
}

bool GradeDisplay::Load( RageTextureID ID )
{
	ID.bStretch = true;
	Sprite::Load( ID );
	Sprite::StopAnimating();

	if( Sprite::GetNumStates() != 8  ||  Sprite::GetNumStates() != 16 )
		RageException::Throw( "The grade graphic '%s' must have either 8 or 16 frames.", ID.filename.GetString() );
	return true;
}

void GradeDisplay::Update( float fDeltaTime )
{
	Sprite::Update( fDeltaTime );

	if( m_bDoScrolling )
	{
		m_fTimeLeftInScroll -= fDeltaTime;
		m_fTimeLeftInScroll = max( 0, m_fTimeLeftInScroll );

		float fPercentIntoScrolling;
		if( m_bDoScrolling == 1)
		{
			fPercentIntoScrolling = 1 - (m_fTimeLeftInScroll/SCROLL_TIME);
			if( fPercentIntoScrolling < 0.75 )
				fPercentIntoScrolling = (fPercentIntoScrolling/0.75f) * (1 + 1.0f/NUM_GRADE_FRAMES);
			else if( fPercentIntoScrolling < 0.9 )
				fPercentIntoScrolling = 1 + 1.0f/NUM_GRADE_FRAMES;
			else
				fPercentIntoScrolling = (1 + 1.0f/NUM_GRADE_FRAMES) - ((fPercentIntoScrolling-0.9f)/0.1f) * 1.0f/NUM_GRADE_FRAMES;
		} else {
			fPercentIntoScrolling = 1 - (m_fTimeLeftInScroll/QUICK_SCROLL_TIME);
		}
		
		m_frectCurTexCoords.left   = m_frectStartTexCoords.left*(1-fPercentIntoScrolling)   + m_frectDestTexCoords.left*fPercentIntoScrolling;
		m_frectCurTexCoords.top    = m_frectStartTexCoords.top*(1-fPercentIntoScrolling)    + m_frectDestTexCoords.top*fPercentIntoScrolling;
		m_frectCurTexCoords.right  = m_frectStartTexCoords.right*(1-fPercentIntoScrolling)  + m_frectDestTexCoords.right*fPercentIntoScrolling;
		m_frectCurTexCoords.bottom = m_frectStartTexCoords.bottom*(1-fPercentIntoScrolling) + m_frectDestTexCoords.bottom*fPercentIntoScrolling;

		this->SetCustomTextureRect( m_frectCurTexCoords );
	}
}

void GradeDisplay::DrawPrimitives()
{
	Sprite::DrawPrimitives();
}

int GradeDisplay::GetFrameNo( PlayerNumber pn, Grade g )
{
	// either 8, or 16 states
	int iNumCols = Sprite::GetNumStates()==8 ? 1 : 2;
	switch( g )
	{
	case GRADE_AAAA:	return 0*iNumCols+pn;	break;
	case GRADE_AAA:		return 1*iNumCols+pn;	break;
	case GRADE_AA:		return 2*iNumCols+pn;	break;
	case GRADE_A:		return 3*iNumCols+pn;	break;
	case GRADE_B:		return 4*iNumCols+pn;	break;
	case GRADE_C:		return 5*iNumCols+pn;	break;
	case GRADE_D:		return 6*iNumCols+pn;	break;
	case GRADE_E:		return 7*iNumCols+pn;	break;
	case GRADE_NO_DATA:	return 0;				break;
	default:			ASSERT(0);	return 0;
	}
}

void GradeDisplay::SetGrade( PlayerNumber pn, Grade g )
{
	m_PlayerNumber = pn;
	m_Grade = g;

	m_bDoScrolling = false;
	StopUsingCustomCoords();

	SetState( GetFrameNo(pn,g) );
	Sprite::SetDiffuse( RageColor(1,1,1,g==GRADE_NO_DATA ? 0 : 1) );
}

void GradeDisplay::Spin()
{
	m_bDoScrolling = true;

	int iFrameNo = GetFrameNo( m_PlayerNumber, m_Grade );

	m_frectDestTexCoords = *m_pTexture->GetTextureCoordRect( iFrameNo );
	m_frectStartTexCoords = m_frectDestTexCoords;
	m_frectStartTexCoords.top += GRADES_TO_SCROLL * GRADE_FRAME_HEIGHT;
	m_frectStartTexCoords.bottom += GRADES_TO_SCROLL * GRADE_FRAME_HEIGHT;

	m_fTimeLeftInScroll = SCROLL_TIME;

	/* Set the initial position. */
	Update(0);
}

void GradeDisplay::SettleImmediately()
{
	m_fTimeLeftInScroll = 0;
}

void GradeDisplay::SettleQuickly()
{
	if(m_bDoScrolling != 1)
		return;

	/* If we're in the last phase of scrolling, don't do this. */
	if( 1 - (m_fTimeLeftInScroll/SCROLL_TIME) >= 0.9 )
		return;

	/* m_frectDestTexCoords.top is between 0 and 1 (inclusive).  m_frectCurTexCoords
	 * is somewhere above that.  Shift m_frectCurTexCoords downwards so it's pointing
	 * at the same physical place (remember, the grade texture is tiled) but no more
	 * than one rotation away from the destination. */
	while(m_frectCurTexCoords.top > m_frectDestTexCoords.top + 1.0f)
	{
		m_frectCurTexCoords.top -= 1.0f;
		m_frectCurTexCoords.bottom -= 1.0f;
	}

	m_frectStartTexCoords = m_frectCurTexCoords;
	m_bDoScrolling = 2;
	m_fTimeLeftInScroll = QUICK_SCROLL_TIME;
}


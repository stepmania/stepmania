#include "stdafx.h"
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


const float SCROLL_TIME = 5.0f;
const int NUM_GRADE_FRAMES = 7;
const float GRADE_FRAME_HEIGHT = 1/(float)NUM_GRADE_FRAMES;
const float GRADES_TO_SCROLL = NUM_GRADE_FRAMES*4;


GradeDisplay::GradeDisplay()
{
	Load( THEME->GetPathTo("Graphics","evaluation grades 1x7") );
	StopAnimating();

	m_fTimeLeftInScroll = 0;
	m_bDoScrolling = false;

	SetGrade( PLAYER_1, GRADE_NO_DATA );
}

void GradeDisplay::Update( float fDeltaTime )
{
	Sprite::Update( fDeltaTime );

	if( m_bDoScrolling )
	{
		m_fTimeLeftInScroll -= fDeltaTime;
		m_fTimeLeftInScroll = max( 0, m_fTimeLeftInScroll );

		float fPercentIntoScrolling = 1 - (m_fTimeLeftInScroll/SCROLL_TIME);
		if( fPercentIntoScrolling < 0.75 )
			fPercentIntoScrolling = (fPercentIntoScrolling/0.75f) * (1 + 1.0f/NUM_GRADE_FRAMES);
		else if( fPercentIntoScrolling < 0.9 )
			fPercentIntoScrolling = 1 + 1.0f/NUM_GRADE_FRAMES;
		else
			fPercentIntoScrolling = (1 + 1.0f/NUM_GRADE_FRAMES) - ((fPercentIntoScrolling-0.9f)/0.1f) * 1.0f/NUM_GRADE_FRAMES;

		FRECT frectCurrentTextureCoords;
		frectCurrentTextureCoords.left   = m_frectStartTexCoords.left*(1-fPercentIntoScrolling)   + m_frectDestTexCoords.left*fPercentIntoScrolling;
		frectCurrentTextureCoords.top    = m_frectStartTexCoords.top*(1-fPercentIntoScrolling)    + m_frectDestTexCoords.top*fPercentIntoScrolling;
		frectCurrentTextureCoords.right  = m_frectStartTexCoords.right*(1-fPercentIntoScrolling)  + m_frectDestTexCoords.right*fPercentIntoScrolling;
		frectCurrentTextureCoords.bottom = m_frectStartTexCoords.bottom*(1-fPercentIntoScrolling) + m_frectDestTexCoords.bottom*fPercentIntoScrolling;

		this->SetCustomTextureRect( frectCurrentTextureCoords );
	}
}

void GradeDisplay::DrawPrimitives()
{
	Sprite::DrawPrimitives();
}

void GradeDisplay::SetGrade( PlayerNumber pn, Grade g )
{
	m_Grade = g;

	m_bDoScrolling = false;
	StopUsingCustomCoords();

	SetDiffuse( D3DXCOLOR(1,1,1,1) );

	// Ugly...  This has to handle cases where the sprite has 7, 8, 14, or 16 states
	int iNumCols = (this->GetNumStates()>8) ? 2 : 1;
	switch( g )
	{
	case GRADE_AAA:		SetState( 0*iNumCols+pn );	break;
	case GRADE_AA:		SetState( 1*iNumCols+pn );	break;
	case GRADE_A:		SetState( 2*iNumCols+pn );	break;
	case GRADE_B:		SetState( 3*iNumCols+pn );	break;
	case GRADE_C:		SetState( 4*iNumCols+pn );	break;
	case GRADE_D:		SetState( 5*iNumCols+pn );	break;
	case GRADE_E:		SetState( 6*iNumCols+pn );	break;
	case GRADE_NO_DATA:	SetDiffuse( D3DXCOLOR(1,1,1,0) );	break;
	default:			ASSERT(0);
	}
};

void GradeDisplay::SpinAndSettleOn( Grade g )
{
	ASSERT( g != GRADE_NO_DATA );
	m_Grade = g;

	m_bDoScrolling = true;


	SetDiffuse( D3DXCOLOR(1,1,1,1) );

	int iFrameNo=0;
	switch( g )
	{
	case GRADE_AAA:		iFrameNo = 0;	break;
	case GRADE_AA:		iFrameNo = 1;	break;
	case GRADE_A:		iFrameNo = 2;	break;
	case GRADE_B:		iFrameNo = 3;	break;
	case GRADE_C:		iFrameNo = 4;	break;
	case GRADE_D:		iFrameNo = 5;	break;
	case GRADE_E:		iFrameNo = 6;	break;
	default:	ASSERT(0);
	}

	m_frectDestTexCoords = *m_pTexture->GetTextureCoordRect( iFrameNo );
	m_frectStartTexCoords = m_frectDestTexCoords;
	m_frectStartTexCoords.top += GRADES_TO_SCROLL * GRADE_FRAME_HEIGHT;
	m_frectStartTexCoords.bottom += GRADES_TO_SCROLL * GRADE_FRAME_HEIGHT;

	m_fTimeLeftInScroll = SCROLL_TIME;
}

void GradeDisplay::SettleImmediately()
{
	m_fTimeLeftInScroll = 0;
}

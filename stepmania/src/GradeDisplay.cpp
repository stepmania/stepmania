#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: Grade.cpp

 Desc: A graphic displayed in the Grade during Dancing.

 Copyright (c) 2001-2002 by the persons listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/



#include "GradeDisplay.h"
#include "RageUtil.h"
#include "GameConstantsAndTypes.h"
#include "ThemeManager.h"


const float SCROLL_TIME = 5.0f;


GradeDisplay::GradeDisplay()
{
	Load( THEME->GetPathTo(GRAPHIC_RESULTS_GRADES) );
	StopAnimating();

	m_fTimeLeftInScroll = 0;

//	SetGrade( GRADE_NO_DATA );
}

void GradeDisplay::Update( float fDeltaTime )
{
	Sprite::Update( fDeltaTime );

	m_fTimeLeftInScroll -= fDeltaTime;
	m_fTimeLeftInScroll = max( 0, m_fTimeLeftInScroll );

	const float fPercentIntoScrolling = 1 - (m_fTimeLeftInScroll/SCROLL_TIME);
	FRECT frectCurrentTextureCoords;
	frectCurrentTextureCoords.left   = m_frectStartTexCoords.left*(1-fPercentIntoScrolling)   + m_frectDestTexCoords.left*fPercentIntoScrolling;
	frectCurrentTextureCoords.top    = m_frectStartTexCoords.top*(1-fPercentIntoScrolling)    + m_frectDestTexCoords.top*fPercentIntoScrolling;
	frectCurrentTextureCoords.right  = m_frectStartTexCoords.right*(1-fPercentIntoScrolling)  + m_frectDestTexCoords.right*fPercentIntoScrolling;
	frectCurrentTextureCoords.bottom = m_frectStartTexCoords.bottom*(1-fPercentIntoScrolling) + m_frectDestTexCoords.bottom*fPercentIntoScrolling;

	this->SetCustomTextureRect( frectCurrentTextureCoords );
}

void GradeDisplay::SetGrade( Grade g )
{
//	StopUsingCustomCoords();

//	SetDiffuseColor( D3DXCOLOR(1,1,1,1) );

	switch( g )
	{
	case GRADE_AAA:		SetState(0);	break;
	case GRADE_AA:		SetState(1);	break;
	case GRADE_A:		SetState(2);	break;
	case GRADE_B:		SetState(3);	break;
	case GRADE_C:		SetState(4);	break;
	case GRADE_D:		SetState(5);	break;
	case GRADE_E:		SetState(6);	break;
//	case GRADE_NO_DATA:	SetDiffuseColor( D3DXCOLOR(1,1,1,0) );	break;
	default:			ASSERT( false );
	}
};

void GradeDisplay::SpinAndSettleOn( Grade g )
{
	SetDiffuseColor( D3DXCOLOR(1,1,1,1) );

	m_frectStartTexCoords = *m_pTexture->GetTextureCoordRect( 0 );
	m_frectDestTexCoords = *m_pTexture->GetTextureCoordRect( 6 );
	m_fTimeLeftInScroll = SCROLL_TIME;
}

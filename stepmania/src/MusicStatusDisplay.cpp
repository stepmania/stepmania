#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: MusicStatusDisplay

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "MusicStatusDisplay.h"
#include "RageUtil.h"
#include "GameConstantsAndTypes.h"
#include "MusicWheel.h"
#include "MusicStatusDisplay.h"
#include "RageTimer.h"



MusicStatusDisplay::MusicStatusDisplay()
{
	Load( THEME->GetPathTo("Graphics","music status icons") );
	StopAnimating();

	SetType( TYPE_NONE );
};

void MusicStatusDisplay::SetType( MusicStatusDisplayType msdt )
{
	m_MusicStatusDisplayType = msdt;

	switch( m_MusicStatusDisplayType )
	{
	case TYPE_NEW:
		m_MusicStatusDisplayType = TYPE_NEW;	
		SetState( 0 );	
		SetDiffuseColor( D3DXCOLOR(1,1,1,1) );
		break;
	case TYPE_CROWN1:
		m_MusicStatusDisplayType = TYPE_CROWN1;	
		SetState( 1 );	
		SetDiffuseColor( D3DXCOLOR(1,1,1,1) );	
		break;
	case TYPE_CROWN2:
		m_MusicStatusDisplayType = TYPE_CROWN2;	
		SetState( 2 );	
		SetDiffuseColor( D3DXCOLOR(1,1,1,1) );	
		break;
	case TYPE_CROWN3:
		m_MusicStatusDisplayType = TYPE_CROWN3;	
		SetState( 3 );	
		SetDiffuseColor( D3DXCOLOR(1,1,1,1) );	
		break;
	case TYPE_NONE:
	default:
		m_MusicStatusDisplayType = TYPE_NONE;	
		SetDiffuseColor( D3DXCOLOR(0,0,0,0) );
		break;
	}
};

void MusicStatusDisplay::Update( float fDeltaTime )
{
	Sprite::Update( fDeltaTime );

};

void MusicStatusDisplay::DrawPrimitives()
{
	switch( m_MusicStatusDisplayType )
	{
	case TYPE_CROWN1:
	case TYPE_CROWN2:
	case TYPE_CROWN3:
		// blink
		if( (TIMER->GetTimeSinceStart() - (int)TIMER->GetTimeSinceStart()) > 0.5f )		// show the new icon
			return;
		break;
	case TYPE_NONE:
		return;
		break;
	}

	Sprite::DrawPrimitives();
}
#include "stdafx.h"
//-----------------------------------------------------------------------------
// File: Combo.cpp
//
// Desc: Combo counter that displays while dancing.
//
// Copyright (c) 2001 Chris Danford.  All rights reserved.
//-----------------------------------------------------------------------------



#include "RageUtil.h"

#include "Combo.h"


#define CENTER_X		320
#define CENTER_Y		240


#define FONT		"Fonts\\Font - Arial Bold numbers 30px.font"

#define COMBO_TWEEN_TIME	0.5f
#define COMBO_SPRITE		"Sprites\\Combo.sprite"
#define COMBO_Y				(CENTER_Y+60)


Combo::Combo()
{
	m_bVisible = FALSE;

	m_sprCombo.LoadFromSpriteFile( COMBO_SPRITE );
	m_textNum.LoadFromFontFile( FONT );
	m_textNum.SetText( "" );

	SetX( CENTER_X );	
}

Combo::~Combo()
{

}


void Combo::SetX( int iNewX )
{
	m_sprCombo.SetXY( iNewX+40, COMBO_Y );
	m_textNum.SetXY(  iNewX-50, COMBO_Y );
}

void Combo::Update( const FLOAT &fDeltaTime )
{
	m_sprCombo.Update( fDeltaTime );
	m_textNum.Update( fDeltaTime );
}

void Combo::Draw()
{
	if( m_bVisible )
	{
		m_textNum.Draw();
		m_sprCombo.Draw();
	}
}


void Combo::SetCombo( int iNum )
{
	if( iNum <= 4 )
	{
		m_bVisible = FALSE;
	}
	else
	{
		m_bVisible = TRUE;

		m_textNum.SetText( ssprintf("%d", iNum) );
		m_textNum.SetZoom( 1.0f + iNum/200.0f ); 
		m_textNum.TweenTo( COMBO_TWEEN_TIME, m_textNum.GetX(), m_textNum.GetY() );
	}

}


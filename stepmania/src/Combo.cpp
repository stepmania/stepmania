#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: Combo.h

 Desc: A graphic displayed in the Combo during Dancing.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/

#include "Combo.h"
#include "ThemeManager.h"


Combo::Combo()
{
	m_iCurCombo = 0;
	m_iMaxCombo = 0;

	m_sprCombo.Load( THEME->GetPathTo(GRAPHIC_GAMEPLAY_COMBO) );
	m_sprCombo.TurnShadowOn();
	m_sprCombo.StopAnimating();
	m_sprCombo.SetX( 40 );
	m_sprCombo.SetZoom( 1.0f );

	m_textComboNumber.Load( THEME->GetPathTo(FONT_COMBO_NUMBERS) );
	m_textComboNumber.TurnShadowOn();
	m_textComboNumber.SetHorizAlign( Actor::align_right );
	m_textComboNumber.SetX( -10 );

	m_textComboNumber.SetDiffuseColor( D3DXCOLOR(1,1,1,0) );	// invisible
	m_sprCombo.SetDiffuseColor( D3DXCOLOR(1,1,1,0) );	// invisible


	this->AddActor( &m_textComboNumber );
	this->AddActor( &m_sprCombo );
}


void Combo::ContinueCombo()
{
	m_iCurCombo++;

	// new max combo
	if( m_iCurCombo > m_iMaxCombo )
		m_iMaxCombo = m_iCurCombo;

	if( m_iCurCombo <= 4 )
	{
		m_textComboNumber.SetDiffuseColor( D3DXCOLOR(1,1,1,0) );	// invisible
		m_sprCombo.SetDiffuseColor( D3DXCOLOR(1,1,1,0) );	// invisible
	}
	else
	{
		m_textComboNumber.SetDiffuseColor( D3DXCOLOR(1,1,1,1) );	// visible
		m_sprCombo.SetDiffuseColor( D3DXCOLOR(1,1,1,1) );	// visible

		m_textComboNumber.SetText( ssprintf("%d", m_iCurCombo) );
		float fNewZoom = 0.5f + m_iCurCombo/800.0f;
		m_textComboNumber.SetZoom( fNewZoom ); 
		m_textComboNumber.SetX( -40 - (fNewZoom-1)*30 ); 
		
		//this->SetZoom( 1.2f );
		//this->BeginTweening( 0.3f );
		//this->SetTweenZoom( 1 );
	}
}

void Combo::EndCombo()
{
	m_iCurCombo = 0;

	m_textComboNumber.SetDiffuseColor( D3DXCOLOR(1,1,1,0) );	// invisible
	m_sprCombo.SetDiffuseColor( D3DXCOLOR(1,1,1,0) );	// invisible

}

int Combo::GetCurrentCombo()
{
	return m_iCurCombo;
}

int Combo::GetMaxCombo()
{
	return m_iMaxCombo;
}
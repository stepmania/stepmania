#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: Combo

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Combo.h"
#include "PrefsManager.h"
#include "ScreenManager.h"
#include "ScreenGameplay.h"


Combo::Combo()
{
	m_iCurCombo = 0;
	m_iMaxCombo = 0;

	m_sprCombo.Load( THEME->GetPathTo("Graphics", "gameplay combo") );
	m_sprCombo.TurnShadowOn();
	m_sprCombo.StopAnimating();
	m_sprCombo.SetX( 40 );
	m_sprCombo.SetZoom( 1.0f );

	m_textComboNumber.LoadFromFont( THEME->GetPathTo("Fonts","combo numbers") );
	m_textComboNumber.TurnShadowOn();
	m_textComboNumber.SetHorizAlign( Actor::align_right );
	m_textComboNumber.SetX( -0 );

	m_textComboNumber.SetDiffuseColor( D3DXCOLOR(1,1,1,0) );	// invisible
	m_sprCombo.SetDiffuseColor( D3DXCOLOR(1,1,1,0) );	// invisible


	this->AddSubActor( &m_textComboNumber );
	this->AddSubActor( &m_sprCombo );
}


void Combo::ContinueCombo()
{
	m_iCurCombo++;

	/* If needed, make this a theme metric. */
	const bool HighComboAlert_OnceOnly = true;

	if(!HighComboAlert_OnceOnly || m_iCurCombo > m_iMaxCombo)
	switch( m_iCurCombo )
	{
	case 100: 		SCREENMAN->SendMessageToTopScreen( SM_100Combo, 0 );	break;
	case 200: 		SCREENMAN->SendMessageToTopScreen( SM_200Combo, 0 );	break;
	case 300: 		SCREENMAN->SendMessageToTopScreen( SM_300Combo, 0 );	break;
	case 400: 		SCREENMAN->SendMessageToTopScreen( SM_400Combo, 0 );	break;
	case 500: 		SCREENMAN->SendMessageToTopScreen( SM_500Combo, 0 );	break;
	case 600: 		SCREENMAN->SendMessageToTopScreen( SM_600Combo, 0 );	break;
	case 700: 		SCREENMAN->SendMessageToTopScreen( SM_700Combo, 0 );	break;
	case 800: 		SCREENMAN->SendMessageToTopScreen( SM_800Combo, 0 );	break;
	case 900: 		SCREENMAN->SendMessageToTopScreen( SM_900Combo, 0 );	break;
	case 1000: 		SCREENMAN->SendMessageToTopScreen( SM_1000Combo, 0 );	break;
	}

	// new max combo
	m_iMaxCombo = max(m_iMaxCombo, m_iCurCombo);

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
		float fNewZoom = min( 0.5f + m_iCurCombo/800.0f, 1.1f );
		m_textComboNumber.SetZoom( fNewZoom ); 
		
		//this->SetZoom( 1.2f );
		//this->BeginTweening( 0.3f );
		//this->SetTweenZoom( 1 );
	}
}

void Combo::EndCombo()
{
	if( m_iCurCombo > 50 )
		SCREENMAN->SendMessageToTopScreen( SM_ComboStopped, 0 );

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
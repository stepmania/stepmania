#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenGameOver

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenGameOver.h"
#include "ThemeManager.h"
#include "RageLog.h"
#include "TransitionFadeWipe.h"
#include "Sprite.h"
#include "AnnouncerManager.h"
#include "ScreenManager.h"
#include "ScreenTitleMenu.h"

const ScreenMessage SM_StartFadingOut	=	ScreenMessage(SM_User + 1);
const ScreenMessage SM_GoToNextState	=	ScreenMessage(SM_User + 2);


ScreenGameOver::ScreenGameOver()
{
	m_psprGameOver = new Sprite;
	m_psprGameOver->Load( THEME->GetPathTo(GRAPHIC_GAME_OVER) );
	m_psprGameOver->SetXY( CENTER_X, CENTER_Y );
	this->AddActor( m_psprGameOver );

	m_pWipe = new TransitionFadeWipe;
	m_pWipe->OpenWipingRight();

	this->SendScreenMessage( SM_StartFadingOut, 5 );
}


ScreenGameOver::~ScreenGameOver()
{
	delete m_psprGameOver;
	delete m_pWipe;
}	

void ScreenGameOver::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_StartFadingOut:
		m_psprGameOver->BeginTweening( 0.8f );
		m_psprGameOver->SetTweenDiffuseColor( D3DXCOLOR(1,1,1,0) );
		this->SendScreenMessage( SM_GoToNextState, 0.8f );
		break;
	case SM_GoToNextState:
		SCREENMAN->SetNewScreen( new ScreenTitleMenu );
		break;
	}
}


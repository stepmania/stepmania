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
#include "AnnouncerManager.h"

const ScreenMessage SM_StartFadingOut	=	ScreenMessage(SM_User + 1);
const ScreenMessage SM_GoToNextState	=	ScreenMessage(SM_User + 2);
const ScreenMessage SM_PlayAnnouncer	=	ScreenMessage(SM_User + 3);


ScreenGameOver::ScreenGameOver()
{
	m_sprGameOver.Load( THEME->GetPathTo(GRAPHIC_GAME_OVER) );
	m_sprGameOver.SetXY( CENTER_X, CENTER_Y );
	this->AddActor( &m_sprGameOver );

	// tween game over
	m_sprGameOver.SetAddColor( D3DXCOLOR(1,1,1,0) );
	m_sprGameOver.SetDiffuseColor( D3DXCOLOR(1,1,1,0) );

	m_sprGameOver.BeginTweeningQueued( 0.5f );		// fade to white
	m_sprGameOver.SetTweenAddColor( D3DXCOLOR(1,1,1,1) );

	m_sprGameOver.BeginTweeningQueued( 0.01f );		// turn color on
	m_sprGameOver.SetTweenDiffuseColor( D3DXCOLOR(1,1,1,1) );

	m_sprGameOver.BeginTweeningQueued( 0.5f );		// fade to color
	m_sprGameOver.SetTweenAddColor( D3DXCOLOR(1,1,1,0) );

	// BUGFIX by ANDY: Stage will now reset back to 0 when game ends.
	PREFSMAN->m_iCurrentStageIndex = 0;

	this->SendScreenMessage( SM_PlayAnnouncer, 0.5 );
	this->SendScreenMessage( SM_StartFadingOut, 5 );
}


void ScreenGameOver::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_PlayAnnouncer:
		SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_GAME_OVER) );
		break;
	case SM_StartFadingOut:
		m_sprGameOver.BeginTweening( 0.8f );
		m_sprGameOver.SetTweenDiffuseColor( D3DXCOLOR(1,1,1,0) );
		this->SendScreenMessage( SM_GoToNextState, 0.8f );
		break;
	case SM_GoToNextState:
		SCREENMAN->SetNewScreen( new ScreenTitleMenu );
		break;
	}
}

void ScreenGameOver::MenuStart( PlayerNumber p )
{
	this->ClearMessageQueue();
	this->SendScreenMessage( SM_StartFadingOut, 0 );
}

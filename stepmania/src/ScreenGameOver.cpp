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
#include "PrefsManager.h"
#include "RageLog.h"
#include "TransitionFadeWipe.h"
#include "Sprite.h"
#include "AnnouncerManager.h"
#include "ScreenManager.h"
#include "AnnouncerManager.h"
#include "GameState.h"
#include "RageMusic.h"


const ScreenMessage SM_StartFadingOut	=	ScreenMessage(SM_User + 1);
const ScreenMessage SM_GoToNextScreen	=	ScreenMessage(SM_User + 2);
const ScreenMessage SM_PlayAnnouncer	=	ScreenMessage(SM_User + 3);


ScreenGameOver::ScreenGameOver()
{
	m_bClosing = false;

	m_sprGameOver.Load( THEME->GetPathTo("Graphics","game over") );
	m_sprGameOver.SetXY( CENTER_X, CENTER_Y );
	this->AddSubActor( &m_sprGameOver );

	// tween game over
	m_sprGameOver.SetGlowColor( D3DXCOLOR(1,1,1,0) );
	m_sprGameOver.SetDiffuseColor( D3DXCOLOR(1,1,1,0) );

	m_sprGameOver.BeginTweeningQueued( 0.5f );		// fade to white
	m_sprGameOver.SetTweenAddColor( D3DXCOLOR(1,1,1,1) );

	m_sprGameOver.BeginTweeningQueued( 0.01f );		// turn color on
	m_sprGameOver.SetTweenDiffuseColor( D3DXCOLOR(1,1,1,1) );

	m_sprGameOver.BeginTweeningQueued( 0.5f );		// fade to color
	m_sprGameOver.SetTweenAddColor( D3DXCOLOR(1,1,1,0) );

	MUSIC->LoadAndPlayIfNotAlready( THEME->GetPathTo("Sounds","game over music") );

	this->SendScreenMessage( SM_PlayAnnouncer, 0.5 );
	this->SendScreenMessage( SM_StartFadingOut, 5 );
}


void ScreenGameOver::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_PlayAnnouncer:
		SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo("game over") );
		break;
	case SM_StartFadingOut:
		m_bClosing = true;
		m_sprGameOver.BeginTweening( 0.8f );
		m_sprGameOver.SetTweenDiffuseColor( D3DXCOLOR(1,1,1,0) );
		this->SendScreenMessage( SM_GoToNextScreen, 0.8f );
		break;
	case SM_GoToNextScreen:
		SCREENMAN->SetNewScreen( "ScreenTitleMenu" );
		break;
	}
}

void ScreenGameOver::MenuStart( PlayerNumber p )
{
	if( m_bClosing )
		return;

	this->ClearMessageQueue();
	this->SendScreenMessage( SM_StartFadingOut, 0 );
}

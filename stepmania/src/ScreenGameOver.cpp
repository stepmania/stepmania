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
	m_Background.LoadFromAniDir( THEME->GetPathTo("BGAnimations","game over") );
	this->AddChild( &m_Background );

	m_Fade.OpenWipingRight( SM_None );
	this->AddChild( &m_Fade );

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
		m_Fade.CloseWipingRight( SM_GoToNextScreen );
		break;
	case SM_GoToNextScreen:
		SCREENMAN->SetNewScreen( "ScreenTitleMenu" );
		break;
	}
}

void ScreenGameOver::MenuStart( PlayerNumber pn )
{
	if( m_Fade.IsClosing() )
		return;

	this->ClearMessageQueue();
	this->SendScreenMessage( SM_StartFadingOut, 0 );
}

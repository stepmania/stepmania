#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenCaution

 Desc: Screen that displays while resources are being loaded.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


#include "ScreenCaution.h"
#include "ScreenTitleMenu.h"
#include "GameConstantsAndTypes.h"
#include "ScreenSelectStyle.h"
#include "ScreenEZ2SelectStyle.h"
#include "ScreenSelectStyle5th.h"
#include "RageTextureManager.h"
#include "PrefsManager.h"
#include "AnnouncerManager.h"
#include "GameState.h"


#define SELECT_STYLE_TYPE		THEME->GetMetricI("General","SelectStyleType")
enum SelectStyleType // for use with the metric above
{
	SELECT_STYLE_TYPE_MAX = 0,
	SELECT_STYLE_TYPE_5TH,
	SELECT_STYLE_TYPE_EZ2,
};


const ScreenMessage SM_GoToPrevState	= ScreenMessage(SM_User-6);
const ScreenMessage SM_DoneOpening		= ScreenMessage(SM_User-7);
const ScreenMessage SM_StartClosing		= ScreenMessage(SM_User-8);
const ScreenMessage SM_GoToSelectMusic	= ScreenMessage(SM_User-9);


ScreenCaution::ScreenCaution()
{
	GAMESTATE->m_bPlayersCanJoin = true;

	m_sprCaution.Load( THEME->GetPathTo("Graphics","caution") );
	m_sprCaution.StretchTo( CRect(SCREEN_LEFT,SCREEN_TOP,SCREEN_RIGHT,SCREEN_BOTTOM) );
	this->AddSubActor( &m_sprCaution );
	
	m_Wipe.OpenWipingRight( SM_DoneOpening );
	this->AddSubActor( &m_Wipe );

	m_FadeWipe.SetOpened();
	this->AddSubActor( &m_FadeWipe );

	this->SendScreenMessage( SM_StartClosing, 1 );
}


void ScreenCaution::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	if( m_Wipe.IsClosing() )
		return;

	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );
}


void ScreenCaution::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_StartClosing:
		if( !m_Wipe.IsClosing() )
			m_Wipe.CloseWipingRight( SM_GoToSelectMusic );
		break;
	case SM_DoneOpening:
		SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_CAUTION) );
		break;
	case SM_GoToPrevState:
		SCREENMAN->SetNewScreen( new ScreenTitleMenu );
		break;
	case SM_GoToSelectMusic:
		switch( SELECT_STYLE_TYPE )
		{
		case SELECT_STYLE_TYPE_MAX:
			SCREENMAN->SetNewScreen( new ScreenSelectStyle );
			break;
		case SELECT_STYLE_TYPE_5TH:
			SCREENMAN->SetNewScreen( new ScreenSelectStyle5th );
			break;
		case SELECT_STYLE_TYPE_EZ2:
			SCREENMAN->SetNewScreen( new ScreenEz2SelectStyle );
			break;
		default:
			ASSERT(0);
			break;
		}
		break;
	}
}

void ScreenCaution::MenuStart( const PlayerNumber p )
{
	if( p != PLAYER_INVALID  &&  !GAMESTATE->m_bSideIsJoined[p] )
	{
		GAMESTATE->m_bSideIsJoined[p] = true;
		SOUND->PlayOnceStreamed( THEME->GetPathTo("Sounds","menu start") );
		SCREENMAN->RefreshCreditsMessages();
		return;	// don't fall though
	}

	if( !m_Wipe.IsOpening()  &&  !m_Wipe.IsClosing() )
		m_Wipe.CloseWipingRight( SM_GoToSelectMusic );
}

void ScreenCaution::MenuBack( const PlayerNumber p )
{
	if(m_FadeWipe.IsClosing())
		return;
	this->ClearMessageQueue();
	m_FadeWipe.CloseWipingLeft( SM_GoToPrevState );
	SOUND->PlayOnceStreamed( THEME->GetPathTo("Sounds","menu back") );
}


#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: ScreenCaution.h

Desc: Screen that displays while resources are being loaded.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/


#include "ScreenCaution.h"
#include "GameConstantsAndTypes.h"
#include "ScreenSelectStyle.h"
#include "RageTextureManager.h"
#include "ThemeManager.h"
#include "AnnouncerManager.h"


const ScreenMessage SM_DoneOpening		= ScreenMessage(SM_User-7);
const ScreenMessage SM_StartClosing		= ScreenMessage(SM_User-8);
const ScreenMessage SM_GoToSelectMusic	= ScreenMessage(SM_User-9);


ScreenCaution::ScreenCaution()
{
	m_sprCaution.Load( THEME->GetPathTo(GRAPHIC_CAUTION) );
	m_sprCaution.StretchTo( CRect(0,0,640,480) );
	this->AddActor( &m_sprCaution );
	
	m_Wipe.OpenWipingRight( SM_DoneOpening );
	this->AddActor( &m_Wipe );

	this->SendScreenMessage( SM_StartClosing, 3 );
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
		m_Wipe.CloseWipingRight( SM_GoToSelectMusic );
		break;
	case SM_DoneOpening:
		if( PREFSMAN->m_bAnnouncer )
			SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_CAUTION) );
		break;
	case SM_GoToSelectMusic:
		SCREENMAN->SetNewScreen( new ScreenSelectStyle );
		break;
	}
}

void ScreenCaution::MenuStart( PlayerNumber p )
{
	m_Wipe.CloseWipingRight( SM_GoToSelectMusic );
}

#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenRemoveMemoryCard

 Desc: Where the player maps device input to pad input.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenRemoveMemoryCard.h"
#include "MemoryCardManager.h"
#include "RageLog.h"
#include "RageSounds.h"
#include "ThemeManager.h"
#include "GameConstantsAndTypes.h"
#include "PlayerNumber.h"
#include "ScreenManager.h"

#define NEXT_SCREEN			THEME->GetMetric (m_sName,"NextScreen")

ScreenRemoveMemoryCard::ScreenRemoveMemoryCard( CString sClassName ) : Screen( sClassName )
{
	if( !AnyCardsInserted() )
	{
		HandleScreenMessage( SM_GoToNextScreen );	// Don't show this screen
		return;
	}

	LOG->Trace( "ScreenRemoveMemoryCard::ScreenRemoveMemoryCard()" );
	
	m_Menu.Load( "ScreenRemoveMemoryCard" );
	this->AddChild( &m_Menu );

	SOUND->PlayMusic( THEME->GetPathToS("ScreenRemoveMemoryCard music") );
}

ScreenRemoveMemoryCard::~ScreenRemoveMemoryCard()
{
	LOG->Trace( "ScreenRemoveMemoryCard::~ScreenRemoveMemoryCard()" );
}

void ScreenRemoveMemoryCard::DrawPrimitives()
{
	m_Menu.DrawBottomLayer();
	Screen::DrawPrimitives();
	m_Menu.DrawTopLayer();
}

void ScreenRemoveMemoryCard::Update( float fDelta )
{
	Screen::Update( fDelta );
	
	if( !AnyCardsInserted() )	// all cards are pulled out
	{
		if( !m_Menu.IsTransitioning() )
			m_Menu.StartTransitioning( SM_GoToPrevScreen );		
	}
}

void ScreenRemoveMemoryCard::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	if( type != IET_FIRST_PRESS && type != IET_SLOW_REPEAT )
		return;	// ignore

	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );	// default handler
}

void ScreenRemoveMemoryCard::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_GoToNextScreen:
		SCREENMAN->SetNewScreen( NEXT_SCREEN );
		break;
	}
}

void ScreenRemoveMemoryCard::MenuStart( PlayerNumber pn )
{
	MenuBack( pn );
}

void ScreenRemoveMemoryCard::MenuBack( PlayerNumber pn )
{
	if(!m_Menu.IsTransitioning())
	{
		m_Menu.StartTransitioning( SM_GoToPrevScreen );		
	}
}
	
bool ScreenRemoveMemoryCard::AnyCardsInserted()
{
	for( int p=0; p<NUM_PLAYERS; p++ )
		if( MEMCARDMAN->GetCardState((PlayerNumber)p) != MEMORY_CARD_STATE_NO_CARD )
			return true;

	return false;
}

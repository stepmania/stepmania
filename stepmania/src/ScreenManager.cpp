#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenManager

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenManager.h"
#include "IniFile.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "GameState.h"


ScreenManager*	SCREENMAN = NULL;	// global and accessable from anywhere in our program


#define STATS_X				THEME->GetMetricF("Other","StatsX")
#define STATS_Y				THEME->GetMetricF("Other","StatsY")
#define CREDITS_P1_X		THEME->GetMetricF("Other","CreditsP1X")
#define CREDITS_P1_Y		THEME->GetMetricF("Other","CreditsP1Y")
#define CREDITS_P2_X		THEME->GetMetricF("Other","CreditsP2X")
#define CREDITS_P2_Y		THEME->GetMetricF("Other","CreditsP2Y")

float CREDITS_X( int p ) {
	switch( p ) {
		case PLAYER_1:	return CREDITS_P1_X;
		case PLAYER_2:	return CREDITS_P2_X;
		default:		ASSERT(0);	return 0;
	}
}
float CREDITS_Y( int p ) {
	switch( p ) {
		case PLAYER_1:	return CREDITS_P1_Y;
		case PLAYER_2:	return CREDITS_P2_Y;
		default:		ASSERT(0);	return 0;
	}
}



ScreenManager::ScreenManager()
{
	m_textStats.LoadFromFont( THEME->GetPathTo("Fonts","normal") );
	m_textStats.SetXY( STATS_X, STATS_Y );
	m_textStats.SetHorizAlign( Actor::align_right );
	m_textStats.SetVertAlign( Actor::align_top );
	m_textStats.SetZoom( 0.5f );
	m_textStats.SetShadowLength( 2 );

	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		m_textCreditInfo[p].LoadFromFont( THEME->GetPathTo("Fonts","normal") );
		m_textCreditInfo[p].SetXY( CREDITS_X(p), CREDITS_Y(p) );
		m_textCreditInfo[p].SetZoom( 0.5f );
		m_textCreditInfo[p].SetDiffuseColor( D3DXCOLOR(1,1,1,1) );
		m_textCreditInfo[p].SetShadowLength( 2 );
	}

	m_textSystemMessage.LoadFromFont( THEME->GetPathTo("Fonts","normal") );
	m_textSystemMessage.SetHorizAlign( Actor::align_left );
	m_textSystemMessage.SetVertAlign( Actor::align_bottom );
	m_textSystemMessage.SetXY( 5.0f, 10.0f );
	m_textSystemMessage.SetZoom( 0.5f );
	m_textSystemMessage.SetShadowLength( 2 );
	m_textSystemMessage.SetDiffuseColor( D3DXCOLOR(1,1,1,0) );
}


ScreenManager::~ScreenManager()
{
	LOG->Trace( "ScreenManager::~ScreenManager()" );

	// delete current states
	for( int i=0; i<m_ScreenStack.GetSize(); i++ )
		SAFE_DELETE( m_ScreenStack[i] );
}




void ScreenManager::Update( float fDeltaTime )
{
	m_textSystemMessage.Update( fDeltaTime );
	m_textStats.Update( fDeltaTime );
	for( int p=0; p<NUM_PLAYERS; p++ )
		m_textCreditInfo[p].Update( fDeltaTime );

	// delete all ScreensToDelete
	for( int i=0; i<m_ScreensToDelete.GetSize(); i++ ) {
		SAFE_DELETE( m_ScreensToDelete[i] );
		m_ScreensToDelete.RemoveAt(i);
	}

	// HACK!  If we deleted at least one state, then skip this update!
	if( i>0 ) return;

	// Update all windows in the stack
	for( i=0; i<m_ScreenStack.GetSize(); i++ )
		m_ScreenStack[i]->Update( fDeltaTime );

}


void ScreenManager::Restore()
{
	// Draw all CurrentScreens (back to front)
	for( int i=0; i<m_ScreenStack.GetSize(); i++ )
		m_ScreenStack[i]->Restore();
}

void ScreenManager::Invalidate()
{
	for( int i=0; i<m_ScreenStack.GetSize(); i++ )
		m_ScreenStack[i]->Invalidate();
}

void ScreenManager::Draw()
{
	// Draw all CurrentScreens (back to front)
	for( int i=0; i<m_ScreenStack.GetSize(); i++ )
		m_ScreenStack[i]->Draw();
	
	if( m_textSystemMessage.GetDiffuseColor().a != 0 )
		m_textSystemMessage.Draw();

	if( PREFSMAN  &&  PREFSMAN->m_bShowStats )
	{
		m_textStats.SetText( ssprintf("%d FPS\n%d TPF\n%d DPF", DISPLAY->GetFPS(),DISPLAY->GetTPF(),DISPLAY->GetDPF()) );
		m_textStats.Draw();
	}
	for( int p=0; p<NUM_PLAYERS; p++ )
		m_textCreditInfo[p].Draw();

}


void ScreenManager::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	LOG->Trace( "ScreenManager::Input( %d-%d, %d-%d, %d-%d, %d-%d )", 
		DeviceI.device, DeviceI.button, GameI.controller, GameI.button, MenuI.player, MenuI.button, StyleI.player, StyleI.col );

	// pass input only to topmost state
	if( m_ScreenStack.GetSize() > 0 )
		m_ScreenStack[m_ScreenStack.GetSize()-1]->Input( DeviceI, type, GameI, MenuI, StyleI );
}

void ScreenManager::SetNewScreen( Screen *pNewScreen )
{
	// move CurrentScreen to ScreenToDelete
	RefreshCreditsMessages();
	m_ScreensToDelete.Copy( m_ScreenStack );

	m_ScreenStack.RemoveAll();
	m_ScreenStack.Add( pNewScreen );
}

void ScreenManager::AddScreenToTop( Screen *pNewScreen )
{
	// our responsibility to tell the old state that it's losing focus
//	m_ScreenStack[m_ScreenStack.GetSize()-1]->HandleScreenMessage( SM_LosingInputFocus );

	// add the new state onto the back of the array
	m_ScreenStack.Add( pNewScreen );
}

void ScreenManager::PopTopScreen( ScreenMessage SM )
{
	Screen* pScreenToPop = m_ScreenStack[m_ScreenStack.GetSize()-1];	// top menu
	//pScreenToPop->HandleScreenMessage( SM_LosingInputFocus );
	m_ScreenStack.RemoveAt(m_ScreenStack.GetSize()-1);
	m_ScreensToDelete.Add( pScreenToPop );
	
	Screen* pNewTopScreen = m_ScreenStack[m_ScreenStack.GetSize()-1];
	pNewTopScreen->HandleScreenMessage( SM );
}

void ScreenManager::SendMessageToTopScreen( ScreenMessage SM, float fDelay )
{
	Screen* pTopScreen = m_ScreenStack[m_ScreenStack.GetSize()-1];
	pTopScreen->SendScreenMessage( SM, fDelay );
}


void ScreenManager::SystemMessage( CString sMessage )
{
	// Look for an open spot
	m_textSystemMessage.StopTweening();
	m_textSystemMessage.SetText( sMessage );
	m_textSystemMessage.SetDiffuseColor( D3DXCOLOR(1,1,1,1) );
	m_textSystemMessage.BeginTweeningQueued( 5 );
	m_textSystemMessage.BeginTweeningQueued( 1 );
	m_textSystemMessage.SetTweenDiffuseColor( D3DXCOLOR(1,1,1,0) );

	LOG->Trace( "WARNING:  Didn't find an empty system messages slot." );
}

/*
void ScreenManager::OverrideCreditsMessage( PlayerNumber p, CString sNewString )
{
	m_textCreditInfo[p].SetText( sNewString );
}
*/

void ScreenManager::RefreshCreditsMessages()
{
	// update joined
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		if(      GAMESTATE->m_bIsJoined[p] )	m_textCreditInfo[p].SetText( "" );
		else if( GAMESTATE->m_bPlayersCanJoin )	m_textCreditInfo[p].SetText( "PRESS START" );
		else									m_textCreditInfo[p].SetText( "CREDIT(s) 0 / 0" );
	}
}
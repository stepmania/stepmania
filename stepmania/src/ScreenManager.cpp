#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: ScreenManager.h

 Desc: Manages the game windows.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/

#include "ScreenManager.h"
#include "IniFile.h"
#include "GameConstantsAndTypes.h"
#include "ThemeManager.h"
#include "RageLog.h"


ScreenManager*	SCREENMAN = NULL;	// global and accessable from anywhere in our program



ScreenManager::ScreenManager()
{
	m_textFPS.Load( THEME->GetPathTo(FONT_NORMAL) );
	m_textFPS.SetXY( SCREEN_WIDTH-35, 15 );
	m_textFPS.SetZ( -2 );
	m_textFPS.SetZoom( 0.5f );
	m_textFPS.SetShadowLength( 2 );

	m_textSystemMessage.Load( THEME->GetPathTo(FONT_NORMAL) );
	m_textSystemMessage.SetHorizAlign( Actor::align_left );
	m_textSystemMessage.SetVertAlign( Actor::align_bottom );
	m_textSystemMessage.SetXY( 5.0f, 10.0f );
	m_textSystemMessage.SetZ( -2 );
	m_textSystemMessage.SetZoom( 0.5f );
	m_textSystemMessage.SetShadowLength( 2 );
	m_textSystemMessage.SetDiffuseColor( D3DXCOLOR(1,1,1,0) );
}


ScreenManager::~ScreenManager()
{
	LOG->WriteLine( "ScreenManager::~ScreenManager()" );

	// delete current states
	for( int i=0; i<m_ScreenStack.GetSize(); i++ )
		SAFE_DELETE( m_ScreenStack[i] );
}




void ScreenManager::Update( float fDeltaTime )
{
	m_textSystemMessage.Update( fDeltaTime );
	m_textFPS.Update( fDeltaTime );

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

	if( PREFSMAN  &&  PREFSMAN->m_bShowFPS )
	{
		m_textFPS.SetText( ssprintf("%3.0f FPS", DISPLAY->GetFPS()) );
		m_textFPS.Draw();
	}
}


void ScreenManager::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	LOG->WriteLine( "ScreenManager::Input( %d-%d, %d-%d, %d-%d, %d-%d )", 
		DeviceI.device, DeviceI.button, GameI.number, GameI.button, MenuI.player, MenuI.button, StyleI.player, StyleI.col );

	// pass input only to topmost state
	if( m_ScreenStack.GetSize() > 0 )
		m_ScreenStack[m_ScreenStack.GetSize()-1]->Input( DeviceI, type, GameI, MenuI, StyleI );
}

void ScreenManager::SetNewScreen( Screen *pNewScreen )
{
	// move CurrentScreen to ScreenToDelete
	m_ScreensToDelete.Copy( m_ScreenStack );

	m_ScreenStack.RemoveAll();
	m_ScreenStack.Add( pNewScreen );
}

void ScreenManager::AddScreenToTop( Screen *pNewScreen )
{
	// our responsibility to tell the old state that it's losing focus
	m_ScreenStack[m_ScreenStack.GetSize()-1]->HandleScreenMessage( SM_LosingInputFocus );

	// add the new state onto the back of the array
	m_ScreenStack.Add( pNewScreen );
}

void ScreenManager::PopTopScreen()
{
	Screen* pScreenToPop = m_ScreenStack[m_ScreenStack.GetSize()-1];	// top menu
	//pScreenToPop->HandleScreenMessage( SM_LosingInputFocus );
	m_ScreenStack.RemoveAt(m_ScreenStack.GetSize()-1);
	m_ScreensToDelete.Add( pScreenToPop );
	
	Screen* pNewTopScreen = m_ScreenStack[m_ScreenStack.GetSize()-1];
	pNewTopScreen->HandleScreenMessage( SM_RegainingInputFocus );
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

	LOG->WriteLine( "WARNING:  Didn't find an empty system messages slot." );
}



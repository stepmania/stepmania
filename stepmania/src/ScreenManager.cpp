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
#include "RageException.h"


ScreenManager*	SCREENMAN = NULL;	// global and accessable from anywhere in our program


#define STATS_X				THEME->GetMetricF("ScreenManager","StatsX")
#define STATS_Y				THEME->GetMetricF("ScreenManager","StatsY")
#define CREDITS_P1_X		THEME->GetMetricF("ScreenManager","CreditsP1X")
#define CREDITS_P1_Y		THEME->GetMetricF("ScreenManager","CreditsP1Y")
#define CREDITS_P2_X		THEME->GetMetricF("ScreenManager","CreditsP2X")
#define CREDITS_P2_Y		THEME->GetMetricF("ScreenManager","CreditsP2Y")

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
		m_textCreditInfo[p].SetDiffuse( D3DXCOLOR(1,1,1,1) );
		m_textCreditInfo[p].SetShadowLength( 2 );
	}

	m_textSystemMessage.LoadFromFont( THEME->GetPathTo("Fonts","normal") );
	m_textSystemMessage.SetHorizAlign( Actor::align_left );
	m_textSystemMessage.SetVertAlign( Actor::align_top );
	m_textSystemMessage.SetXY( 4.0f, 4.0f );
	m_textSystemMessage.SetZoom( 0.5f );
	m_textSystemMessage.SetShadowLength( 2 );
	m_textSystemMessage.SetDiffuse( D3DXCOLOR(1,1,1,0) );
}


ScreenManager::~ScreenManager()
{
	LOG->Trace( "ScreenManager::~ScreenManager()" );

	EmptyDeleteQueue();

	// delete current states
	for( int i=0; i<m_ScreenStack.GetSize(); i++ )
		SAFE_DELETE( m_ScreenStack[i] );
}

void ScreenManager::EmptyDeleteQueue()
{
	// delete all ScreensToDelete
	for( int i=0; i<m_ScreensToDelete.GetSize(); i++ )
		SAFE_DELETE( m_ScreensToDelete[i] );

	m_ScreensToDelete.RemoveAll();
}

void ScreenManager::Update( float fDeltaTime )
{
	m_textSystemMessage.Update( fDeltaTime );
	m_textStats.Update( fDeltaTime );
	for( int p=0; p<NUM_PLAYERS; p++ )
		m_textCreditInfo[p].Update( fDeltaTime );

	EmptyDeleteQueue();

	// Update all windows in the stack
	for( int i=0; i<m_ScreenStack.GetSize(); i++ )
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
	
	if( m_textSystemMessage.GetDiffuse().a != 0 )
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


// Screen classes
#include "ScreenAppearanceOptions.h"
#include "ScreenCaution.h"
#include "ScreenEdit.h"
#include "ScreenEditMenu.h"
#include "ScreenEvaluation.h"
#include "ScreenEz2SelectPlayer.h"
#include "ScreenEz2SelectStyle.h"
#include "ScreenGameOptions.h"
#include "ScreenGameOver.h"
#include "ScreenGameplay.h"
#include "ScreenGraphicOptions.h"
#include "ScreenHowToPlay.h"
#include "ScreenMapInstruments.h"
#include "ScreenMusicScroll.h"
#include "ScreenPlayerOptions.h"
#include "ScreenSandbox.h"
#include "ScreenSelectCourse.h"
#include "ScreenSelectDifficulty.h"
#include "ScreenSelectGame.h"
#include "ScreenSelectGroup.h"
#include "ScreenSelectMusic.h"
#include "ScreenSelectStyle5th.h"
#include "ScreenSelectStyle.h"
#include "ScreenSongOptions.h"
#include "ScreenStage.h"
#include "ScreenTitleMenu.h"

#include "ScreenPrompt.h"
#include "ScreenTextEntry.h"

void ScreenManager::SetNewScreen( CString sClassName )
{
	// It makes sense that ScreenManager should allocate memory for a new screen since it 
	// deletes it later on.  This also convention will reduce includes because screens won't 
	// have to include each other's headers of other screens.

	Screen* pNewScreen;

	if(		 0==stricmp(sClassName, "ScreenAppearanceOptions") )pNewScreen = new ScreenAppearanceOptions;
	else if( 0==stricmp(sClassName, "ScreenCaution") )			pNewScreen = new ScreenCaution;
	else if( 0==stricmp(sClassName, "ScreenEdit") )				pNewScreen = new ScreenEdit;
	else if( 0==stricmp(sClassName, "ScreenEditMenu") )			pNewScreen = new ScreenEditMenu;
	else if( 0==stricmp(sClassName, "ScreenEvaluation") )		pNewScreen = new ScreenEvaluation;
	else if( 0==stricmp(sClassName, "ScreenFinalEvaluation") )	pNewScreen = new ScreenFinalEvaluation;
	else if( 0==stricmp(sClassName, "ScreenEz2SelectPlayer") )	pNewScreen = new ScreenEz2SelectPlayer;
	else if( 0==stricmp(sClassName, "ScreenEz2SelectStyle") )	pNewScreen = new ScreenEz2SelectStyle;
	else if( 0==stricmp(sClassName, "ScreenGameOptions") )		pNewScreen = new ScreenGameOptions;
	else if( 0==stricmp(sClassName, "ScreenGameOver") )			pNewScreen = new ScreenGameOver;
	else if( 0==stricmp(sClassName, "ScreenGameplay") )			pNewScreen = new ScreenGameplay;
	else if( 0==stricmp(sClassName, "ScreenGraphicOptions") )	pNewScreen = new ScreenGraphicOptions;
	else if( 0==stricmp(sClassName, "ScreenHowToPlay") )		pNewScreen = new ScreenHowToPlay;
	else if( 0==stricmp(sClassName, "ScreenMapInstruments") )	pNewScreen = new ScreenMapInstruments;
	else if( 0==stricmp(sClassName, "ScreenMusicScroll") )		pNewScreen = new ScreenMusicScroll;
	else if( 0==stricmp(sClassName, "ScreenPlayerOptions") )	pNewScreen = new ScreenPlayerOptions;
	else if( 0==stricmp(sClassName, "ScreenSandbox") )			pNewScreen = new ScreenSandbox;
	else if( 0==stricmp(sClassName, "ScreenSelectCourse") )		pNewScreen = new ScreenSelectCourse;
	else if( 0==stricmp(sClassName, "ScreenSelectDifficulty") )	pNewScreen = new ScreenSelectDifficulty;
	else if( 0==stricmp(sClassName, "ScreenSelectGame") )		pNewScreen = new ScreenSelectGame;
	else if( 0==stricmp(sClassName, "ScreenSelectGroup") )		pNewScreen = new ScreenSelectGroup;
	else if( 0==stricmp(sClassName, "ScreenSelectMusic") )		pNewScreen = new ScreenSelectMusic;
	else if( 0==stricmp(sClassName, "ScreenSelectStyle5th") )	pNewScreen = new ScreenSelectStyle5th;
	else if( 0==stricmp(sClassName, "ScreenSelectStyle") )		pNewScreen = new ScreenSelectStyle;
	else if( 0==stricmp(sClassName, "ScreenSongOptions") )		pNewScreen = new ScreenSongOptions;
	else if( 0==stricmp(sClassName, "ScreenStage") )			pNewScreen = new ScreenStage;
	else if( 0==stricmp(sClassName, "ScreenTitleMenu") )		pNewScreen = new ScreenTitleMenu;
	else
		throw RageException( "Invalid Screen class name '%s'", sClassName );

	// move current screen to ScreenToDelete
	RefreshCreditsMessages();
	m_ScreensToDelete.Copy( m_ScreenStack );

	m_ScreenStack.RemoveAll();
	m_ScreenStack.Add( pNewScreen );
}

void ScreenManager::Prompt( ScreenMessage SM_SendWhenDone, CString sText, bool bYesNo, bool bDefaultAnswer, void(*OnYes)(), void(*OnNo)() )
{
	// add the new state onto the back of the array
	m_ScreenStack.Add( new ScreenPrompt(SM_SendWhenDone, sText, bYesNo, bDefaultAnswer, OnYes, OnNo) );
}

void ScreenManager::TextEntry( ScreenMessage SM_SendWhenDone, CString sQuestion, CString sInitialAnswer, void(*OnOK)(CString sAnswer), void(*OnCanel)() )
{	
	// add the new state onto the back of the array
	m_ScreenStack.Add( new ScreenTextEntry(SM_SendWhenDone, sQuestion, sInitialAnswer, OnOK, OnCanel) );
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
	m_textSystemMessage.SetDiffuse( D3DXCOLOR(1,1,1,1) );
	m_textSystemMessage.BeginTweening( 5 );
	m_textSystemMessage.BeginTweening( 1 );
	m_textSystemMessage.SetTweenDiffuse( D3DXCOLOR(1,1,1,0) );

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
		if(      GAMESTATE->m_bSideIsJoined[p] )	m_textCreditInfo[p].SetText( "" );
		else if( GAMESTATE->m_bPlayersCanJoin )	m_textCreditInfo[p].SetText( "PRESS START" );
		else									m_textCreditInfo[p].SetText( "CREDIT(s) 0 / 0" );
	}
}
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
#include "DXUtil.h"


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
	m_ScreenBuffered = NULL;

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
	SAFE_DELETE( m_ScreenBuffered );
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
	for( int i=0; i<m_ScreenStack.GetSize(); i++ ) {
		/* Screens take some time to load.  If we don't do this, then screens
		 * receive an initial update that includes all of the time they spent
		 * loading, which will chop off their tweens.  
		 *
		 * We don't want to simply cap update times; for example, the stage
		 * screen sets a 4 second timer, preps the gameplay screen, and then
		 * displays the prepped screen after the timer runs out; this lets the
		 * load time be masked (as long as the load takes less than 4 seconds).
		 * If we cap that large update delta from the screen load, the update
		 * to load the new screen will come after 4 seconds plus the load time.
		 *
		 * So, let's just drop the first update for every screen.
		 */
		if(m_ScreenStack[i]->FirstUpdate())
			continue;

		m_ScreenStack[i]->Update( fDeltaTime );
	}

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

Screen* ScreenManager::MakeNewScreen( CString sClassName )
{
	if(		 0==stricmp(sClassName, "ScreenAppearanceOptions") )return new ScreenAppearanceOptions;
	else if( 0==stricmp(sClassName, "ScreenCaution") )			return new ScreenCaution;
	else if( 0==stricmp(sClassName, "ScreenEdit") )				return new ScreenEdit;
	else if( 0==stricmp(sClassName, "ScreenEditMenu") )			return new ScreenEditMenu;
	else if( 0==stricmp(sClassName, "ScreenEvaluation") )		return new ScreenEvaluation;
	else if( 0==stricmp(sClassName, "ScreenFinalEvaluation") )	return new ScreenFinalEvaluation;
	else if( 0==stricmp(sClassName, "ScreenEz2SelectPlayer") )	return new ScreenEz2SelectPlayer;
	else if( 0==stricmp(sClassName, "ScreenEz2SelectStyle") )	return new ScreenEz2SelectStyle;
	else if( 0==stricmp(sClassName, "ScreenGameOptions") )		return new ScreenGameOptions;
	else if( 0==stricmp(sClassName, "ScreenGameOver") )			return new ScreenGameOver;
	else if( 0==stricmp(sClassName, "ScreenGameplay") )			return new ScreenGameplay;
	else if( 0==stricmp(sClassName, "ScreenGraphicOptions") )	return new ScreenGraphicOptions;
	else if( 0==stricmp(sClassName, "ScreenHowToPlay") )		return new ScreenHowToPlay;
	else if( 0==stricmp(sClassName, "ScreenMapInstruments") )	return new ScreenMapInstruments;
	else if( 0==stricmp(sClassName, "ScreenMusicScroll") )		return new ScreenMusicScroll;
	else if( 0==stricmp(sClassName, "ScreenPlayerOptions") )	return new ScreenPlayerOptions;
	else if( 0==stricmp(sClassName, "ScreenSandbox") )			return new ScreenSandbox;
	else if( 0==stricmp(sClassName, "ScreenSelectCourse") )		return new ScreenSelectCourse;
	else if( 0==stricmp(sClassName, "ScreenSelectDifficulty") )	return new ScreenSelectDifficulty;
	else if( 0==stricmp(sClassName, "ScreenSelectGame") )		return new ScreenSelectGame;
	else if( 0==stricmp(sClassName, "ScreenSelectGroup") )		return new ScreenSelectGroup;
	else if( 0==stricmp(sClassName, "ScreenSelectMusic") )		return new ScreenSelectMusic;
	else if( 0==stricmp(sClassName, "ScreenSelectStyle5th") )	return new ScreenSelectStyle5th;
	else if( 0==stricmp(sClassName, "ScreenSelectStyle") )		return new ScreenSelectStyle;
	else if( 0==stricmp(sClassName, "ScreenSongOptions") )		return new ScreenSongOptions;
	else if( 0==stricmp(sClassName, "ScreenStage") )			return new ScreenStage;
	else if( 0==stricmp(sClassName, "ScreenTitleMenu") )		return new ScreenTitleMenu;
	else
		throw RageException( "Invalid Screen class name '%s'", sClassName );
}

void ScreenManager::PrepNewScreen( CString sClassName )
{
	if(!sClassName.GetLength()) {
		ASSERT( m_ScreenBuffered != NULL);
		SetNewScreen( m_ScreenBuffered  );
		m_ScreenBuffered = NULL;
	} else {
		ASSERT(m_ScreenBuffered == NULL);
		m_ScreenBuffered = MakeNewScreen(sClassName);
	}
}

void ScreenManager::SetNewScreen( Screen *pNewScreen )
{
	// move current screen to ScreenToDelete
	RefreshCreditsMessages();
	m_ScreensToDelete.Copy( m_ScreenStack );

	m_ScreenStack.RemoveAll();
	m_ScreenStack.Add( pNewScreen );
}

void ScreenManager::SetNewScreen( CString sClassName )
{
	/* If we prepped a screen but didn't use it, nuke it. */
	SAFE_DELETE( m_ScreenBuffered );
	/* Explicitely flush the directory cache each time we load a new screen.
	 * Perhaps we should only do this in debug? */
	FlushDirCache();

	float f = DXUtil_Timer(TIMER_GETAPPTIME);
	// It makes sense that ScreenManager should allocate memory for a new screen since it 
	// deletes it later on.  This also convention will reduce includes because screens won't 
	// have to include each other's headers of other screens.
	Screen* pNewScreen = MakeNewScreen(sClassName);
LOG->Trace( "Loaded %s in %f", sClassName, DXUtil_Timer(TIMER_GETAPPTIME)-f);
	SetNewScreen( pNewScreen );
}

void ScreenManager::Prompt( ScreenMessage SM_SendWhenDone, CString sText, bool bYesNo, bool bDefaultAnswer, void(*OnYes)(), void(*OnNo)() )
{
	FlushDirCache();

	// add the new state onto the back of the array
	m_ScreenStack.Add( new ScreenPrompt(SM_SendWhenDone, sText, bYesNo, bDefaultAnswer, OnYes, OnNo) );
}

void ScreenManager::TextEntry( ScreenMessage SM_SendWhenDone, CString sQuestion, CString sInitialAnswer, void(*OnOK)(CString sAnswer), void(*OnCanel)() )
{	
	FlushDirCache();

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
#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenManager

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
	Tim Hentenaar
-----------------------------------------------------------------------------
*/

#include "ScreenManager.h"
#include "IniFile.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "GameState.h"
#include "RageException.h"
#include "RageTimer.h"
#include "ThemeManager.h"
#include "RageDisplay.h"
#include "Screen.h"
#include "SongManager.h"


ScreenManager*	SCREENMAN = NULL;	// global and accessable from anywhere in our program


#define STATS_X					THEME->GetMetricF("ScreenManager","StatsX")
#define STATS_Y					THEME->GetMetricF("ScreenManager","StatsY")
#define CREDITS_X( p )			THEME->GetMetricF("ScreenManager",ssprintf("CreditsP%dX",p+1))
#define CREDITS_Y( p )			THEME->GetMetricF("ScreenManager",ssprintf("CreditsP%dY",p+1))
#define CREDITS_COLOR			THEME->GetMetricC("ScreenManager","CreditsColor")
#define CREDITS_SHADOW_LENGTH	THEME->GetMetricF("ScreenManager","CreditsShadowLength")
#define CREDITS_ZOOM			THEME->GetMetricF("ScreenManager","CreditsZoom")


ScreenManager::ScreenManager()
{
	m_ScreenBuffered = NULL;

	m_textStats.LoadFromFont( THEME->GetPathTo("Fonts","normal") );
	m_textStats.SetXY( STATS_X, STATS_Y );
	m_textStats.SetHorizAlign( Actor::align_right );
	m_textStats.SetVertAlign( Actor::align_top );
	m_textStats.SetZoom( 0.5f );
	m_textStats.SetShadowLength( 2 );

	// set credits properties on RefreshCreditsMessage

	m_textSystemMessage.LoadFromFont( THEME->GetPathTo("Fonts","normal") );
	m_textSystemMessage.SetHorizAlign( Actor::align_left );
	m_textSystemMessage.SetVertAlign( Actor::align_top );
	m_textSystemMessage.SetXY( 4.0f, 4.0f );
	m_textSystemMessage.SetZoom( 0.7f );
//	m_textSystemMessage.SetShadowLength( 2 );
	m_textSystemMessage.SetDiffuse( RageColor(1,1,1,0) );

	m_textSysTime.LoadFromFont( THEME->GetPathTo("Fonts","normal") );
	m_textSysTime.SetXY( 4.0f, 40.0f );
	m_textSysTime.SetHorizAlign( Actor::align_left );
	m_textSysTime.SetVertAlign( Actor::align_top );
	m_textSysTime.SetZoom( 0.5f );
	m_textSysTime.SetDiffuse( RageColor(1,0,1,1) );
	m_textSysTime.EnableShadow(false);
}


ScreenManager::~ScreenManager()
{
	LOG->Trace( "ScreenManager::~ScreenManager()" );

	EmptyDeleteQueue();

	// delete current states
	for( unsigned i=0; i<m_ScreenStack.size(); i++ )
		SAFE_DELETE( m_ScreenStack[i] );
	SAFE_DELETE( m_ScreenBuffered );
}

void ScreenManager::EmptyDeleteQueue()
{
	// delete all ScreensToDelete
	for( unsigned i=0; i<m_ScreensToDelete.size(); i++ )
		SAFE_DELETE( m_ScreensToDelete[i] );

	m_ScreensToDelete.clear();
}

void ScreenManager::Update( float fDeltaTime )
{
	m_textSystemMessage.Update( fDeltaTime );
	m_textStats.Update( fDeltaTime );
	for( int p=0; p<NUM_PLAYERS; p++ )
		m_textCreditInfo[p].Update( fDeltaTime );
	m_textSysTime.Update( fDeltaTime );

	EmptyDeleteQueue();

	// Only update the topmost screen on the stack.

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
	Screen* pScreen = m_ScreenStack[m_ScreenStack.size()-1];
	if( pScreen->IsFirstUpdate() )
		pScreen->Update( 0 );
	else
		pScreen->Update( fDeltaTime );
}


void ScreenManager::Restore()
{
	// Restore all CurrentScreens (back to front)
	for( unsigned i=0; i<m_ScreenStack.size(); i++ )
		m_ScreenStack[i]->Restore();
}

void ScreenManager::Invalidate()
{
	for( unsigned i=0; i<m_ScreenStack.size(); i++ )
		m_ScreenStack[i]->Invalidate();
}

void ScreenManager::Draw()
{
	if( !m_ScreenStack.empty() && !m_ScreenStack.back()->IsTransparent() )	// top screen isn't transparent
		m_ScreenStack.back()->Draw();
	else
		for( unsigned i=0; i<m_ScreenStack.size(); i++ )	// Draw all screens bottom to top
			m_ScreenStack[i]->Draw();

	
	if( m_textSystemMessage.GetDiffuse().a != 0 )
		m_textSystemMessage.Draw();

	if( PREFSMAN  &&  PREFSMAN->m_bShowStats )
	{
		/* If FPS == 0, we don't have stats yet. */
		m_textStats.SetText( ssprintf(DISPLAY->GetFPS()?
			"%i FPS\n%i av FPS\n%i VPF":
			"-- FPS\n-- av FPS\n-- VPF",
			DISPLAY->GetFPS(), DISPLAY->GetCumFPS(), 
			DISPLAY->GetVPF()) );

		m_textStats.Draw();
	}
	for( int p=0; p<NUM_PLAYERS; p++ )
		m_textCreditInfo[p].Draw();

	if(PREFSMAN->m_bTimestamping)
	{
		m_textSysTime.SetText( SecondsToTime(RageTimer::GetTimeSinceStart()) );
		m_textSysTime.Draw();
	}
}


void ScreenManager::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
//	LOG->Trace( "ScreenManager::Input( %d-%d, %d-%d, %d-%d, %d-%d )", 
//		DeviceI.device, DeviceI.button, GameI.controller, GameI.button, MenuI.player, MenuI.button, StyleI.player, StyleI.col );

	// pass input only to topmost state
	if( !m_ScreenStack.empty() )
		m_ScreenStack.back()->Input( DeviceI, type, GameI, MenuI, StyleI );
}


Screen* ScreenManager::MakeNewScreen( CString sClassName )
{
	Screen *ret = Screen::Create( sClassName );

	/* Loading probably took a little while.  Let's reset stats.  This prevents us
	 * from displaying an unnaturally low FPS value, and the next FPS value we
	 * display will be accurate, which makes skips in the initial tween-ins more
	 * apparent. */
	DISPLAY->ResetStats();

	/* This is a convenient time to clean up our song cache. */
	SONGMAN->CleanData();

	return ret;
}

void ScreenManager::PrepNewScreen( CString sClassName )
{
	ASSERT(m_ScreenBuffered == NULL);
	m_ScreenBuffered = MakeNewScreen(sClassName);
}

void ScreenManager::LoadPreppedScreen()
{
	ASSERT( m_ScreenBuffered != NULL);
	SetNewScreen( m_ScreenBuffered  );
	m_ScreenBuffered = NULL;
}

void ScreenManager::SetNewScreen( Screen *pNewScreen )
{
	RefreshCreditsMessages();

	// move current screen(s) to ScreenToDelete
	m_ScreensToDelete.insert(m_ScreensToDelete.end(), m_ScreenStack.begin(), m_ScreenStack.end());

	m_ScreenStack.clear();
	m_ScreenStack.push_back( pNewScreen );
}

void ScreenManager::SetNewScreen( CString sClassName )
{
	/* If we prepped a screen but didn't use it, nuke it. */
	SAFE_DELETE( m_ScreenBuffered );

	RageTimer t;
	
	// It makes sense that ScreenManager should allocate memory for a new screen since it 
	// deletes it later on.  This also convention will reduce includes because screens won't 
	// have to include each other's headers of other screens.
	Screen* pNewScreen = MakeNewScreen(sClassName);
	LOG->Trace( "Loaded %s in %f", sClassName.GetString(), t.GetDeltaTime());

	SetNewScreen( pNewScreen );

	/* If this is a system menu, don't let the operator key touch it! 
		However, if you add an options screen, please include it here -- Miryokuteki */
	if(	sClassName == "ScreenOptionsMenu" || sClassName == "ScreenMachineOptions" || 
		 sClassName == "ScreenOptions" || sClassName == "ScreenInputOptions" || 
		 sClassName == "ScreenGraphicOptions" || sClassName == "ScreenGameplayOptions" || 
		 sClassName == "ScreenMapControllers" || sClassName == "ScreenPlayerOptions" || 
		 sClassName == "ScreenAppearanceOptions" || sClassName == "ScreenEdit" || 
		 sClassName == "ScreenEditMenu" || sClassName == "ScreenSoundOptions") GAMESTATE->m_bIsOnSystemMenu = true;
	else GAMESTATE->m_bIsOnSystemMenu = false;
}

void ScreenManager::AddNewScreenToTop( CString sClassName )
{	
	Screen* pNewScreen = MakeNewScreen(sClassName);
	m_ScreenStack.push_back( pNewScreen );
}

#include "ScreenPrompt.h"
#include "ScreenTextEntry.h"
#include "ScreenMiniMenu.h"

void ScreenManager::Prompt( ScreenMessage SM_SendWhenDone, CString sText, bool bYesNo, bool bDefaultAnswer, void(*OnYes)(), void(*OnNo)() )
{
	// add the new state onto the back of the array
	m_ScreenStack.push_back( new ScreenPrompt(SM_SendWhenDone, sText, bYesNo, bDefaultAnswer, OnYes, OnNo) );
}

void ScreenManager::TextEntry( ScreenMessage SM_SendWhenDone, CString sQuestion, CString sInitialAnswer, void(*OnOK)(CString sAnswer), void(*OnCanel)() )
{	
	// add the new state onto the back of the array
	m_ScreenStack.push_back( new ScreenTextEntry(SM_SendWhenDone, sQuestion, sInitialAnswer, OnOK, OnCanel) );
}

void ScreenManager::MiniMenu( MiniMenuDefinition* pDef, ScreenMessage SM_SendOnOK, ScreenMessage SM_SendOnCancel )
{
	// add the new state onto the back of the array
	m_ScreenStack.push_back( new ScreenMiniMenu(pDef, SM_SendOnOK, SM_SendOnCancel) );
}

void ScreenManager::PopTopScreen( ScreenMessage SM )
{
	Screen* pScreenToPop = m_ScreenStack.back();	// top menu
	//pScreenToPop->HandleScreenMessage( SM_LosingInputFocus );
	m_ScreenStack.erase(m_ScreenStack.end()-1, m_ScreenStack.end());
	m_ScreensToDelete.push_back( pScreenToPop );
	
	Screen* pNewTopScreen = m_ScreenStack[m_ScreenStack.size()-1];
	pNewTopScreen->HandleScreenMessage( SM );
}

void ScreenManager::SendMessageToTopScreen( ScreenMessage SM, float fDelay )
{
	Screen* pTopScreen = m_ScreenStack.back();
	pTopScreen->SendScreenMessage( SM, fDelay );
}


void ScreenManager::SystemMessage( CString sMessage )
{
	// Look for an open spot
	m_textSystemMessage.StopTweening();
	m_textSystemMessage.SetText( sMessage );
	m_textSystemMessage.SetDiffuse( RageColor(1,1,1,1) );
	m_textSystemMessage.BeginTweening( 5 );
	m_textSystemMessage.BeginTweening( 1 );
	m_textSystemMessage.SetTweenDiffuse( RageColor(1,1,1,0) );

	LOG->Trace( "WARNING:  Didn't find an empty system messages slot." );
}

void ScreenManager::RefreshCreditsMessages()
{
	// update joined
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		m_textCreditInfo[p].LoadFromFont( THEME->GetPathTo("Fonts","credits") );
		m_textCreditInfo[p].SetXY( CREDITS_X(p), CREDITS_Y(p) );
		m_textCreditInfo[p].SetZoom( CREDITS_ZOOM );
		m_textCreditInfo[p].SetDiffuse( CREDITS_COLOR );
		m_textCreditInfo[p].SetShadowLength( CREDITS_SHADOW_LENGTH );
		
		switch( PREFSMAN->m_CoinMode )
		{
		case PrefsManager::COIN_HOME:
			if( GAMESTATE->m_bSideIsJoined[p] )
				m_textCreditInfo[p].SetText( "" );
			else if( GAMESTATE->m_bPlayersCanJoin )		// would  (GAMESTATE->m_CurStyle!=STYLE_INVALID) do the same thing?
				m_textCreditInfo[p].SetText( "PRESS START" );
			else
				m_textCreditInfo[p].SetText( "NOT PRESENT" );
			break;
		case PrefsManager::COIN_PAY:
			{
				int Coins = GAMESTATE->m_iCoins % PREFSMAN->m_iCoinsPerCredit;
				CString txt = ssprintf("CREDIT(S) %d ", GAMESTATE->m_iCoins / PREFSMAN->m_iCoinsPerCredit);
				if (Coins)
					txt += ssprintf(" (%d / %d)", Coins, PREFSMAN->m_iCoinsPerCredit );
				m_textCreditInfo[p].SetText(txt);
			}
			break;
		case PrefsManager::COIN_FREE:
			m_textCreditInfo[p].SetText( "FREE PLAY" );
			break;
		default:
			ASSERT(0);
		}
	}
}


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

}


void ScreenManager::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
//	LOG->Trace( "ScreenManager::Input( %d-%d, %d-%d, %d-%d, %d-%d )", 
//		DeviceI.device, DeviceI.button, GameI.controller, GameI.button, MenuI.player, MenuI.button, StyleI.player, StyleI.col );

	// pass input only to topmost state
	if( !m_ScreenStack.empty() )
		m_ScreenStack.back()->Input( DeviceI, type, GameI, MenuI, StyleI );
}


// Screen classes
#include "ScreenAppearanceOptions.h"
#include "ScreenCaution.h"
#include "ScreenEdit.h"
#include "ScreenEditMenu.h"
#include "ScreenEvaluation.h"
#include "ScreenEz2SelectPlayer.h"
#include "ScreenSelectMode.h"
#include "ScreenGameOver.h"
#include "ScreenGameplay.h"
#include "ScreenGraphicOptions.h"
#include "ScreenHowToPlay.h"
#include "ScreenInputOptions.h"
#include "ScreenMachineOptions.h"
#include "ScreenMapControllers.h"
#include "ScreenMusicScroll.h"
#include "ScreenPlayerOptions.h"
#include "ScreenSelectCourse.h"
#include "ScreenSelectDifficulty.h"
#include "ScreenSelectDifficultyEX.h"
#include "ScreenSelectGame.h"
#include "ScreenSelectGroup.h"
#include "ScreenSelectMusic.h"
#include "ScreenSelectStyle5th.h"
#include "ScreenSelectStyle.h"
#include "ScreenSongOptions.h"
#include "ScreenSoundOptions.h"
#include "ScreenStage.h"
#include "ScreenTest.h"
#include "ScreenTestFonts.h"
#include "ScreenTestSound.h"
#include "ScreenTitleMenu.h"
#include "ScreenEz2SelectMusic.h"
#include "ScreenWarning.h"
#include "ScreenRanking.h"
#include "ScreenMemoryCard.h"
#include "ScreenCompany.h"
#include "ScreenIntroMovie.h"
#include "ScreenAlbums.h"
#include "ScreenLogo.h"
#include "ScreenUnlock.h"
#include "ScreenDemonstration.h"
#include "ScreenInstructions.h"
#include "ScreenNameEntry.h"
#include "ScreenJukebox.h"
#include "ScreenJukeboxMenu.h"
#include "ScreenOptionsMenu.h"
#include "ScreenGameplayOptions.h"
#include "ScreenStyleSplash.h"

#include "ScreenPrompt.h"
#include "ScreenTextEntry.h"
#include "ScreenMiniMenu.h"

Screen* ScreenManager::MakeNewScreen( CString sClassName )
{
#define RETURN_IF_MATCH(className)	if(0==stricmp(sClassName,"##className##")) return new className
	Screen *ret = NULL;

	if(		 0==stricmp(sClassName, "ScreenAppearanceOptions") )	ret = new ScreenAppearanceOptions;
	else if( 0==stricmp(sClassName, "ScreenCaution") )				ret = new ScreenCaution;
	else if( 0==stricmp(sClassName, "ScreenEdit") )					ret = new ScreenEdit;
	else if( 0==stricmp(sClassName, "ScreenEditMenu") )				ret = new ScreenEditMenu;
	else if( 0==stricmp(sClassName, "ScreenEvaluation") )			ret = new ScreenEvaluation;
	else if( 0==stricmp(sClassName, "ScreenFinalEvaluation") )		ret = new ScreenFinalEvaluation;
	else if( 0==stricmp(sClassName, "ScreenEz2SelectPlayer") )		ret = new ScreenEz2SelectPlayer;
	else if( 0==stricmp(sClassName, "ScreenSelectMode") )			ret = new ScreenSelectMode;
	else if( 0==stricmp(sClassName, "ScreenGameOver") )				ret = new ScreenGameOver;
	else if( 0==stricmp(sClassName, "ScreenGameplay") )				ret = new ScreenGameplay;
	else if( 0==stricmp(sClassName, "ScreenGraphicOptions") )		ret = new ScreenGraphicOptions;
	else if( 0==stricmp(sClassName, "ScreenHowToPlay") )			ret = new ScreenHowToPlay;
	else if( 0==stricmp(sClassName, "ScreenInputOptions") )			ret = new ScreenInputOptions;
	else if( 0==stricmp(sClassName, "ScreenMachineOptions") )		ret = new ScreenMachineOptions;
	else if( 0==stricmp(sClassName, "ScreenMapControllers") )		ret = new ScreenMapControllers;
	else if( 0==stricmp(sClassName, "ScreenInputOptions") )			ret = new ScreenInputOptions;
	else if( 0==stricmp(sClassName, "ScreenMusicScroll") )			ret = new ScreenMusicScroll;
	else if( 0==stricmp(sClassName, "ScreenPlayerOptions") )		ret = new ScreenPlayerOptions;
	else if( 0==stricmp(sClassName, "ScreenSelectCourse") )			ret = new ScreenSelectCourse;
	else if( 0==stricmp(sClassName, "ScreenSelectDifficulty") )		ret = new ScreenSelectDifficulty;
	else if( 0==stricmp(sClassName, "ScreenSelectDifficultyEX") )	ret = new ScreenSelectDifficultyEX;
	else if( 0==stricmp(sClassName, "ScreenSelectGame") )			ret = new ScreenSelectGame;
	else if( 0==stricmp(sClassName, "ScreenSelectGroup") )			ret = new ScreenSelectGroup;
	else if( 0==stricmp(sClassName, "ScreenSelectMusic") )			ret = new ScreenSelectMusic;
	else if( 0==stricmp(sClassName, "ScreenSelectStyle5th") )		ret = new ScreenSelectStyle5th;
	else if( 0==stricmp(sClassName, "ScreenSelectStyle") )			ret = new ScreenSelectStyle;
	else if( 0==stricmp(sClassName, "ScreenSongOptions") )			ret = new ScreenSongOptions;
	else if( 0==stricmp(sClassName, "ScreenStage") )				ret = new ScreenStage;
	else if( 0==stricmp(sClassName, "ScreenTest") )					ret = new ScreenTest;
	else if( 0==stricmp(sClassName, "ScreenTestFonts") )			ret = new ScreenTestFonts;
	else if( 0==stricmp(sClassName, "ScreenTestSound") )			ret = new ScreenTestSound;
	else if( 0==stricmp(sClassName, "ScreenTitleMenu") )			ret = new ScreenTitleMenu;
	else if( 0==stricmp(sClassName, "ScreenEz2SelectMusic") )		ret = new ScreenEz2SelectMusic;
	else if( 0==stricmp(sClassName, "ScreenWarning") )				ret = new ScreenWarning;
	else if( 0==stricmp(sClassName, "ScreenRanking") )				ret = new ScreenRanking;
	else if( 0==stricmp(sClassName, "ScreenMemoryCard") )			ret = new ScreenMemoryCard;
	else if( 0==stricmp(sClassName, "ScreenCompany") )				ret = new ScreenCompany;
	else if( 0==stricmp(sClassName, "ScreenIntroMovie") )			ret = new ScreenIntroMovie;
	else if( 0==stricmp(sClassName, "ScreenAlbums") )				ret = new ScreenAlbums;
	else if( 0==stricmp(sClassName, "ScreenLogo") )					ret = new ScreenLogo;
	else if( 0==stricmp(sClassName, "ScreenUnlock") )				ret = new ScreenUnlock;
	else if( 0==stricmp(sClassName, "ScreenDemonstration") )		ret = (ScreenGameplay*)new ScreenDemonstration;
	else if( 0==stricmp(sClassName, "ScreenInstructions") )			ret = new ScreenInstructions;
	else if( 0==stricmp(sClassName, "ScreenNameEntry") )			ret = new ScreenNameEntry;
	else if( 0==stricmp(sClassName, "ScreenJukebox") )				ret = new ScreenJukebox;
	else if( 0==stricmp(sClassName, "ScreenJukeboxMenu") )			ret = new ScreenJukeboxMenu;
	else if( 0==stricmp(sClassName, "ScreenOptionsMenu") )			ret = new ScreenOptionsMenu;
	else if( 0==stricmp(sClassName, "ScreenSoundOptions") )			ret = new ScreenSoundOptions;
	else if( 0==stricmp(sClassName, "ScreenGameplayOptions") )		ret = new ScreenGameplayOptions;
	else if( 0==stricmp(sClassName, "ScreenStyleSplash") )			ret = new ScreenStyleSplash;
	else
		RageException::Throw( "Invalid Screen class name '%s'", sClassName.GetString() );

	/* Give a null update to the new screen.  This bumps everything into its
	 * initial tween state, if any, so we don't show stuff at an incorrect
	 * position for a frame. 
	 *
	 * XXX: Can't do this here, since this update might cause another screen
	 * to be loaded; that'll land back here, and the screen list gets messed
	 * up.  We need to make sure Update(0) is called some time before the
	 * first Draw(). */
//	ret->Update(0);

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
			else if( GAMESTATE->m_bPlayersCanJoin )
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


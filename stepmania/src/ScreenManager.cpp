/*
 * SM_GainFocus/SM_LoseFocus: These are sent to screens when they become the
 * topmost screen, or stop being the topmost screen.
 *
 * A few subtleties:
 *
 * With delayed screens (eg. ScreenGameplay being pre-loaded by ScreenStage), SM_GainFocus
 * isn't sent until the loaded screen actually is activated (put on the stack).
 *
 * With normal screen loads, the new screen is loaded before the old screen is destroyed.
 * This means that the old dtor is called *after* the new ctor.  If some global properties
 * (eg. GAMESTATE) are being unset by the old screen's destructor, and set by the new
 * screen's constructor, they'll happen in the wrong order.  Use SM_GainFocus and
 * SM_LoseFocus, instead.
 *
 * SM_LoseFocus is always sent after SM_GainFocus, and vice-versa: you can't gain focus
 * if you already have it, and you can't lose focus if you don't have it.
 */

#include "global.h"
#include "ScreenManager.h"
#include "IniFile.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "ActorUtil.h"
#include "GameState.h"
#include "RageException.h"
#include "RageTimer.h"
#include "GameSoundManager.h"
#include "ThemeManager.h"
#include "RageDisplay.h"
#include "Screen.h"
#include "SongManager.h"
#include "BitmapText.h"
#include "Quad.h"
#include "RageTextureManager.h"
#include "ProfileManager.h"
#include "ThemeManager.h"
#include "MemoryCardManager.h"
#include "CodeDetector.h"
#include "StepMania.h"

ScreenManager*	SCREENMAN = NULL;	// global and accessable from anywhere in our program


#define CREDITS_PRESS_START		THEME->GetMetric ("ScreenSystemLayer","CreditsPressStart")
#define CREDITS_INSERT_CARD		THEME->GetMetric ("ScreenSystemLayer","CreditsInsertCard")
#define CREDITS_CARD_ERROR		THEME->GetMetric ("ScreenSystemLayer","CreditsCardError")
#define CREDITS_CARD_TOO_LATE	THEME->GetMetric ("ScreenSystemLayer","CreditsCardTooLate")
#define CREDITS_CARD_NO_NAME	THEME->GetMetric ("ScreenSystemLayer","CreditsCardNoName")
#define CREDITS_FREE_PLAY		THEME->GetMetric ("ScreenSystemLayer","CreditsFreePlay")
#define CREDITS_CREDITS			THEME->GetMetric ("ScreenSystemLayer","CreditsCredits")
#define CREDITS_NOT_PRESENT		THEME->GetMetric ("ScreenSystemLayer","CreditsNotPresent")
#define CREDITS_JOIN_ONLY		THEME->GetMetricB("ScreenSystemLayer","CreditsJoinOnly")

const int NUM_SKIPS_TO_SHOW = 5;

/* This screen is drawn on top of everything else, and receives updates,
 * but not input. */
class ScreenSystemLayer: public Screen
{
	BitmapText m_textStats;
	BitmapText m_textMessage;
	BitmapText m_textCredits[NUM_PLAYERS];
	BitmapText m_textTime;
	BitmapText m_textSkips[NUM_SKIPS_TO_SHOW];
	int m_LastSkip;
	Quad m_SkipBackground;

	RageTimer SkipTimer;
	void AddTimestampLine( const CString &txt, RageColor color );
	void UpdateTimestampAndSkips();

public:
	ScreenSystemLayer();
	void SystemMessage( CString sMessage );
	void SystemMessageNoAnimate( CString sMessage );
	void ReloadCreditsText();
	void RefreshCreditsMessages();
	void Update( float fDeltaTime );
};

ScreenSystemLayer::ScreenSystemLayer() : Screen("ScreenSystemLayer")
{
	this->AddChild(&m_textMessage);
	this->AddChild(&m_textStats);
	this->AddChild(&m_textTime);
	FOREACH_PlayerNumber( p )
		this->AddChild(&m_textCredits[p]);


	/* "Was that a skip?"  This displays a message when an update takes
	 * abnormally long, to quantify skips more precisely, verify them
	 * when they're subtle, and show the time it happened, so you can pinpoint
	 * the time in the log.  Put a dim quad behind it to make it easier to
	 * read. */
	m_LastSkip = 0;
	const float SKIP_LEFT = 320.0f, SKIP_TOP = 60.0f, 
		SKIP_WIDTH = 160.0f, SKIP_Y_DIST = 16.0f;

	m_SkipBackground.StretchTo(RectF(SKIP_LEFT-8, SKIP_TOP-8,
						SKIP_LEFT+SKIP_WIDTH, SKIP_TOP+SKIP_Y_DIST*NUM_SKIPS_TO_SHOW));
	m_SkipBackground.SetDiffuse( RageColor(0,0,0,0.4f) );
	m_SkipBackground.SetHidden( !PREFSMAN->m_bTimestamping );
	this->AddChild(&m_SkipBackground);

	for( int i=0; i<NUM_SKIPS_TO_SHOW; i++ )
	{
		/* This is somewhat big.  Let's put it on the right side, where it'll
		 * obscure the 2P side during gameplay; there's nowhere to put it that
		 * doesn't obscure something, and it's just a diagnostic. */
		m_textSkips[i].LoadFromFont( THEME->GetPathToF("Common normal") );
		m_textSkips[i].SetXY( SKIP_LEFT, SKIP_TOP + SKIP_Y_DIST*i );
		m_textSkips[i].SetHorizAlign( Actor::align_left );
		m_textSkips[i].SetVertAlign( Actor::align_top );
		m_textSkips[i].SetZoom( 0.5f );
		m_textSkips[i].SetDiffuse( RageColor(1,1,1,0) );
		m_textSkips[i].SetShadowLength( 0 );
		this->AddChild(&m_textSkips[i]);
	}

	ReloadCreditsText();
	/* This will be done when we set up the first screen, after GAMESTATE->Reset has
	 * been called. */
	// RefreshCreditsMessages();
}

void ScreenSystemLayer::ReloadCreditsText()
{
	m_textMessage.LoadFromFont( THEME->GetPathToF("ScreenSystemLayer message") );
	m_textMessage.SetName( "Message" );
	SET_XY_AND_ON_COMMAND( m_textMessage ); 

	m_textStats.LoadFromFont( THEME->GetPathToF("ScreenSystemLayer stats") );
	m_textStats.SetName( "Stats" );
	SET_XY_AND_ON_COMMAND( m_textStats ); 

	m_textTime.LoadFromFont( THEME->GetPathToF("ScreenSystemLayer time") );
	m_textTime.SetName( "Time" );
	m_textTime.SetHidden( !PREFSMAN->m_bTimestamping );
	SET_XY_AND_ON_COMMAND( m_textTime ); 

	FOREACH_PlayerNumber( p )
	{
		m_textCredits[p].LoadFromFont( THEME->GetPathToF("ScreenManager credits") );
		m_textCredits[p].SetName( ssprintf("CreditsP%d",p+1) );
		SET_XY_AND_ON_COMMAND( &m_textCredits[p] );
	}
}

void ScreenSystemLayer::SystemMessage( CString sMessage )
{
	m_textMessage.FinishTweening();
	m_textMessage.SetText( sMessage );
	m_textMessage.SetDiffuseAlpha( 1 );
	m_textMessage.SetX( -640 );
	m_textMessage.BeginTweening( 0.5f );
	m_textMessage.SetX( 4 );
	m_textMessage.BeginTweening( 5 );
	m_textMessage.BeginTweening( 0.5f );
	m_textMessage.SetDiffuse( RageColor(1,1,1,0) );
}

void ScreenSystemLayer::SystemMessageNoAnimate( CString sMessage )
{
	m_textMessage.FinishTweening();
	m_textMessage.SetText( sMessage );
	m_textMessage.SetX( 4 );
	m_textMessage.SetDiffuseAlpha( 1 );
	m_textMessage.BeginTweening( 5 );
	m_textMessage.BeginTweening( 0.5f );
	m_textMessage.SetDiffuse( RageColor(1,1,1,0) );
}

void ScreenSystemLayer::RefreshCreditsMessages()
{
	// update joined
	FOREACH_PlayerNumber( p )
	{
		CString sCredits;

		bool bShowCreditsMessage;
		if( GAMESTATE->m_bIsOnSystemMenu )
			bShowCreditsMessage = true;
		else if( GAMESTATE->m_bPlayersFinalized )
			bShowCreditsMessage = !GAMESTATE->IsPlayerEnabled( p );	
		else 
			bShowCreditsMessage = !GAMESTATE->m_bSideIsJoined[p];
		
		if( !bShowCreditsMessage )
		{
			MemoryCardState mcs = MEMCARDMAN->GetCardState( p );
			Profile* pProfile = PROFILEMAN->GetProfile( p );
			switch( mcs )
			{
			case MEMORY_CARD_STATE_NO_CARD:
				if( pProfile )	// this is a local machine profile
					sCredits = pProfile->GetDisplayName();
				else if( GAMESTATE->PlayersCanJoin() )
					sCredits = CREDITS_INSERT_CARD;
				else
					sCredits = "";
				break;
			case MEMORY_CARD_STATE_WRITE_ERROR:
				sCredits = CREDITS_CARD_ERROR;
				break;
			case MEMORY_CARD_STATE_TOO_LATE:
				sCredits = CREDITS_CARD_TOO_LATE;
				break;
			case MEMORY_CARD_STATE_READY:
				if( !MEMCARDMAN->GetName(p).empty() )
					sCredits = MEMCARDMAN->GetName(p);
				else	
					sCredits = CREDITS_CARD_NO_NAME;
				break;
			default:
				ASSERT(0);
			}
		}
		else // bShowCreditsMessage
		{
			switch( PREFSMAN->GetCoinMode() )
			{
			case COIN_HOME:
				if( GAMESTATE->PlayersCanJoin() )
					sCredits = CREDITS_PRESS_START;
				else
					sCredits = CREDITS_NOT_PRESENT;
				break;
			case COIN_PAY:
				{
					int Credits = GAMESTATE->m_iCoins / PREFSMAN->m_iCoinsPerCredit;
					int Coins = GAMESTATE->m_iCoins % PREFSMAN->m_iCoinsPerCredit;
					sCredits = CREDITS_CREDITS;
					if( Credits > 0 || PREFSMAN->m_iCoinsPerCredit == 1 )
						sCredits += ssprintf("  %d", Credits);
					if( PREFSMAN->m_iCoinsPerCredit > 1 )
						sCredits += ssprintf("  %d/%d", Coins, PREFSMAN->m_iCoinsPerCredit );
				}
				break;
			case COIN_FREE:
				if( GAMESTATE->PlayersCanJoin() )
					sCredits = CREDITS_FREE_PLAY;
				else
					sCredits = CREDITS_NOT_PRESENT;
				break;
			default:
				ASSERT(0);
			}
		}

		if( CREDITS_JOIN_ONLY && !GAMESTATE->PlayersCanJoin() )
			sCredits = "";
		m_textCredits[p].SetText( sCredits );
	}
}

void ScreenSystemLayer::AddTimestampLine( const CString &txt, RageColor color )
{
	m_textSkips[m_LastSkip].SetText( txt );
	m_textSkips[m_LastSkip].StopTweening();
	m_textSkips[m_LastSkip].SetDiffuse( RageColor(1,1,1,1) );
	m_textSkips[m_LastSkip].BeginTweening( 0.2f );
	m_textSkips[m_LastSkip].SetDiffuse( color );
	m_textSkips[m_LastSkip].BeginTweening( 3.0f );
	m_textSkips[m_LastSkip].BeginTweening( 0.2f );
	m_textSkips[m_LastSkip].SetDiffuse( RageColor(1,1,1,0) );

	m_LastSkip++;
	m_LastSkip %= NUM_SKIPS_TO_SHOW;
}

void ScreenSystemLayer::UpdateTimestampAndSkips()
{
	/* Use our own timer, so we ignore `/tab. */
	const float UpdateTime = SkipTimer.GetDeltaTime();

	/* FPS is 0 for a little while after we load a screen; don't report
	 * during this time. Do clear the timer, though, so we don't report
	 * a big "skip" after this period passes. */
	if(DISPLAY->GetFPS())
	{
		/* We want to display skips.  We expect to get updates of about 1.0/FPS ms. */
		const float ExpectedUpdate = 1.0f / DISPLAY->GetFPS();
		
		/* These are thresholds for severity of skips.  The smallest
		 * is slightly above expected, to tolerate normal jitter. */
		const float Thresholds[] = {
			ExpectedUpdate * 2.0f, ExpectedUpdate * 4.0f, 0.1f, -1
		};

		int skip = 0;
		while(Thresholds[skip] != -1 && UpdateTime > Thresholds[skip])
			skip++;

		if(skip)
		{
			CString time(SecondsToMMSSMsMs(RageTimer::GetTimeSinceStart()));

			static const RageColor colors[] = {
				RageColor(0,0,0,0),		  /* unused */
				RageColor(0.2f,0.2f,1,1), /* light blue */
				RageColor(1,1,0,1),		  /* yellow */
				RageColor(1,0.2f,0.2f,1)  /* light red */
			};

			if( PREFSMAN->m_bTimestamping )
				AddTimestampLine( ssprintf("%s: %.0fms (%.0f)", time.c_str(), 1000*UpdateTime, UpdateTime/ExpectedUpdate),
					colors[skip] );
			if( PREFSMAN->m_bLogSkips )
				LOG->Trace( "Frame skip: %.0fms (%.0f)", 1000*UpdateTime, UpdateTime/ExpectedUpdate );
		}
	}

	if( PREFSMAN->m_bTimestamping )
		m_textTime.SetText( SecondsToMMSSMsMs(RageTimer::GetTimeSinceStart()) );
}

void ScreenSystemLayer::Update( float fDeltaTime )
{
	Screen::Update(fDeltaTime);

	if( PREFSMAN  &&  PREFSMAN->m_bShowStats )
	{
		m_textStats.SetDiffuse( RageColor(1,1,1,0.7f) );

		/* If FPS == 0, we don't have stats yet. */
		if(DISPLAY->GetFPS())
			m_textStats.SetText( ssprintf(
				"%i FPS\n%i av FPS\n%i VPF",
				DISPLAY->GetFPS(), DISPLAY->GetCumFPS(), 
				DISPLAY->GetVPF()) );
		else
			m_textStats.SetText( "-- FPS\n-- av FPS\n-- VPF" );
	} else
		m_textStats.SetDiffuse( RageColor(1,1,1,0) ); /* hide */

	UpdateTimestampAndSkips();
}

ScreenManager::ScreenManager()
{
	m_SystemLayer = NULL;

	/* By the time this is constructed, THEME has already been set up and set to
	 * the current theme.  Call ThemeChanged(), to handle the starting theme
	 * and set up m_SystemLayer. */
	ASSERT( THEME );
	ASSERT( !THEME->GetCurThemeName().empty() );
	this->ThemeChanged();

	m_ScreenBuffered = NULL;

	m_MessageSendOnPop = SM_None;
}


ScreenManager::~ScreenManager()
{
	LOG->Trace( "ScreenManager::~ScreenManager()" );

	EmptyDeleteQueue();

	// delete current Screens
	for( unsigned i=0; i<m_ScreenStack.size(); i++ )
		delete m_ScreenStack[i];
	delete m_ScreenBuffered;
	delete m_SystemLayer;
}

/* This is called when we start up, and when the theme changes or is reloaded. */
void ScreenManager::ThemeChanged()
{
	LOG->Trace( "ScreenManager::ThemeChanged" );

	// reload common sounds
	m_soundStart.Load( THEME->GetPathS("Common","start") );
	m_soundCoin.Load( THEME->GetPathS("Common","coin") );
	m_soundInvalid.Load( THEME->GetPathS("Common","invalid") );
	m_soundScreenshot.Load( THEME->GetPathS("Common","screenshot") );
	m_soundBack.Load( THEME->GetPathS("Common","back") );

	// reload system layer
	delete m_SystemLayer;
	m_SystemLayer = NULL; // new ScreenSystemLayer may throw
	m_SystemLayer = new ScreenSystemLayer;
	m_SystemLayer->RefreshCreditsMessages();
}

void ScreenManager::EmptyDeleteQueue()
{
	if(!m_ScreensToDelete.size())
		return;

	// delete all ScreensToDelete
	for( unsigned i=0; i<m_ScreensToDelete.size(); i++ )
		SAFE_DELETE( m_ScreensToDelete[i] );

	m_ScreensToDelete.clear();

	/* Now that we've actually deleted a screen, it makes sense to clear out
	 * cached textures. */
	TEXTUREMAN->DeleteCachedTextures();
	TEXTUREMAN->DiagnosticOutput();
}

Screen *ScreenManager::GetTopScreen()
{
	if( m_ScreenStack.empty() )
		return NULL;
	return m_ScreenStack[m_ScreenStack.size()-1];
}

void ScreenManager::Update( float fDeltaTime )
{
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
	 * So, let's just zero the first update for every screen.
	 */
	ASSERT( !m_ScreenStack.empty() || m_DelayedScreen != "" );	// Why play the game if there is nothing showing?

	if( !m_ScreenStack.empty() )
	{
		Screen* pScreen = GetTopScreen();
		if( pScreen->IsFirstUpdate() )
			pScreen->Update( 0 );
		else
			pScreen->Update( fDeltaTime );
	}

	m_SystemLayer->Update( fDeltaTime );

	EmptyDeleteQueue();

	if(m_DelayedScreen.size() != 0)
	{
		/* We have a screen to display.  Delete the current screens and load it. */
		ClearScreenStack();
		EmptyDeleteQueue();

		/* This is the purpose of delayed screen loads: clear out the texture cache
		 * now, while there's (mostly) nothing loaded. */
		TEXTUREMAN->DeleteCachedTextures();
		TEXTUREMAN->DiagnosticOutput();

		LoadDelayedScreen();
	}
}


void ScreenManager::Draw()
{
	/* If it hasn't been updated yet, skip the render.  We can't call Update(0), since
	 * that'll confuse the "zero out the next update after loading a screen logic.
	 * If we don't render, don't call BeginFrame or EndFrame.  That way, we won't
	 * clear the buffer, and we won't wait for vsync. */
	if( m_ScreenStack.back()->IsFirstUpdate() )
		return;

	if( !DISPLAY->BeginFrame() )
		return;

	if( !m_ScreenStack.empty() && !m_ScreenStack.back()->IsTransparent() )	// top screen isn't transparent
		m_ScreenStack.back()->Draw();
	else
		for( unsigned i=0; i<m_ScreenStack.size(); i++ )	// Draw all screens bottom to top
			m_ScreenStack[i]->Draw();
	m_SystemLayer->Draw();

	DISPLAY->EndFrame();
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
	/* By default, RageSounds handles the song timer.  When we change screens, reset this;
	 * screens turn this off in SM_GainFocus if they handle timers themselves (edit). 
	 * XXX: screens should turn this on in SM_LoseFocus if they handle timers themselves, too */
	SOUND->HandleSongTimer( true );

	/* Cleanup song data.  This can free up a fair bit of memory, so do it before
	 * creating the new screen, to lower peak memory usage slightly. */
	SONGMAN->Cleanup();

	RageTimer t;
	LOG->Trace( "Loading screen %s", sClassName.c_str() );
	Screen *ret = Screen::Create( sClassName );
	LOG->Trace( "Loaded %s in %f", sClassName.c_str(), t.GetDeltaTime());

	/* Loading probably took a little while.  Let's reset stats.  This prevents us
	 * from displaying an unnaturally low FPS value, and the next FPS value we
	 * display will be accurate, which makes skips in the initial tween-ins more
	 * apparent. */
	DISPLAY->ResetStats();

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
	SetFromNewScreen( m_ScreenBuffered, false  );
	
	m_ScreenBuffered = NULL;
}

void ScreenManager::DeletePreppedScreen()
{
	SAFE_DELETE( m_ScreenBuffered );
	TEXTUREMAN->DeleteCachedTextures();
}

/* Remove all screens from the stack, sending a SM_LoseFocus message to the top. 
 * (There's no need to send them to any lower screens; they don't have focus anyway,
 * and received the message when they actually lost it. */
void ScreenManager::ClearScreenStack()
{
	if( m_ScreenStack.size() )
		m_ScreenStack.back()->HandleScreenMessage( SM_LoseFocus );

	// move current screen(s) to ScreenToDelete
	m_ScreensToDelete.insert(m_ScreensToDelete.end(), m_ScreenStack.begin(), m_ScreenStack.end());
	m_ScreenStack.clear();
}

/* Add a screen to m_ScreenStack.  If Stack is true, it's added to the stack; otherwise any
 * current screens are removed.  This is the only function that adds to m_ScreenStack. */
void ScreenManager::SetFromNewScreen( Screen *pNewScreen, bool Stack )
{
	RefreshCreditsMessages();

	if( !Stack )
		ClearScreenStack();

	m_ScreenStack.push_back( pNewScreen );
	
	PostMessageToTopScreen( SM_GainFocus, 0 );
}

void ScreenManager::SetNewScreen( CString sClassName )
{
	m_DelayedScreen = sClassName;

	/* If we're not delaying screen loads, load it now.  Otherwise, we'll load
	 * it on the next iteration.  Only delay if we already have a screen
	 * loaded; otherwise, there's no reason to delay. */
	if(!PREFSMAN->m_bDelayedScreenLoad) // || m_ScreenStack.empty() )
		LoadDelayedScreen();
}

void ScreenManager::LoadDelayedScreen()
{
retry:
	CString sClassName = m_DelayedScreen;
	m_DelayedScreen = "";

	/* If we prepped a screen but didn't use it, nuke it. */
	SAFE_DELETE( m_ScreenBuffered );

	Screen* pOldTopScreen = m_ScreenStack.empty() ? NULL : m_ScreenStack.back();

	// It makes sense that ScreenManager should allocate memory for a new screen since it 
	// deletes it later on.  This also convention will reduce includes because screens won't 
	// have to include each other's headers of other screens.
	Screen* pNewScreen = MakeNewScreen(sClassName);

	if( pOldTopScreen!=NULL  &&  m_ScreenStack.back()!=pOldTopScreen )
	{
		// While constructing this Screen, it's constructor called
		// SetNewScreen again!  That SetNewScreen Command should
		// override this older one.
		SAFE_DELETE( pNewScreen );
		return;
	}

	if( PREFSMAN->m_bDelayedScreenLoad && m_DelayedScreen != "" )
	{
		/* Same deal: the ctor called SetNewScreen again.  Delete the screen
		 * we just made, but don't delay again. */
		SAFE_DELETE( pNewScreen );
		goto retry;
	}
	
	bool bWasOnSystemMenu = GAMESTATE->m_bIsOnSystemMenu;

	/* If this is a system menu, don't let the operator key touch it! 
		However, if you add an options screen, please include it here -- Miryokuteki */
	if(	sClassName == "ScreenOptionsMenu" ||
		sClassName == "ScreenMachineOptions" || 
		sClassName == "ScreenInputOptions" || 
		sClassName == "ScreenGraphicOptions" || 
		sClassName == "ScreenGameplayOptions" || 
		sClassName == "ScreenMapControllers" || 
		sClassName == "ScreenAppearanceOptions" || 
		sClassName == "ScreenEdit" || 
		sClassName == "ScreenEditMenu" || 
		sClassName == "ScreenSoundOptions" ) 
		GAMESTATE->m_bIsOnSystemMenu = true;
	else 
		GAMESTATE->m_bIsOnSystemMenu = false;
	
	// If we're exiting a system menu, persist settings in case we don't exit normally
	if( bWasOnSystemMenu && !GAMESTATE->m_bIsOnSystemMenu )
		PREFSMAN->SaveGlobalPrefsToDisk();

	LOG->Trace("... SetFromNewScreen");
	SetFromNewScreen( pNewScreen, false );
}

void ScreenManager::AddNewScreenToTop( CString sClassName, ScreenMessage messageSendOnPop )
{	
	/* Send this before making the new screen, since it might set things that will be re-set
	 * in the new screen's ctor. */
	if( m_ScreenStack.size() )
		m_ScreenStack.back()->HandleScreenMessage( SM_LoseFocus );

	Screen* pNewScreen = MakeNewScreen(sClassName);
	SetFromNewScreen( pNewScreen, true );
	m_MessageSendOnPop = messageSendOnPop;
}

#include "ScreenPrompt.h"
#include "ScreenTextEntry.h"
#include "ScreenMiniMenu.h"

void ScreenManager::Prompt( ScreenMessage SM_SendWhenDone, CString sText, bool bYesNo, bool bDefaultAnswer, void(*OnYes)(void*), void(*OnNo)(void*), void* pCallbackData )
{
	if( m_ScreenStack.size() )
		m_ScreenStack.back()->HandleScreenMessage( SM_LoseFocus );

	// add the new state onto the back of the array
	Screen *pNewScreen = new ScreenPrompt( sText, bYesNo, bDefaultAnswer, OnYes, OnNo, pCallbackData);
	SetFromNewScreen( pNewScreen, true );

	m_MessageSendOnPop = SM_SendWhenDone;
}

void ScreenManager::TextEntry( ScreenMessage SM_SendWhenDone, CString sQuestion, CString sInitialAnswer, void(*OnOK)(CString sAnswer), void(*OnCancel)() )
{	
	if( m_ScreenStack.size() )
		m_ScreenStack.back()->HandleScreenMessage( SM_LoseFocus );

	// add the new state onto the back of the array
	Screen *pNewScreen = new ScreenTextEntry( "ScreenTextEntry", sQuestion, sInitialAnswer, OnOK, OnCancel );
	SetFromNewScreen( pNewScreen, true );

	m_MessageSendOnPop = SM_SendWhenDone;
}

void ScreenManager::MiniMenu( Menu* pDef, ScreenMessage SM_SendOnOK, ScreenMessage SM_SendOnCancel )
{
	if( m_ScreenStack.size() )
		m_ScreenStack.back()->HandleScreenMessage( SM_LoseFocus );

	// add the new state onto the back of the array
	Screen *pNewScreen = new ScreenMiniMenu( pDef, SM_SendOnOK, SM_SendOnCancel );
	SetFromNewScreen( pNewScreen, true );
}

void ScreenManager::PopTopScreen( ScreenMessage SM )
{
	Screen* pScreenToPop = m_ScreenStack.back();	// top menu
	pScreenToPop->HandleScreenMessage( SM_LoseFocus );
	m_ScreenStack.erase(m_ScreenStack.end()-1, m_ScreenStack.end());
	m_ScreensToDelete.push_back( pScreenToPop );

	/* Post to the new top.  This must be done now; otherwise, we'll have a single
	 * frame between popping and these messages, which can result in a frame where eg.
	 * input is accepted where it shouldn't be.  Watch out; sending m_MessageSendOnPop
	 * might push another screen (eg. editor menu -> PlayerOptions), which will set
	 * a new m_MessageSendOnPop. */
	ScreenMessage MessageToSend = m_MessageSendOnPop;
	m_MessageSendOnPop = SM_None;
	SendMessageToTopScreen( SM );
	SendMessageToTopScreen( SM_GainFocus );
	SendMessageToTopScreen( MessageToSend );
}

void ScreenManager::PostMessageToTopScreen( ScreenMessage SM, float fDelay )
{
	Screen* pTopScreen = m_ScreenStack.back();
	pTopScreen->PostScreenMessage( SM, fDelay );
}

void ScreenManager::SendMessageToTopScreen( ScreenMessage SM )
{
	Screen* pTopScreen = m_ScreenStack.back();
	pTopScreen->HandleScreenMessage( SM );
}


void ScreenManager::SystemMessage( CString sMessage )
{
	LOG->Trace( "%s", sMessage.c_str() );
	m_SystemLayer->SystemMessage( sMessage );
}

void ScreenManager::SystemMessageNoAnimate( CString sMessage )
{
//	LOG->Trace( "%s", sMessage.c_str() );	// don't log because the caller is likely calling us every frame
	m_SystemLayer->SystemMessageNoAnimate( sMessage );
}

void ScreenManager::RefreshCreditsMessages()
{
	m_SystemLayer->RefreshCreditsMessages();

	/* This is called when GAMESTATE->m_bSideIsJoined changes. */
	CString joined;
	FOREACH_PlayerNumber( pn )
	{
		if( GAMESTATE->m_bSideIsJoined[pn] )
		{
			if( joined != "" )
				joined += ", ";
			joined += ssprintf( "P%i", pn+1 );
		}
	}

	if( joined == "" )
		joined = "none";

	LOG->MapLog( "JOINED", "Players joined: %s", joined.c_str() );
}

void ScreenManager::ReloadCreditsText()
{
	m_SystemLayer->ReloadCreditsText();
}

/* Always play these sounds, even if we're in a silent attract loop. */
void ScreenManager::PlayStartSound()
{
	RageSoundParams p;
	p.m_Volume = PREFSMAN->m_fSoundVolume;
	m_soundStart.Play( &p );
}

void ScreenManager::PlayCoinSound()
{
	RageSoundParams p;
	p.m_Volume = PREFSMAN->m_fSoundVolume;
	m_soundCoin.Play( &p );
}

void ScreenManager::PlayInvalidSound()
{
	RageSoundParams p;
	p.m_Volume = PREFSMAN->m_fSoundVolume;
	m_soundInvalid.Play( &p );
}

void ScreenManager::PlayScreenshotSound()
{
	RageSoundParams p;
	p.m_Volume = PREFSMAN->m_fSoundVolume;
	m_soundScreenshot.Play( &p );
}

void ScreenManager::PlayBackSound()
{
	RageSoundParams p;
	p.m_Volume = PREFSMAN->m_fSoundVolume;
	m_soundBack.Play( &p );
}

/*
 * (c) 2001-2003 Chris Danford, Glenn Maynard
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

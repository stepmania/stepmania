#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenManager

 Desc: See header.

 Copyright (c) 2001-2003 by the person(s) listed below.  All rights reserved.
	Chris Danford
	Tim Hentenaar
	Glenn Maynard
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

#include "BitmapText.h"
#include "Quad.h"

ScreenManager*	SCREENMAN = NULL;	// global and accessable from anywhere in our program


#define STATS_X					THEME->GetMetricF("ScreenManager","StatsX")
#define STATS_Y					THEME->GetMetricF("ScreenManager","StatsY")
#define CREDITS_X( p )			THEME->GetMetricF("ScreenManager",ssprintf("CreditsP%dX",p+1))
#define CREDITS_Y( p )			THEME->GetMetricF("ScreenManager",ssprintf("CreditsP%dY",p+1))
#define CREDITS_COLOR			THEME->GetMetricC("ScreenManager","CreditsColor")
#define CREDITS_SHADOW_LENGTH	THEME->GetMetricF("ScreenManager","CreditsShadowLength")
#define CREDITS_ZOOM			THEME->GetMetricF("ScreenManager","CreditsZoom")

const int NUM_SKIPS = 5;

/* This screen is drawn on top of everything else, and receives updates,
 * but not input. */
class ScreenSystemLayer: public Screen
{
	BitmapText m_textStats;
	BitmapText m_textSystemMessage;
	BitmapText m_textCreditInfo[NUM_PLAYERS];
	BitmapText m_textSysTime;
	BitmapText m_Skips[NUM_SKIPS];
	int m_LastSkip;
	Quad m_SkipBackground;

	RageTimer SkipTimer;
	void UpdateTimestampAndSkips();

public:
	ScreenSystemLayer();
	void SystemMessage( CString sMessage );
	void RefreshCreditsMessages();
	void Update( float fDeltaTime );
};

ScreenSystemLayer::ScreenSystemLayer()
{
	m_textSystemMessage.LoadFromFont( THEME->GetPathTo("Fonts","normal") );
	m_textSystemMessage.SetHorizAlign( Actor::align_left );
	m_textSystemMessage.SetVertAlign( Actor::align_top );
	m_textSystemMessage.SetXY( 4.0f, 4.0f );
	m_textSystemMessage.SetZoom( 0.8f );
	m_textSystemMessage.SetShadowLength( 2 );
	m_textSystemMessage.SetDiffuse( RageColor(1,1,1,0) );
	this->AddChild(&m_textSystemMessage);

	m_textStats.LoadFromFont( THEME->GetPathTo("Fonts","normal") );
	m_textStats.SetXY( STATS_X, STATS_Y );
	m_textStats.SetHorizAlign( Actor::align_right );
	m_textStats.SetVertAlign( Actor::align_top );
	m_textStats.SetZoom( 0.5f );
	m_textStats.SetShadowLength( 2 );
	this->AddChild(&m_textStats);

	m_textSysTime.LoadFromFont( THEME->GetPathTo("Fonts","normal") );
	m_textSysTime.SetXY( 4.0f, 40.0f );
	m_textSysTime.SetHorizAlign( Actor::align_left );
	m_textSysTime.SetVertAlign( Actor::align_top );
	m_textSysTime.SetZoom( 0.5f );
	m_textSysTime.SetDiffuse( RageColor(1,0,1,1) );
	m_textSysTime.EnableShadow(false);
	this->AddChild(&m_textSysTime);

	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		m_textCreditInfo[p].LoadFromFont( THEME->GetPathTo("Fonts","credits") );
		m_textCreditInfo[p].SetXY( CREDITS_X(p), CREDITS_Y(p) );
		m_textCreditInfo[p].SetZoom( CREDITS_ZOOM );
		m_textCreditInfo[p].SetDiffuse( CREDITS_COLOR );
		m_textCreditInfo[p].SetShadowLength( CREDITS_SHADOW_LENGTH );
		this->AddChild(&m_textCreditInfo[p]);
	}

	/* "Was that a skip?"  This displays a message when an update takes
	 * abnormally long, to quantify skips more precisely, verify them
	 * when they're subtle, and show the time it happened, so you can pinpoint
	 * the time in the log.  Put a dim quad behind it to make it easier to
	 * read. */
	m_LastSkip = 0;
	const float SKIP_LEFT = 320.0f, SKIP_TOP = 60.0f, 
		SKIP_WIDTH = 160.0f, SKIP_Y_DIST = 16.0f;

	m_SkipBackground.StretchTo(RectF(SKIP_LEFT-8, SKIP_TOP-8,
						SKIP_LEFT+SKIP_WIDTH, SKIP_TOP+SKIP_Y_DIST*NUM_SKIPS));
	m_SkipBackground.SetDiffuse( RageColor(0,0,0,0) );
	this->AddChild(&m_SkipBackground);

	for( int i=0; i<NUM_SKIPS; i++ )
	{
		/* This is somewhat big.  Let's put it on the right side, where it'll
		 * obscure the 2P side during gameplay; there's nowhere to put it that
		 * doesn't obscure something, and it's just a diagnostic. */
		m_Skips[i].LoadFromFont( THEME->GetPathTo("Fonts","normal") );
		m_Skips[i].SetXY( SKIP_LEFT, SKIP_TOP + SKIP_Y_DIST*i );
		m_Skips[i].SetHorizAlign( Actor::align_left );
		m_Skips[i].SetVertAlign( Actor::align_top );
		m_Skips[i].SetZoom( 0.5f );
		m_Skips[i].SetDiffuse( RageColor(1,1,1,0) );
		m_Skips[i].EnableShadow(false);
		this->AddChild(&m_Skips[i]);
	}
}

void ScreenSystemLayer::SystemMessage( CString sMessage )
{
	m_textSystemMessage.StopTweening();
	m_textSystemMessage.SetText( sMessage );
	m_textSystemMessage.SetDiffuse( RageColor(1,1,1,1) );
	m_textSystemMessage.SetX( -640 );
	m_textSystemMessage.BeginTweening( 0.3f );
	m_textSystemMessage.SetX( 4 );
	m_textSystemMessage.BeginTweening( 5 );
	m_textSystemMessage.BeginTweening( 0.3f );
	m_textSystemMessage.SetDiffuse( RageColor(1,1,1,0) );
}

void ScreenSystemLayer::RefreshCreditsMessages()
{
	// update joined
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
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
				CString txt = ssprintf("CREDIT(S) %d", GAMESTATE->m_iCoins / PREFSMAN->m_iCoinsPerCredit);
				if (Coins)
					txt += ssprintf("  %d/%d", Coins, PREFSMAN->m_iCoinsPerCredit );
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

void ScreenSystemLayer::UpdateTimestampAndSkips()
{
	if(!PREFSMAN->m_bTimestamping)
	{
		/* Hide: */
		m_textSysTime.SetDiffuse( RageColor(1,1,1,0) );
		m_SkipBackground.SetDiffuse( RageColor(0,0,0,0) );
		return;
	}

	m_SkipBackground.SetDiffuse(RageColor(0,0,0,0.4f));

	CString time(SecondsToTime(RageTimer::GetTimeSinceStart()));

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
			static const RageColor colors[] = {
				RageColor(0,0,0,0),		  /* unused */
				RageColor(0.2f,0.2f,1,1), /* light blue */
				RageColor(1,1,0,1),		  /* yellow */
				RageColor(1,0.2f,0.2f,1)  /* light red */
			};
			m_Skips[m_LastSkip].SetText(ssprintf("%s: %.0fms (%.0f)",
				time.GetString(), 1000*UpdateTime, UpdateTime/ExpectedUpdate));
			m_Skips[m_LastSkip].StopTweening();
			m_Skips[m_LastSkip].SetDiffuse(RageColor(1,1,1,1));
			m_Skips[m_LastSkip].BeginTweening(0.2f);
			m_Skips[m_LastSkip].SetDiffuse(colors[skip]);
			m_Skips[m_LastSkip].BeginTweening(3.0f);
			m_Skips[m_LastSkip].BeginTweening(0.2f);
			m_Skips[m_LastSkip].SetDiffuse(RageColor(1,1,1,0));

			m_LastSkip++;
			m_LastSkip %= NUM_SKIPS;
		}
	}

	m_textSysTime.SetText( time );
	m_textSysTime.SetDiffuse( RageColor(1,0,1,1) );
}

void ScreenSystemLayer::Update( float fDeltaTime )
{
	Screen::Update(fDeltaTime);

	if( PREFSMAN  &&  PREFSMAN->m_bShowStats )
	{
		m_textStats.SetDiffuse( RageColor(1,1,1,1) );

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
	m_SystemLayer = new ScreenSystemLayer;

	m_ScreenBuffered = NULL;
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

void ScreenManager::EmptyDeleteQueue()
{
	// delete all ScreensToDelete
	for( unsigned i=0; i<m_ScreensToDelete.size(); i++ )
		SAFE_DELETE( m_ScreensToDelete[i] );

	m_ScreensToDelete.clear();
}

/* XXX: Big hack:
 * All screens must receive at least one update before they draw.
 * However, no screen should ever receive an update until it's ready
 * to be drawn; otherwise Gameplay will queue music to start even though
 * Stage might still be delaying for a while, throwing off timing.
 *
 * This is tricky with prepped screens.  We can't do an Update(0) in 
 * LoadPreppedScreen, since that'll cause the delete queue to be cleared
 * while the screen is on the stack, and possibly other odd things.
 * So, update before we draw, which is also a hack (rendering shouldn't
 * change state!).  Only do this when we have to, so we don't double
 * the number of updates. */
static bool g_TopNeedsNeedsNullUpdate = false;

void ScreenManager::Update( float fDeltaTime )
{
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
	ASSERT( !m_ScreenStack.empty() );	// Why play the game if there is nothing showing?
	Screen* pScreen = m_ScreenStack[m_ScreenStack.size()-1];
	if( pScreen->IsFirstUpdate() )
		pScreen->Update( 0 );
	else
		pScreen->Update( fDeltaTime );

	m_SystemLayer->Update( fDeltaTime );
}


void ScreenManager::Draw()
{
	if(g_TopNeedsNeedsNullUpdate)
	{
		g_TopNeedsNeedsNullUpdate = false;
		m_ScreenStack.back()->Update( 0 );
	}

	if( !m_ScreenStack.empty() && !m_ScreenStack.back()->IsTransparent() )	// top screen isn't transparent
		m_ScreenStack.back()->Draw();
	else
		for( unsigned i=0; i<m_ScreenStack.size(); i++ )	// Draw all screens bottom to top
			m_ScreenStack[i]->Draw();
	m_SystemLayer->Draw();
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
	
	// Need to update the new screen once, or else it will be 
	// drawn before ever being Update()d.
	g_TopNeedsNeedsNullUpdate = true;
	
	m_ScreenBuffered = NULL;
}

void ScreenManager::DeletePreppedScreen()
{
	SAFE_DELETE( m_ScreenBuffered );
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
	
	Screen* pOldTopScreen = m_ScreenStack.empty() ? NULL : m_ScreenStack.back();

	// It makes sense that ScreenManager should allocate memory for a new screen since it 
	// deletes it later on.  This also convention will reduce includes because screens won't 
	// have to include each other's headers of other screens.
	Screen* pNewScreen = MakeNewScreen(sClassName);
	LOG->Trace( "Loaded %s in %f", sClassName.GetString(), t.GetDeltaTime());

	if( pOldTopScreen!=NULL  &&  m_ScreenStack.back()!=pOldTopScreen )
	{
		// While constructing this Screen, it's constructor called
		// SetNewScreen again!  That SetNewScreen Command should
		// override this older one.
		SAFE_DELETE( pNewScreen );
		return;
	}

	SetNewScreen( pNewScreen );

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

void ScreenManager::MiniMenu( Menu* pDef, ScreenMessage SM_SendOnOK, ScreenMessage SM_SendOnCancel )
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
	m_SystemLayer->SystemMessage( sMessage );
}

void ScreenManager::RefreshCreditsMessages()
{
	m_SystemLayer->RefreshCreditsMessages();
}


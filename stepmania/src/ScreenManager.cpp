/*
 * We maintain a pool of "prepared" screens, which are screens previously loaded
 * to be available on demand.
 *
 * When a screen pops off the stack that's in g_setPersistantScreens, it goes to
 * the prepared list.  If that screen is used again before it's deleted, it'll be
 * reused.
 *
 * Typically, the first screen in a group will prepare all of the nearby screens it
 * may load.  When it loads one, the prepared screen will be used.
 *
 * A group of screens may only want to preload some screens, and not others,
 * while still considering those screens part of the same group.  For example,
 * an attract loop typically has several very lightweight screens and a few
 * expensive screens.  Preloading the lightweight screens can improve responsiveness,
 * but preloading the expensive screens may use too much memory and take too long
 * to load all at once.  By calling GroupScreen(), entering these screens will not
 * trigger cleanup.
 * 
 * Example uses:
 *  - ScreenOptions1 preloads ScreenOptions2, and persists both.  Moving from Options1
 *    and Options2 and back is instant and reuses both.
 *  - ScreenMenu groups and persists itself and ScreenSubmenu.  ScreenSubmenu
 *    is not preloaded, so it will be loaded on demand the first time it's used,
 *    but will remain loaded if it returns to ScreenMenu.
 *  - ScreenSelectMusic groups itself and ScreenPlayerOptions, and persists only
 *    ScreenSelectMusic.  This will cause SSMusic will be loaded once, and SPO to
 *    be loaded on demand.
 *  - ScreenAttract1 preloads and persists ScreenAttract1, 3, 5 and 7, and groups 1
 *    through 7.  1, 3, 5 and 7 will remain in memory; the rest will be loaded on
 *    demand.
 * 
 * If a screen is added to the screen stack that isn't in the current screen group
 * (added to by GroupScreen), the screen group is reset: all prepared screens are
 * unloaded and the persistance list is cleared.
 *
 * (For persistance, concurrently preparing a screen is logically equivalent
 * to SetNewScreen, since it's going to be set after it finishes loading.)
 *
 * Note that not all screens yet support reuse in this way; proper use of BeginScreen
 * is required.  This will misbehave if a screen pushes another screen that's already
 * in use lower on the stack, but that's not useful; it would allow infinite screen
 * recursion.
 *
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
#include "Preference.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "GameSoundManager.h"
#include "RageDisplay.h"
#include "SongManager.h"
#include "RageTextureManager.h"
#include "ThemeManager.h"
#include "Screen.h"
#include "ScreenDimensions.h"
#include "Foreach.h"
#include "ActorUtil.h"
#include "GameLoop.h"

ScreenManager*	SCREENMAN = NULL;	// global and accessable from anywhere in our program

static Preference<bool> g_bConcurrentLoading( "ConcurrentLoading", false );
static Preference<bool> g_bDelayedScreenLoad( "DelayedScreenLoad", false );

// Screen registration
static map<RString,CreateScreenFn>	*g_pmapRegistrees = NULL;

namespace ScreenManagerUtil
{
	//
	// in draw order first to last
	//
	struct LoadedScreen
	{
		Screen *m_pScreen;
	
		/* Normally true.  If false, the screen is owned by another screen
		 * and was given to us for use, and it's not ours to free. */
		bool m_bDeleteWhenDone;

		ScreenMessage m_SendOnPop;

		LoadedScreen()
		{
			m_pScreen = NULL;
			m_bDeleteWhenDone = true;
			m_SendOnPop = SM_None;
		}
	};

	Actor                   *g_pSharedBGA;  // BGA object that's persistent between screens
	vector<LoadedScreen>    g_ScreenStack;  // bottommost to topmost
	vector<Screen*>         g_OverlayScreens;
	set<RString>		g_setGroupedScreens;
	set<RString>		g_setPersistantScreens;

	vector<LoadedScreen>    g_vPreparedScreens;
	vector<Actor*>          g_vPreparedBackgrounds;

	/* Add a screen to g_ScreenStack.  This is the only function that adds to g_ScreenStack. */
	void PushLoadedScreen( const LoadedScreen &ls )
	{
		// Be sure to push the screen first, so GetTopScreen returns the screen
		// during BeginScreen.
		g_ScreenStack.push_back( ls );

		/* Set the name of the loading screen. */
		ActorUtil::ActorParam LoadingScreen( "LoadingScreen", ls.m_pScreen->GetName() );
		ls.m_pScreen->BeginScreen();

		LoadingScreen.Release();

		SCREENMAN->RefreshCreditsMessages();

		SCREENMAN->PostMessageToTopScreen( SM_GainFocus, 0 );
	}

	bool ScreenIsPrepped( const RString &sScreenName )
	{
		FOREACH( LoadedScreen, g_vPreparedScreens, s )
		{
			if( s->m_pScreen->GetName() == sScreenName )
			return true;
		}
		return false;
	}

	/* If the named screen is loaded, remove it from the prepared list and
	 * return it in ls. */
	bool GetPreppedScreen( const RString &sScreenName, LoadedScreen &ls )
	{
		FOREACH( LoadedScreen, g_vPreparedScreens, s )
		{
			if( s->m_pScreen->GetName() == sScreenName )
			{
				ls = *s;
				g_vPreparedScreens.erase( s );
				return true;
			}
		}
		return false;
	}

	void BeforeDeleteScreen()
	{
		/* Deleting a screen can take enough time to cause a frame skip. */
		SCREENMAN->ZeroNextUpdate();
	}

	/* If we're deleting a screen, it's probably releasing texture and other
	 * resources, so trigger cleanups. */
	void AfterDeleteScreen()
	{
		/* Now that we've actually deleted a screen, it makes sense to clear out
		 * cached textures. */
		TEXTUREMAN->DeleteCachedTextures();

		/* Cleanup song data.  This can free up a fair bit of memory, so do it after
		 * deleting screens. */
		SONGMAN->Cleanup();
	}

	/* Take ownership of all screens and backgrounds that are owned by
	 * us (this excludes screens where m_bDeleteWhenDone is false).
	 * Clear the prepared lists.  The contents of apOut must be
	 * freed by the caller. */
	void GrabPreparedActors( vector<Actor*> &apOut )
	{
		FOREACH( LoadedScreen, g_vPreparedScreens, s )
			if( s->m_bDeleteWhenDone )
				apOut.push_back( s->m_pScreen );
		g_vPreparedScreens.clear();
		FOREACH( Actor*, g_vPreparedBackgrounds, a )
			apOut.push_back( *a );
		g_vPreparedBackgrounds.clear();

		g_setGroupedScreens.clear();
		g_setPersistantScreens.clear();
	}
	
	/* Called when changing screen groups.  Delete all prepared screens,
	 * reset the screen group and list of persistant screens. */
	void DeletePreparedScreens()
	{
		vector<Actor*> apActorsToDelete;
		GrabPreparedActors( apActorsToDelete );

		BeforeDeleteScreen();
		FOREACH( Actor*, apActorsToDelete, a )
			SAFE_DELETE( *a );
		AfterDeleteScreen();
	}
};
using namespace ScreenManagerUtil;

void RegisterScreenClass( const RString& sClassName, CreateScreenFn pfn )
{
	if( g_pmapRegistrees == NULL )
		g_pmapRegistrees = new map<RString,CreateScreenFn>;

	map<RString,CreateScreenFn>::iterator iter = g_pmapRegistrees->find( sClassName );
	ASSERT_M( iter == g_pmapRegistrees->end(), ssprintf("Screen class '%s' already registered.", sClassName.c_str()) );

	(*g_pmapRegistrees)[sClassName] = pfn;
}


ScreenManager::ScreenManager()
{
	// Register with Lua.
	{
		Lua *L = LUA->Get();
		lua_pushstring( L, "SCREENMAN" );
		this->PushSelf( L );
		lua_settable( L, LUA_GLOBALSINDEX );
		LUA->Release( L );
	}

	g_pSharedBGA = new Actor;

	m_bZeroNextUpdate = false;
	m_PopTopScreen = SM_Invalid;
	m_OnDonePreparingScreen = SM_Invalid;
}


ScreenManager::~ScreenManager()
{
	LOG->Trace( "ScreenManager::~ScreenManager()" );

	SAFE_DELETE( g_pSharedBGA );
	for( unsigned i=0; i<g_ScreenStack.size(); i++ )
	{
		if( g_ScreenStack[i].m_bDeleteWhenDone )
			SAFE_DELETE( g_ScreenStack[i].m_pScreen );
	}
	g_ScreenStack.clear();
	DeletePreparedScreens();
	for( unsigned i=0; i<g_OverlayScreens.size(); i++ )
		SAFE_DELETE( g_OverlayScreens[i] );
	g_OverlayScreens.clear();

	// Unregister with Lua.
	LUA->UnsetGlobal( "SCREENMAN" );
}

/* This is called when we start up, and when the theme changes or is reloaded. */
void ScreenManager::ThemeChanged()
{
	LOG->Trace( "ScreenManager::ThemeChanged" );

	// reload common sounds
	m_soundStart.Load( THEME->GetPathS("Common","start") );
	m_soundCoin.Load( THEME->GetPathS("Common","coin"), true );
	m_soundCancel.Load( THEME->GetPathS("Common","cancel"), true );
	m_soundInvalid.Load( THEME->GetPathS("Common","invalid") );
	m_soundScreenshot.Load( THEME->GetPathS("Common","screenshot") );

	// unload overlay screens
	for( unsigned i=0; i<g_OverlayScreens.size(); i++ )
		SAFE_DELETE( g_OverlayScreens[i] );
	g_OverlayScreens.clear();

	// reload overlay screens
	RString sOverlays = THEME->GetMetric( "Common","OverlayScreens" );
	vector<RString> asOverlays;
	split( sOverlays, ",", asOverlays );
	for( unsigned i=0; i<asOverlays.size(); i++ )
	{
		Screen *pScreen = MakeNewScreen( asOverlays[i] );
		g_OverlayScreens.push_back( pScreen );
	}
	
	// force recreate of new BGA
	SAFE_DELETE( g_pSharedBGA );
	g_pSharedBGA = new Actor;

	this->RefreshCreditsMessages();
}

Screen *ScreenManager::GetTopScreen()
{
	if( g_ScreenStack.empty() )
		return NULL;
	return g_ScreenStack[g_ScreenStack.size()-1].m_pScreen;
}

Screen *ScreenManager::GetScreen( int iPosition )
{
	if( iPosition >= (int) g_ScreenStack.size() )
		return NULL;
	return g_ScreenStack[iPosition].m_pScreen;
}

bool ScreenManager::AllowOperatorMenuButton() const
{
	FOREACH( LoadedScreen, g_ScreenStack, s )
	{
		if( !s->m_pScreen->AllowOperatorMenuButton() )
			return false;
	}

	return true;
}

bool ScreenManager::IsStackedScreen( const Screen *pScreen ) const
{
	/* True if the screen is in the screen stack, but not the first. */
	for( unsigned i = 1; i < g_ScreenStack.size(); ++i )
		if( g_ScreenStack[i].m_pScreen == pScreen )
			return true;
	return false;
}

static bool g_bIsConcurrentlyLoading = false;

/* Pop the top screen off the stack, sending SM_LoseFocus messages and
 * returning the message the popped screen wants sent to the new top
 * screen.  Does not send SM_GainFocus. */
ScreenMessage ScreenManager::PopTopScreenInternal( bool bSendLoseFocus )
{
	if( g_ScreenStack.empty() )
		return SM_None;

	LoadedScreen ls = g_ScreenStack.back();
	g_ScreenStack.erase( g_ScreenStack.end()-1, g_ScreenStack.end() );

	if( bSendLoseFocus )
		ls.m_pScreen->HandleScreenMessage( SM_LoseFocus );
	ls.m_pScreen->EndScreen();

	if( g_setPersistantScreens.find(ls.m_pScreen->GetName()) != g_setPersistantScreens.end() )
	{
		/* Move the screen back to the prepared list. */
		g_vPreparedScreens.push_back( ls );
	}
	else
	{
		if( ls.m_bDeleteWhenDone )
		{
			BeforeDeleteScreen();
			SAFE_DELETE( ls.m_pScreen );
			AfterDeleteScreen();
		}
	}

	return ls.m_SendOnPop;
}

void ScreenManager::Update( float fDeltaTime )
{
	//
	// Pop the top screen, if PopTopScreen was called.
	//
	if( m_PopTopScreen != SM_Invalid )
	{
		ScreenMessage SM = m_PopTopScreen;
		m_PopTopScreen = SM_Invalid;

		ScreenMessage SM2 = PopTopScreenInternal();

		SendMessageToTopScreen( SM_GainFocus );
		SendMessageToTopScreen( SM );
		SendMessageToTopScreen( SM2 );
	}

	/*
	 * Screens take some time to load.  If we don't do this, then screens
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
	ASSERT( !g_ScreenStack.empty() || m_sDelayedScreen != "" );	// Why play the game if there is nothing showing?

	Screen* pScreen = g_ScreenStack.empty() ? NULL : GetTopScreen();

	bool bFirstUpdate = pScreen && pScreen->IsFirstUpdate();

	/* Loading a new screen can take seconds and cause a big jump on the new 
	 * Screen's first update.  Clamp the first update delta so that the 
	 * animations don't jump. */
	if( pScreen && m_bZeroNextUpdate )
	{
		LOG->Trace( "Zeroing this update.  Was %f", fDeltaTime );
		fDeltaTime = 0;
		m_bZeroNextUpdate = false;
	}

	//
	// Update screens.
	//
	{
		for( unsigned i=0; i<g_ScreenStack.size(); i++ )
			g_ScreenStack[i].m_pScreen->Update( fDeltaTime );

		g_pSharedBGA->Update( fDeltaTime );

		for( unsigned i=0; i<g_OverlayScreens.size(); i++ )
			g_OverlayScreens[i]->Update( fDeltaTime );	
	}
	//
	// Handle messages after updating.
	//
	{
		for( unsigned i=0; i<g_ScreenStack.size(); i++ )
			g_ScreenStack[i].m_pScreen->ProcessMessages( fDeltaTime );

		g_pSharedBGA->ProcessMessages( fDeltaTime );

		for( unsigned i=0; i<g_OverlayScreens.size(); i++ )
			g_OverlayScreens[i]->ProcessMessages( fDeltaTime );
	}

	/* The music may be started on the first update.  If we're reading from a CD,
	 * it might not start immediately.  Make sure we start playing the sound before
	 * continuing, since it's strange to start rendering before the music starts. */
	if( bFirstUpdate )
		SOUND->Flush();

	/* If we're currently inside a background screen load, and m_sDelayedScreen
	 * is set, then the screen called SetNewScreen before we finished preparing.
	 * Postpone it until we're finished loading. */
	if( !IsConcurrentlyLoading() && m_sDelayedScreen.size() != 0 )
	{
		LoadDelayedScreen();
	}

	if( !m_sDelayedConcurrentPrepare.empty() )
	{
		RunConcurrentlyPrepareScreen();
	}
}

bool ScreenManager::IsConcurrentlyLoading() const
{
	return g_bIsConcurrentlyLoading;
}

void ScreenManager::Draw()
{
	/* If it hasn't been updated yet, skip the render.  We can't call Update(0), since
	 * that'll confuse the "zero out the next update after loading a screen logic.
	 * If we don't render, don't call BeginFrame or EndFrame.  That way, we won't
	 * clear the buffer, and we won't wait for vsync. */
	if( g_ScreenStack.size() && g_ScreenStack.back().m_pScreen->IsFirstUpdate() )
		return;

	if( !DISPLAY->BeginFrame() )
		return;

	DISPLAY->CameraPushMatrix();
	DISPLAY->LoadMenuPerspective( 0, SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_CENTER_X, SCREEN_CENTER_Y );
	g_pSharedBGA->Draw();
	DISPLAY->CameraPopMatrix();

	for( unsigned i=0; i<g_ScreenStack.size(); i++ )	// Draw all screens bottom to top
		g_ScreenStack[i].m_pScreen->Draw();

	for( unsigned i=0; i<g_OverlayScreens.size(); i++ )
		g_OverlayScreens[i]->Draw();


	DISPLAY->EndFrame();
}


void ScreenManager::Input( const InputEventPlus &input )
{
//	LOG->Trace( "ScreenManager::Input( %d-%d, %d-%d, %d-%d, %d-%d )", 
//		DeviceI.device, DeviceI.button, GameI.controller, GameI.button, MenuI.player, MenuI.button, StyleI.player, StyleI.col );

	// First, give overlay screens a shot at the input.  If OverlayInput returns
	// true, it handled the input, so don't pass it further.  OverlayInput could
	// probably be merged with Input, but that would require changing all Input
	// overloads, as well as all MenuLeft, etc. overloads.
	for( unsigned i = 0; i < g_OverlayScreens.size(); ++i )
	{
		Screen *pScreen = g_OverlayScreens[i];
		if( pScreen->OverlayInput(input) )
			return;
	}

	// Pass input to the topmost screen.  If we have a new top screen pending, don't
	// send to the old screen, but do send to overlay screens.
	if( m_sDelayedScreen != "" )
		return;

	if( g_ScreenStack.empty() )
		return;

	g_ScreenStack.back().m_pScreen->Input( input );
}

/* Just create a new screen; don't do any associated cleanup. */
Screen* ScreenManager::MakeNewScreen( const RString &sScreenName )
{
	RageTimer t;
	LOG->Trace( "Loading screen name '%s'", sScreenName.c_str() );

	RString sClassName = THEME->GetMetric( sScreenName,"Class" );
	
	map<RString,CreateScreenFn>::iterator iter = g_pmapRegistrees->find( sClassName );
	if( iter == g_pmapRegistrees->end() )
		RageException::Throw( "Screen \"%s\" has an invalid class \"%s\".", sScreenName.c_str(), sClassName.c_str() );

	this->ZeroNextUpdate();

	CreateScreenFn pfn = iter->second;
	Screen *ret = pfn( sScreenName );

	LOG->Trace( "Loaded '%s' ('%s') in %f", sScreenName.c_str(), sClassName.c_str(), t.GetDeltaTime() );

	return ret;
}

void ScreenManager::PrepareScreen( const RString &sScreenName )
{
	// If the screen is already prepared, stop.
	if( ScreenIsPrepped(sScreenName) )
		return;

	Screen* pNewScreen = MakeNewScreen(sScreenName);

	{
		LoadedScreen ls;
		ls.m_pScreen = pNewScreen;

		g_vPreparedScreens.push_back( ls );
	}

	/* Don't delete previously prepared versions of the screen's background,
	 * and only prepare it if it's different than the current background
	 * and not already loaded. */
	RString sNewBGA = THEME->GetPathB(sScreenName,"background");

	if( !sNewBGA.empty() && sNewBGA != g_pSharedBGA->GetName() )
	{
		Actor *pNewBGA = NULL;
		FOREACH( Actor*, g_vPreparedBackgrounds, a )
		{
			if( (*a)->GetName() == sNewBGA )
			{
				pNewBGA = *a;
				break;
			}
		}

		// Create the new background before deleting the previous so that we keep
		// any common textures loaded.
		if( pNewBGA == NULL )
		{
			LOG->Trace( "Loading screen background \"%s\"", sNewBGA.c_str() );
			Actor *pActor = ActorUtil::MakeActor( sNewBGA );
			pActor->SetName( sNewBGA );
			g_vPreparedBackgrounds.push_back( pActor );
		}
	}

	TEXTUREMAN->DiagnosticOutput();
}

void ScreenManager::GroupScreen( const RString &sScreenName )
{
	g_setGroupedScreens.insert( sScreenName );
}

void ScreenManager::PersistantScreen( const RString &sScreenName )
{
	g_setPersistantScreens.insert( sScreenName );
}

bool ScreenManager::ConcurrentlyPrepareScreen( const RString &sScreenName, ScreenMessage SM )
{
	ASSERT_M( m_sDelayedConcurrentPrepare == "", m_sDelayedConcurrentPrepare );

	if( !g_bConcurrentLoading || !DISPLAY->SupportsThreadedRendering() )
		return false;

	LOG->Trace( "ConcurrentlyPrepareScreen(%s)", sScreenName.c_str() );
	ASSERT( !IsConcurrentlyLoading() );

	m_sDelayedConcurrentPrepare = sScreenName;
	m_OnDonePreparingScreen = SM;
	return true;
}

void ScreenManager::RunConcurrentlyPrepareScreen()
{
	/* Don't call BackgroundPrepareScreen() from within another background load. */
	ASSERT( !IsConcurrentlyLoading() );

	RString sScreenName = m_sDelayedConcurrentPrepare;
	m_sDelayedConcurrentPrepare = "";

	ScreenMessage SM = m_OnDonePreparingScreen;
	m_OnDonePreparingScreen = SM_None;

	/* If the screen is already prepared, we're all set. */
	if( !ScreenIsPrepped(sScreenName) )
	{
		g_bIsConcurrentlyLoading = true;
		GameLoop::StartConcurrentRendering();

		if( g_setGroupedScreens.find(sScreenName) == g_setGroupedScreens.end() )
			DeletePreparedScreens();
		
		PrepareScreen( sScreenName );
		
		GameLoop::FinishConcurrentRendering();
		g_bIsConcurrentlyLoading = false;

		LOG->Trace( "Concurrent prepare of %s finished", sScreenName.c_str() );
	}

	/* We're done.  Send the message.  The screen is allowed to start
	 * another concurrent prepare from this message. */
	SendMessageToTopScreen( SM );
}

void ScreenManager::SetNewScreen( const RString &sScreenName )
{
	ASSERT( sScreenName != "" );
	m_sDelayedScreen = sScreenName;
}

/* Activate the screen and/or its background, if either are loaded.  Return true if both were
 * activated. */
bool ScreenManager::ActivatePreparedScreenAndBackground( const RString &sScreenName )
{
	bool bLoadedBoth = true;

	//
	// Find the prepped screen.
	//
	if( GetTopScreen() == NULL || GetTopScreen()->GetName() != sScreenName )
	{
		LoadedScreen ls;
		if( !GetPreppedScreen(sScreenName, ls) )
		{
			bLoadedBoth = false;
		}
		else
		{
			LOG->Trace("... PushScreen");
			PushLoadedScreen( ls );
		}
	}

	// Find the prepared shared background (if any), and activate it.
	RString sNewBGA = THEME->GetPathB(sScreenName,"background");
	if( sNewBGA != g_pSharedBGA->GetName() )
	{
		Actor *pNewBGA = NULL;
		if( sNewBGA.empty() )
		{
			pNewBGA = new Actor;
		}
		else
		{
			FOREACH( Actor*, g_vPreparedBackgrounds, a )
			{
				if( (*a)->GetName() == sNewBGA )
				{
					pNewBGA = *a;
					g_vPreparedBackgrounds.erase( a );
					break;
				}
			}
		}

		/* If the BGA isn't loaded yet, load a dummy actor.  If we're not going to use the same
		 * BGA for the new screen, always move the old BGA back to g_vPreparedBackgrounds now. */
		if( pNewBGA == NULL )
		{
			bLoadedBoth = false;
			pNewBGA = new Actor;
		}

		/* Move the old background back to the prepared list, or delete it if
		 * it's a blank actor. */
		if( g_pSharedBGA->GetName() == "" )
			delete g_pSharedBGA;
		else
			g_vPreparedBackgrounds.push_back( g_pSharedBGA );
		g_pSharedBGA = pNewBGA;
		g_pSharedBGA->PlayCommand( "On" );
	}

	return bLoadedBoth;
}

void ScreenManager::LoadDelayedScreen()
{
	RString sScreenName = m_sDelayedScreen;
	m_sDelayedScreen = "";

	// Pop the top screen, if any.
	ScreenMessage SM = PopTopScreenInternal();

	/* If the screen is already prepared, activate it before performing any cleanup, so
	 * it doesn't get deleted by cleanup. */
	bool bLoaded = ActivatePreparedScreenAndBackground( sScreenName );

	vector<Actor*> apActorsToDelete;
	if( g_setGroupedScreens.find(sScreenName) == g_setGroupedScreens.end() )
	{
		/* It's time to delete all old prepared screens.  Depending on DelayedScreenLoad,
		 * we can either delete the screens before or after we load the new screen.  Either
		 * way, we must remove them from the prepared list before we prepare new screens.
		 *
		 * If DelayedScreenLoad is true, delete them now; this lowers memory requirements,
		 * but results in redundant loads as we unload common data. */
		if( g_bDelayedScreenLoad )
			DeletePreparedScreens();
		else
			GrabPreparedActors( apActorsToDelete );
	}

	/* If the screen wasn't already prepared, load it. */
	if( !bLoaded )
	{
		PrepareScreen( sScreenName );

		// Screens may not call SetNewScreen from the ctor or Init().  (We don't do this
		// check inside PrepareScreen; that may be called from a thread for concurrent
		// loading, and the main thread may call SetNewScreen during that time.)
		ASSERT_M( m_sDelayedScreen.empty(), m_sDelayedScreen );

		bLoaded = ActivatePreparedScreenAndBackground( sScreenName );
		ASSERT( bLoaded );
	}

	if( !apActorsToDelete.empty() )
	{
		BeforeDeleteScreen();
		FOREACH( Actor*, apActorsToDelete, a )
			SAFE_DELETE( *a );
		AfterDeleteScreen();
	}

	MESSAGEMAN->Broadcast( Message_ScreenChanged );

	SendMessageToTopScreen( SM );
}

void ScreenManager::AddNewScreenToTop( const RString &sScreenName, ScreenMessage SendOnPop )
{
	// Load the screen, if it's not already prepared.
	PrepareScreen( sScreenName );

	// Find the prepped screen.
	LoadedScreen ls;
	bool b = GetPreppedScreen( sScreenName, ls );
	ASSERT( b );

	ls.m_SendOnPop = SendOnPop;

	if( g_ScreenStack.size() )
		g_ScreenStack.back().m_pScreen->HandleScreenMessage( SM_LoseFocus );
	PushLoadedScreen( ls );
}

void ScreenManager::PopTopScreen( ScreenMessage SM )
{
	ASSERT( g_ScreenStack.size() > 0 );

	m_PopTopScreen = SM;
}

/* Clear the screen stack; only used before major, unusual state changes,
 * such as resetting the game or jumping to the service menu.  Don't call
 * from inside a screen. */
void ScreenManager::PopAllScreens()
{
	/* Make sure only the top screen receives LoseFocus. */
	bool bFirst = true;
	while( !g_ScreenStack.empty() )
	{
		PopTopScreenInternal( bFirst );
		bFirst = false;
	}

	DeletePreparedScreens();
}

void ScreenManager::PostMessageToTopScreen( ScreenMessage SM, float fDelay )
{
	Screen* pTopScreen = GetTopScreen();
	if( pTopScreen != NULL )
		pTopScreen->PostScreenMessage( SM, fDelay );
}

void ScreenManager::SendMessageToTopScreen( ScreenMessage SM )
{
	Screen* pTopScreen = GetTopScreen();
	if( pTopScreen != NULL )
		pTopScreen->HandleScreenMessage( SM );
}


void ScreenManager::SystemMessage( const RString &sMessage )
{
	m_sSystemMessage = sMessage;
	LOG->Trace( "%s", sMessage.c_str() );
	MESSAGEMAN->Broadcast( "SystemMessage" );
}

void ScreenManager::SystemMessageNoAnimate( const RString &sMessage )
{
//	LOG->Trace( "%s", sMessage.c_str() );	// don't log because the caller is likely calling us every frame
	m_sSystemMessage = sMessage;
	MESSAGEMAN->Broadcast( "SystemMessageNoAnimate" );
}

void ScreenManager::HideSystemMessage()
{
	MESSAGEMAN->Broadcast( "HideSystemMessage" );
}


void ScreenManager::RefreshCreditsMessages()
{
	MESSAGEMAN->Broadcast( "RefreshCreditText" );

	/* This is called when GAMESTATE->m_bSideIsJoined changes. */
	RString joined;
	FOREACH_HumanPlayer( pn )
	{
		if( joined != "" )
			joined += ", ";
		joined += ssprintf( "P%i", pn+1 );
	}

	if( joined == "" )
		joined = "none";

	LOG->MapLog( "JOINED", "Players joined: %s", joined.c_str() );
}

void ScreenManager::ZeroNextUpdate()
{
	if( !IsConcurrentlyLoading() )
	{
		LOG->Trace("ScreenManager::ZeroNextUpdate");
		m_bZeroNextUpdate = true;

		/* Loading probably took a little while.  Let's reset stats.  This prevents us
		 * from displaying an unnaturally low FPS value, and the next FPS value we
		 * display will be accurate, which makes skips in the initial tween-ins more
		 * apparent. */
		DISPLAY->ResetStats();
	}
}

void ScreenManager::PlayInvalidSound()
{
	RageSoundParams p;
	p.m_bIsCriticalSound = true;
	m_soundInvalid.Play( &p );
}

/* Always play these sounds, even if we're in a silent attract loop. */
void ScreenManager::PlayStartSound()
{
	RageSoundParams p;
	p.m_bIsCriticalSound = true;
	m_soundStart.Play( &p );
}

void ScreenManager::PlayCoinSound()
{
	RageSoundParams p;
	p.m_bIsCriticalSound = true;
	m_soundCoin.Play( &p );
}

void ScreenManager::PlayCancelSound()
{
	RageSoundParams p;
	p.m_bIsCriticalSound = true;
	m_soundCancel.Play( &p );
}

void ScreenManager::PlayScreenshotSound()
{
	RageSoundParams p;
	p.m_bIsCriticalSound = true;
	m_soundScreenshot.Play( &p );
}

void ScreenManager::PlaySharedBackgroundOffCommand()
{
	g_pSharedBGA->PlayCommand("Off");
}

// lua start
#include "LuaBinding.h"

class LunaScreenManager: public Luna<ScreenManager>
{
public:
	// Note: PrepareScreen binding is not allowed; loading data inside
	// Lua causes the Lua lock to be held for the duration of the load,
	// which blocks concurrent rendering
	static int SetNewScreen( T* p, lua_State *L )		{ p->SetNewScreen( SArg(1) ); return 0; }
	static int GetTopScreen( T* p, lua_State *L )
	{
		Actor *pScreen = p->GetTopScreen();
		if( pScreen != NULL )
			pScreen->PushSelf(L);
		else
			lua_pushnil( L );
		return 1;
	}
	static int SystemMessage( T* p, lua_State *L )		{ p->SystemMessage( SArg(1) ); return 0; }
	static int ConcurrentlyPrepareScreen( T* p, lua_State *L )	{ p->ConcurrentlyPrepareScreen( SArg(1) ); return 0; }
	static int ScreenIsPrepped( T* p, lua_State *L )	{ lua_pushboolean( L, ScreenManagerUtil::ScreenIsPrepped( SArg(1) ) ); return 1; }

	LunaScreenManager()
	{
		LUA->Register( Register );

		ADD_METHOD( SetNewScreen );
		ADD_METHOD( GetTopScreen );
		ADD_METHOD( SystemMessage );
		ADD_METHOD( ConcurrentlyPrepareScreen );
		ADD_METHOD( ScreenIsPrepped );
	}
};

LUA_REGISTER_CLASS( ScreenManager )
// lua end

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

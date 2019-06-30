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
 * unloaded and the persistence list is cleared.
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
#include "FontManager.h"
#include "Screen.h"
#include "ScreenDimensions.h"
#include "ActorUtil.h"
#include "InputEventPlus.h"

ScreenManager*	SCREENMAN = nullptr;	// global and accessible from anywhere in our program

static Preference<bool> g_bDelayedScreenLoad( "DelayedScreenLoad", false );
//static Preference<bool> g_bPruneFonts( "PruneFonts", true );

// Screen registration
static map<RString,CreateScreenFn>	*g_pmapRegistrees = nullptr;

/** @brief Utility functions for the ScreenManager. */
namespace ScreenManagerUtil
{
	// in draw order first to last
	struct LoadedScreen
	{
		Screen *m_pScreen;

		/* Normally true. If false, the screen is owned by another screen
		 * and was given to us for use, and it's not ours to free. */
		bool m_bDeleteWhenDone;

		ScreenMessage m_SendOnPop;

		LoadedScreen()
		{
			m_pScreen = nullptr;
			m_bDeleteWhenDone = true;
			m_SendOnPop = SM_None;
		}
	};

	Actor				*g_pSharedBGA;  // BGA object that's persistent between screens
	RString				m_sPreviousTopScreen;
	vector<LoadedScreen>	g_ScreenStack;  // bottommost to topmost
	vector<Screen*>		g_OverlayScreens;
	set<RString>		g_setGroupedScreens;
	set<RString>		g_setPersistantScreens;

	vector<LoadedScreen>    g_vPreparedScreens;
	vector<Actor*>          g_vPreparedBackgrounds;

	// Add a screen to g_ScreenStack. This is the only function that adds to g_ScreenStack.
	void PushLoadedScreen( const LoadedScreen &ls )
	{
		LOG->Trace( "PushScreen: \"%s\"", ls.m_pScreen->GetName().c_str() );
		LOG->MapLog( "ScreenManager::TopScreen", "Top Screen: %s", ls.m_pScreen->GetName().c_str() );

		// Be sure to push the screen first, so GetTopScreen returns the screen
		// during BeginScreen.
		g_ScreenStack.push_back( ls );

		// Set the name of the loading screen.
		{
			LuaThreadVariable var1( "PreviousScreen", m_sPreviousTopScreen );
			LuaThreadVariable var2( "LoadingScreen", ls.m_pScreen->GetName() );
			ls.m_pScreen->BeginScreen();
		}

		// If this is the new top screen, save the name.
		if( g_ScreenStack.size() == 1 )
			m_sPreviousTopScreen = ls.m_pScreen->GetName();

		SCREENMAN->RefreshCreditsMessages();

		SCREENMAN->PostMessageToTopScreen( SM_GainFocus, 0 );
	}

	bool ScreenIsPrepped( const RString &sScreenName )
	{
		return std::any_of(g_vPreparedScreens.begin(), g_vPreparedScreens.end(), [&](LoadedScreen const &s) {
			return s.m_pScreen->GetName() == sScreenName;
		});
	}

	/* If the named screen is loaded, remove it from the prepared list and
	 * return it in ls. */
	bool GetPreppedScreen( const RString &sScreenName, LoadedScreen &ls )
	{
		for (vector<LoadedScreen>::iterator s = g_vPreparedScreens.begin(); s != g_vPreparedScreens.end(); ++s)
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
		// Deleting a screen can take enough time to cause a frame skip.
		SCREENMAN->ZeroNextUpdate();
	}

	/* If we're deleting a screen, it's probably releasing texture and other
	 * resources, so trigger cleanups. */
	void AfterDeleteScreen()
	{
		/* Now that we've actually deleted a screen, it makes sense to clear out
		 * cached textures. */
		TEXTUREMAN->DeleteCachedTextures();

		/* Cleanup song data. This can free up a fair bit of memory, so do it
		 * after deleting screens. */
		SONGMAN->Cleanup();
	}

	/* Take ownership of all screens and backgrounds that are owned by
	 * us (this excludes screens where m_bDeleteWhenDone is false).
	 * Clear the prepared lists. The contents of apOut must be
	 * freed by the caller. */
	void GrabPreparedActors( vector<Actor*> &apOut )
	{
		for (LoadedScreen const &s : g_vPreparedScreens)
			if( s.m_bDeleteWhenDone )
				apOut.push_back( s.m_pScreen );
		g_vPreparedScreens.clear();
		for (Actor *a : g_vPreparedBackgrounds)
			apOut.push_back( a );
		g_vPreparedBackgrounds.clear();

		g_setGroupedScreens.clear();
		g_setPersistantScreens.clear();
	}
	
	/* Called when changing screen groups. Delete all prepared screens,
	 * reset the screen group and list of persistant screens. */
	void DeletePreparedScreens()
	{
		vector<Actor*> apActorsToDelete;
		GrabPreparedActors( apActorsToDelete );

		BeforeDeleteScreen();
		for (Actor *a : apActorsToDelete)
		{
			SAFE_DELETE( a );
		}
		AfterDeleteScreen();
	}
};
using namespace ScreenManagerUtil;

RegisterScreenClass::RegisterScreenClass( const RString& sClassName, CreateScreenFn pfn )
{
	if( g_pmapRegistrees == nullptr )
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

	m_bReloadOverlayScreensAfterInput= false;
	m_bZeroNextUpdate = false;
	m_PopTopScreen = SM_Invalid;
	m_OnDonePreparingScreen = SM_Invalid;

}


ScreenManager::~ScreenManager()
{
	LOG->Trace( "ScreenManager::~ScreenManager()" );
	LOG->UnmapLog( "ScreenManager::TopScreen" );

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

// This is called when we start up, and when the theme changes or is reloaded.
void ScreenManager::ThemeChanged()
{
	LOG->Trace( "ScreenManager::ThemeChanged" );

	// reload common sounds
	m_soundStart.Load( THEME->GetPathS("Common","start") );
	m_soundCoin.Load( THEME->GetPathS("Common","coin"), true );
	m_soundCancel.Load( THEME->GetPathS("Common","cancel"), true );
	m_soundInvalid.Load( THEME->GetPathS("Common","invalid") );
	m_soundScreenshot.Load( THEME->GetPathS("Common","screenshot") );

	// reload song manager colors (to avoid crashes) -aj
	SONGMAN->ResetGroupColors();

	ReloadOverlayScreens();

	// force recreate of new BGA
	SAFE_DELETE( g_pSharedBGA );
	g_pSharedBGA = new Actor;
}

void ScreenManager::ReloadOverlayScreens()
{
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
		if(pScreen)
		{
			LuaThreadVariable var2( "LoadingScreen", pScreen->GetName() );
			pScreen->BeginScreen();
			g_OverlayScreens.push_back( pScreen );
		}
	}

	this->RefreshCreditsMessages();
}

void ScreenManager::ReloadOverlayScreensAfterInputFinishes()
{
	m_bReloadOverlayScreensAfterInput= true;
}

Screen *ScreenManager::GetTopScreen()
{
	if( g_ScreenStack.empty() )
		return nullptr;
	return g_ScreenStack[g_ScreenStack.size()-1].m_pScreen;
}

Screen *ScreenManager::GetScreen( int iPosition )
{
	if( iPosition >= (int) g_ScreenStack.size() )
		return nullptr;
	return g_ScreenStack[iPosition].m_pScreen;
}

bool ScreenManager::AllowOperatorMenuButton() const
{
	return std::all_of(g_ScreenStack.begin(), g_ScreenStack.end(), [](LoadedScreen const &s) {
		return s.m_pScreen->AllowOperatorMenuButton();
	});
}

bool ScreenManager::IsScreenNameValid(RString const& name) const
{
	if(name.empty() || !THEME->HasMetric(name, "Class"))
	{
		return false;
	}
	RString ClassName = THEME->GetMetric(name, "Class");
	return g_pmapRegistrees->find(ClassName) != g_pmapRegistrees->end();
}

bool ScreenManager::IsStackedScreen( const Screen *pScreen ) const
{
	return std::any_of(g_ScreenStack.begin() + 1, g_ScreenStack.end(), [&](LoadedScreen const &s) {
		return s.m_pScreen == pScreen;
	});
}

bool ScreenManager::get_input_redirected(PlayerNumber pn)
{
	if(pn >= m_input_redirected.size())
	{
		return false;
	}
	return m_input_redirected[pn];
}

void ScreenManager::set_input_redirected(PlayerNumber pn, bool redir)
{
	while(pn >= m_input_redirected.size())
	{
		m_input_redirected.push_back(false);
	}
	m_input_redirected[pn]= redir;
}

/* Pop the top screen off the stack, sending SM_LoseFocus messages and
 * returning the message the popped screen wants sent to the new top
 * screen. Does not send SM_GainFocus. */
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
		// Move the screen back to the prepared list.
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

	if( g_ScreenStack.size() )
		LOG->MapLog( "ScreenManager::TopScreen", "Top Screen: %s", g_ScreenStack.back().m_pScreen->GetName().c_str() );
	else
		LOG->UnmapLog( "ScreenManager::TopScreen" );

	return ls.m_SendOnPop;
}

void ScreenManager::Update( float fDeltaTime )
{
	// Pop the top screen, if PopTopScreen was called.
	if( m_PopTopScreen != SM_Invalid )
	{
		ScreenMessage SM = m_PopTopScreen;
		m_PopTopScreen = SM_Invalid;

		ScreenMessage SM2 = PopTopScreenInternal();

		SendMessageToTopScreen( SM_GainFocus );
		SendMessageToTopScreen( SM );
		SendMessageToTopScreen( SM2 );
	}

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
	 * So, let's just zero the first update for every screen. */
	ASSERT( !g_ScreenStack.empty() || m_sDelayedScreen != "" );	// Why play the game if there is nothing showing?

	Screen* pScreen = g_ScreenStack.empty() ? nullptr : GetTopScreen();

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

	// Update screens.
	{
		for( unsigned i=0; i<g_ScreenStack.size(); i++ )
			g_ScreenStack[i].m_pScreen->Update( fDeltaTime );

		g_pSharedBGA->Update( fDeltaTime );

		for( unsigned i=0; i<g_OverlayScreens.size(); i++ )
			g_OverlayScreens[i]->Update( fDeltaTime );	
	}

	/* The music may be started on the first update. If we're reading from a CD,
	 * it might not start immediately. Make sure we start playing the sound before
	 * continuing, since it's strange to start rendering before the music starts. */
	if( bFirstUpdate )
		SOUND->Flush();

	/* If we're currently inside a background screen load, and m_sDelayedScreen
	 * is set, then the screen called SetNewScreen before we finished preparing.
	 * Postpone it until we're finished loading. */
	if( m_sDelayedScreen.size() != 0 )
	{
		LoadDelayedScreen();
	}
}

void ScreenManager::Draw()
{
	/* If it hasn't been updated yet, skip the render. We can't call Update(0), since
	 * that'll confuse the "zero out the next update after loading a screen logic.
	 * If we don't render, don't call BeginFrame or EndFrame. That way, we won't
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

	// First, give overlay screens a shot at the input.  If Input returns
	// true, it handled the input, so don't pass it further.
	for( unsigned i = 0; i < g_OverlayScreens.size(); ++i )
	{
		Screen *pScreen = g_OverlayScreens[i];
		bool handled= pScreen->Input(input);
		// Pass input to the screen and lua.  Contention shouldn't be a problem
		// because anybody setting an input callback is probably doing it to
		// do something in addition to whatever the screen does.
		if(pScreen->PassInputToLua(input) || handled)
		{
			if(m_bReloadOverlayScreensAfterInput)
			{
				ReloadOverlayScreens();
				m_bReloadOverlayScreensAfterInput= false;
			}
			return;
		}
	}

	// Pass input to the topmost screen.  If we have a new top screen pending, don't
	// send to the old screen, but do send to overlay screens.
	if( m_sDelayedScreen != "" )
		return;

	if( g_ScreenStack.empty() )
		return;

	if(!get_input_redirected(input.pn))
	{
		g_ScreenStack.back().m_pScreen->Input( input );
	}
	g_ScreenStack.back().m_pScreen->PassInputToLua( input );
}

// Just create a new screen; don't do any associated cleanup.
Screen* ScreenManager::MakeNewScreen( const RString &sScreenName )
{
	RageTimer t;
	LOG->Trace( "Loading screen: \"%s\"", sScreenName.c_str() );

	RString sClassName = THEME->GetMetric( sScreenName,"Class" );

	map<RString,CreateScreenFn>::iterator iter = g_pmapRegistrees->find( sClassName );
	if( iter == g_pmapRegistrees->end() )
	{
		LuaHelpers::ReportScriptErrorFmt("Screen \"%s\" has an invalid class \"%s\".", sScreenName.c_str(), sClassName.c_str());
		return nullptr;
	}

	this->ZeroNextUpdate();

	CreateScreenFn pfn = iter->second;
	Screen *ret = pfn( sScreenName );

	LOG->Trace( "Loaded \"%s\" (\"%s\") in %f", sScreenName.c_str(), sClassName.c_str(), t.GetDeltaTime() );

	return ret;
}

void ScreenManager::PrepareScreen( const RString &sScreenName )
{
	// If the screen is already prepared, stop.
	if( ScreenIsPrepped(sScreenName) )
		return;

	Screen* pNewScreen = MakeNewScreen(sScreenName);
	if(pNewScreen == nullptr)
	{
		return;
	}

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
		Actor *pNewBGA = nullptr;
		for (Actor *a : g_vPreparedBackgrounds)
		{
			if( a->GetName() == sNewBGA )
			{
				pNewBGA = a;
				break;
			}
		}

		// Create the new background before deleting the previous so that we keep
		// any common textures loaded.
		if( pNewBGA == nullptr )
		{
			LOG->Trace( "Loading screen background \"%s\"", sNewBGA.c_str() );
			Actor *pActor = ActorUtil::MakeActor( sNewBGA );
			if( pActor != nullptr )
			{
				pActor->SetName( sNewBGA );
				g_vPreparedBackgrounds.push_back( pActor );
			}
		}
	}

	// Prune any unused fonts now that we have had a chance to reference the fonts
	/*
	if(g_bPruneFonts) {
		FONT->PruneFonts();
	}
	*/

	//TEXTUREMAN->DiagnosticOutput();
}

void ScreenManager::GroupScreen( const RString &sScreenName )
{
	g_setGroupedScreens.insert( sScreenName );
}

void ScreenManager::PersistantScreen( const RString &sScreenName )
{
	g_setPersistantScreens.insert( sScreenName );
}

void ScreenManager::SetNewScreen( const RString &sScreenName )
{
	ASSERT( sScreenName != "" );
	m_sDelayedScreen = sScreenName;
}

/* Activate the screen and/or its background, if either are loaded.
 * Return true if both were activated. */
bool ScreenManager::ActivatePreparedScreenAndBackground( const RString &sScreenName )
{
	bool bLoadedBoth = true;

	// Find the prepped screen.
	if( GetTopScreen() == nullptr || GetTopScreen()->GetName() != sScreenName )
	{
		LoadedScreen ls;
		if( !GetPreppedScreen(sScreenName, ls) )
		{
			bLoadedBoth = false;
		}
		else
		{
			PushLoadedScreen( ls );
		}
	}

	// Find the prepared shared background (if any), and activate it.
	RString sNewBGA = THEME->GetPathB(sScreenName,"background");
	if( sNewBGA != g_pSharedBGA->GetName() )
	{
		Actor *pNewBGA = nullptr;
		if( sNewBGA.empty() )
		{
			pNewBGA = new Actor;
		}
		else
		{
			for (vector<Actor *>::iterator a = g_vPreparedBackgrounds.begin(); a != g_vPreparedBackgrounds.end(); ++a)
			{
				if( (*a)->GetName() == sNewBGA )
				{
					pNewBGA = *a;
					g_vPreparedBackgrounds.erase( a );
					break;
				}
			}
		}

		/* If the BGA isn't loaded yet, load a dummy actor. If we're not going to use the same
		 * BGA for the new screen, always move the old BGA back to g_vPreparedBackgrounds now. */
		if( pNewBGA == nullptr )
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
	if(!IsScreenNameValid(sScreenName))
	{
		LuaHelpers::ReportScriptError("Tried to go to invalid screen: " + sScreenName, "INVALID_SCREEN");
		return;
	}

	// Pop the top screen, if any.
	ScreenMessage SM = PopTopScreenInternal();

	/* If the screen is already prepared, activate it before performing any
	 * cleanup, so it doesn't get deleted by cleanup. */
	bool bLoaded = ActivatePreparedScreenAndBackground( sScreenName );

	vector<Actor*> apActorsToDelete;
	if( g_setGroupedScreens.find(sScreenName) == g_setGroupedScreens.end() )
	{
		/* It's time to delete all old prepared screens. Depending on
		 * DelayedScreenLoad, we can either delete the screens before or after
		 * we load the new screen. Either way, we must remove them from the
		 * prepared list before we prepare new screens. 
		 * If DelayedScreenLoad is true, delete them now; this lowers memory
		 * requirements, but results in redundant loads as we unload common data. */
		if( g_bDelayedScreenLoad )
			DeletePreparedScreens();
		else
			GrabPreparedActors( apActorsToDelete );
	}

	// If the screen wasn't already prepared, load it.
	if( !bLoaded )
	{
		PrepareScreen( sScreenName );

		// Screens may not call SetNewScreen from the ctor or Init(). (We don't do this
		// check inside PrepareScreen; that may be called from a thread for concurrent
		// loading, and the main thread may call SetNewScreen during that time.)
		// Emit an error instead of asserting. -Kyz
		if(!m_sDelayedScreen.empty())
		{
			LuaHelpers::ReportScriptError("Setting a new screen during an InitCommand is not allowed.");
			m_sDelayedScreen= "";
		}

		bLoaded = ActivatePreparedScreenAndBackground( sScreenName );
		ASSERT( bLoaded );
	}

	if( !apActorsToDelete.empty() )
	{
		BeforeDeleteScreen();
		for (Actor *a : apActorsToDelete)
		{
			SAFE_DELETE( a );
		}
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
	bool screen_load_success = GetPreppedScreen( sScreenName, ls );
	ASSERT_M(screen_load_success, ssprintf("ScreenManager::AddNewScreenToTop: Failed to load screen %s", sScreenName.c_str()));

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
	// Make sure only the top screen receives LoseFocus.
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
	if( pTopScreen != nullptr )
		pTopScreen->PostScreenMessage( SM, fDelay );
}

void ScreenManager::SendMessageToTopScreen( ScreenMessage SM )
{
	Screen* pTopScreen = GetTopScreen();
	if( pTopScreen != nullptr )
		pTopScreen->HandleScreenMessage( SM );
}


void ScreenManager::SystemMessage( const RString &sMessage )
{
	LOG->Trace( "%s", sMessage.c_str() );
	Message msg( "SystemMessage" );
	msg.SetParam( "Message", sMessage );
	msg.SetParam( "NoAnimate", false );
	MESSAGEMAN->Broadcast( msg );
}

void ScreenManager::SystemMessageNoAnimate( const RString &sMessage )
{
//	LOG->Trace( "%s", sMessage.c_str() );	// don't log because the caller is likely calling us every frame
	Message msg( "SystemMessage" );
	msg.SetParam( "Message", sMessage );
	msg.SetParam( "NoAnimate", true );
	MESSAGEMAN->Broadcast( msg );
}

void ScreenManager::HideSystemMessage()
{
	MESSAGEMAN->Broadcast( "HideSystemMessage" );
}


void ScreenManager::RefreshCreditsMessages()
{
	MESSAGEMAN->Broadcast( "RefreshCreditText" );
}

void ScreenManager::ZeroNextUpdate()
{
	m_bZeroNextUpdate = true;

	/* Loading probably took a little while.  Let's reset stats.  This prevents us
	 * from displaying an unnaturally low FPS value, and the next FPS value we
	 * display will be accurate, which makes skips in the initial tween-ins more
	 * apparent. */
	DISPLAY->ResetStats();
}

/** @brief Offer a quick way to play any critical sound. */
#define PLAY_CRITICAL(snd) \
{ \
	RageSoundParams p; \
	p.m_bIsCriticalSound = true; \
	snd.Play(false, &p); \
}

/* Always play these sounds, even if we're in a silent attract loop. */
void ScreenManager::PlayInvalidSound()  { PLAY_CRITICAL(m_soundInvalid); }
void ScreenManager::PlayStartSound()  { PLAY_CRITICAL(m_soundStart); }
void ScreenManager::PlayCoinSound()    { PLAY_CRITICAL(m_soundCoin); }
void ScreenManager::PlayCancelSound()  { PLAY_CRITICAL(m_soundCancel); }
void ScreenManager::PlayScreenshotSound() { PLAY_CRITICAL(m_soundScreenshot); }

#undef PLAY_CRITICAL

void ScreenManager::PlaySharedBackgroundOffCommand()
{
	g_pSharedBGA->PlayCommand("Off");
}

// lua start
#include "LuaBinding.h"

/** @brief Allow Lua to have access to the ScreenManager. */ 
class LunaScreenManager: public Luna<ScreenManager>
{
public:
	// Note: PrepareScreen binding is not allowed; loading data inside
	// Lua causes the Lua lock to be held for the duration of the load,
	// which blocks concurrent rendering
	static void ValidateScreenName(lua_State* L, RString& name)
	{
		if(name == "")
		{
			RString errstr= "Screen name is empty.";
			SCREENMAN->SystemMessage(errstr);
			luaL_error(L, errstr.c_str());
		}
		RString ClassName= THEME->GetMetric(name, "Class");
		if(g_pmapRegistrees->find(ClassName) == g_pmapRegistrees->end())
		{
			RString errstr= "Screen \"" + name + "\" has an invalid class \"" + ClassName + "\".";
			SCREENMAN->SystemMessage(errstr);
			luaL_error(L, errstr.c_str());
		}
	}
	static int SetNewScreen( T* p, lua_State *L )
	{
		RString screen= SArg(1);
		ValidateScreenName(L, screen);
		p->SetNewScreen(screen);
		COMMON_RETURN_SELF;
	}
	static int GetTopScreen( T* p, lua_State *L )
	{
		Actor *pScreen = p->GetTopScreen();
		if( pScreen != nullptr )
			pScreen->PushSelf(L);
		else
			lua_pushnil( L );
		return 1;
	}
	static int SystemMessage( T* p, lua_State *L )		{ p->SystemMessage( SArg(1) ); COMMON_RETURN_SELF; }
	static int ScreenIsPrepped( T* p, lua_State *L )	{ lua_pushboolean( L, ScreenManagerUtil::ScreenIsPrepped( SArg(1) ) ); return 1; }
	static int ScreenClassExists( T* p, lua_State *L )	{ lua_pushboolean( L, g_pmapRegistrees->find( SArg(1) ) != g_pmapRegistrees->end() ); return 1; }
	static int AddNewScreenToTop( T* p, lua_State *L )
	{
		RString screen= SArg(1);
		ValidateScreenName(L, screen);
		ScreenMessage SM = SM_None;
		if( lua_gettop(L) >= 2 && !lua_isnil(L,2) )
		{
			RString sMessage = SArg(2);
			SM = ScreenMessageHelpers::ToScreenMessage( sMessage );
		}

		p->AddNewScreenToTop( screen, SM );
		COMMON_RETURN_SELF;
	}
	//static int GetScreenStackSize( T* p, lua_State *L )	{ lua_pushnumber( L, ScreenManagerUtil::g_ScreenStack.size() ); return 1; }
	static int ReloadOverlayScreens( T* p, lua_State *L )	{ p->ReloadOverlayScreens(); COMMON_RETURN_SELF; }

	static int get_input_redirected(T* p, lua_State* L)
	{
		PlayerNumber pn= Enum::Check<PlayerNumber>(L, 1);
		lua_pushboolean(L, p->get_input_redirected(pn));
		return 1;
	}
	static int set_input_redirected(T* p, lua_State* L)
	{
		PlayerNumber pn= Enum::Check<PlayerNumber>(L, 1);
		p->set_input_redirected(pn, BArg(2));
		COMMON_RETURN_SELF;
	}

#define SCRMAN_PLAY_SOUND(sound_name) \
	static int Play##sound_name(T* p, lua_State* L) \
	{ \
		p->Play##sound_name(); \
		COMMON_RETURN_SELF; \
	}
	SCRMAN_PLAY_SOUND(InvalidSound);
	SCRMAN_PLAY_SOUND(StartSound);
	SCRMAN_PLAY_SOUND(CoinSound);
	SCRMAN_PLAY_SOUND(CancelSound);
	SCRMAN_PLAY_SOUND(ScreenshotSound);
#undef SCRMAN_PLAY_SOUND

	LunaScreenManager()
	{
		ADD_METHOD( SetNewScreen );
		ADD_METHOD( GetTopScreen );
		ADD_METHOD( SystemMessage );
		ADD_METHOD( ScreenIsPrepped );
		ADD_METHOD( ScreenClassExists );
		ADD_METHOD( AddNewScreenToTop );
		//ADD_METHOD( GetScreenStackSize );
		ADD_METHOD( ReloadOverlayScreens );
		ADD_METHOD(PlayInvalidSound);
		ADD_METHOD(PlayStartSound);
		ADD_METHOD(PlayCoinSound);
		ADD_METHOD(PlayCancelSound);
		ADD_METHOD(PlayScreenshotSound);
		ADD_GET_SET_METHODS(input_redirected);
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

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
#include "PrefsManager.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "GameState.h"
#include "GameSoundManager.h"
#include "RageDisplay.h"
#include "SongManager.h"
#include "RageTextureManager.h"
#include "ThemeManager.h"
#include "Screen.h"
#include "Foreach.h"
#include "ActorUtil.h"

ScreenManager*	SCREENMAN = NULL;	// global and accessable from anywhere in our program


// Screen registration
static map<CString,CreateScreenFn>	*g_pmapRegistrees = NULL;

void ScreenManager::Register( const CString& sClassName, CreateScreenFn pfn )
{
	if( g_pmapRegistrees == NULL )
		g_pmapRegistrees = new map<CString,CreateScreenFn>;

	map<CString,CreateScreenFn>::iterator iter = g_pmapRegistrees->find( sClassName );
	ASSERT_M( iter == g_pmapRegistrees->end(), ssprintf("Screen class '%s' already registered.", sClassName.c_str()) );

	(*g_pmapRegistrees)[sClassName] = pfn;
}


ScreenManager::ScreenManager()
{
	m_pSharedBGA = new Actor;

	m_bZeroNextUpdate = false;
}


ScreenManager::~ScreenManager()
{
	LOG->Trace( "ScreenManager::~ScreenManager()" );

	EmptyDeleteQueue();

	SAFE_DELETE( m_pSharedBGA );
	for( unsigned i=0; i<m_ScreenStack.size(); i++ )
		SAFE_DELETE( m_ScreenStack[i] );
	DeletePreparedScreens();
	for( unsigned i=0; i<m_OverlayScreens.size(); i++ )
		SAFE_DELETE( m_OverlayScreens[i] );
}

/* This is called when we start up, and when the theme changes or is reloaded. */
void ScreenManager::ThemeChanged()
{
	LOG->Trace( "ScreenManager::ThemeChanged" );

	// reload common sounds
	m_soundStart.Load( THEME->GetPathS("Common","start") );
	m_soundCoin.Load( THEME->GetPathS("Common","coin"), true );
	m_soundInvalid.Load( THEME->GetPathS("Common","invalid") );
	m_soundScreenshot.Load( THEME->GetPathS("Common","screenshot") );

	// reload overlay screens
	for( unsigned i=0; i<m_OverlayScreens.size(); i++ )
		SAFE_DELETE( m_OverlayScreens[i] );
	m_OverlayScreens.clear();

	CString sOverlays = THEME->GetMetric( "Common","OverlayScreens" );
	vector<CString> asOverlays;
	split( sOverlays, ",", asOverlays );
	for( unsigned i=0; i<asOverlays.size(); i++ )
	{
		Screen *pScreen = MakeNewScreenInternal( asOverlays[i] );
		m_OverlayScreens.push_back( pScreen );
	}
	
	this->RefreshCreditsMessages();
}

void ScreenManager::EmptyDeleteQueue()
{
	if( !m_vScreensToDelete.size() )
		return;

	ZeroNextUpdate();

	for( unsigned i=0; i<m_vScreensToDelete.size(); i++ )
		SAFE_DELETE( m_vScreensToDelete[i] );

	m_vScreensToDelete.clear();

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

bool ScreenManager::IsStackedScreen( const Screen *pScreen ) const
{
	/* True if the screen is in the screen stack, but not the first. */
	for( unsigned i = 1; i < m_ScreenStack.size(); ++i )
		if( m_ScreenStack[i] == pScreen )
			return true;
	return false;
}

void ScreenManager::Update( float fDeltaTime )
{
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
	ASSERT( !m_ScreenStack.empty() || m_sDelayedScreen != "" );	// Why play the game if there is nothing showing?

	Screen* pScreen = m_ScreenStack.empty() ? NULL : GetTopScreen();

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

	// Only update the topmost screen on the stack.
	if( pScreen )
		pScreen->Update( fDeltaTime );

	m_pSharedBGA->Update( fDeltaTime );

	for( unsigned i=0; i<m_OverlayScreens.size(); i++ )
		m_OverlayScreens[i]->Update( fDeltaTime );	
	
	/* The music may be started on the first update.  If we're reading from a CD,
	 * it might not start immediately.  Make sure we start playing the sound before
	 * continuing, since it's strange to start rendering before the music starts. */
	if( bFirstUpdate )
		SOUND->Flush();

	EmptyDeleteQueue();

	if( m_sDelayedScreen.size() != 0 )
	{
		LoadDelayedScreen();
	}
}


void ScreenManager::Draw()
{
	/* If it hasn't been updated yet, skip the render.  We can't call Update(0), since
	 * that'll confuse the "zero out the next update after loading a screen logic.
	 * If we don't render, don't call BeginFrame or EndFrame.  That way, we won't
	 * clear the buffer, and we won't wait for vsync. */
	if( m_ScreenStack.size() && m_ScreenStack.back()->IsFirstUpdate() )
		return;

	if( !DISPLAY->BeginFrame() )
		return;

	m_pSharedBGA->Draw();

	if( !m_ScreenStack.empty() && !m_ScreenStack.back()->IsTransparent() )	// top screen isn't transparent
	{
		m_ScreenStack.back()->Draw();
	}
	else
	{
		for( unsigned i=0; i<m_ScreenStack.size(); i++ )	// Draw all screens bottom to top
			m_ScreenStack[i]->Draw();
	}

	for( unsigned i=0; i<m_OverlayScreens.size(); i++ )
		m_OverlayScreens[i]->Draw();


	DISPLAY->EndFrame();
}


void ScreenManager::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
//	LOG->Trace( "ScreenManager::Input( %d-%d, %d-%d, %d-%d, %d-%d )", 
//		DeviceI.device, DeviceI.button, GameI.controller, GameI.button, MenuI.player, MenuI.button, StyleI.player, StyleI.col );

	// First, give overlay screens a shot at the input.  If OverlayInput returns
	// true, it handled the input, so don't pass it further.  OverlayInput could
	// probably be merged with Input, but that would require changing all Input
	// overloads, as well as all MenuLeft, etc. overloads.
	for( unsigned i = 0; i < m_OverlayScreens.size(); ++i )
	{
		Screen *pScreen = m_OverlayScreens[i];
		if( pScreen->OverlayInput(DeviceI, type, GameI, MenuI, StyleI) )
			return;
	}

	// Pass input to the topmost screen.  If we have a new top screen pending, don't
	// send to the old screen, but do send to overlay screens.
	if( m_sDelayedScreen != "" )
		return;

	if( !m_ScreenStack.empty() )
		m_ScreenStack.back()->Input( DeviceI, type, GameI, MenuI, StyleI );
}

/* Just create a new screen; don't do any associated cleanup. */
Screen* ScreenManager::MakeNewScreenInternal( const CString &sScreenName )
{
	RageTimer t;
	LOG->Trace( "Loading screen name '%s'", sScreenName.c_str() );

	CString sClassName = THEME->GetMetric(sScreenName,"Class");
	
	map<CString,CreateScreenFn>::iterator iter = g_pmapRegistrees->find( sClassName );
	ASSERT_M( iter != g_pmapRegistrees->end(), ssprintf("Screen '%s' has an invalid class '%s'",sScreenName.c_str(),sClassName.c_str()) )

	CreateScreenFn pfn = iter->second;
	Screen* ret = pfn( sScreenName );

	LOG->Trace( "Loaded '%s' ('%s') in %f", sScreenName.c_str(), sClassName.c_str(), t.GetDeltaTime());

	this->ZeroNextUpdate();

	return ret;
}

Screen* ScreenManager::MakeNewScreen( const CString &sScreenName )
{
	/* By default, RageSounds handles the song timer.  When we change screens, reset this;
	 * screens turn this off in SM_GainFocus if they handle timers themselves (edit). 
	 * XXX: screens should turn this on in SM_LoseFocus if they handle timers themselves, too */
	SOUND->HandleSongTimer( true );

	/* Cleanup song data.  This can free up a fair bit of memory, so do it before
	 * creating the new screen, to lower peak memory usage slightly. */
	SONGMAN->Cleanup();

	Screen* ret = MakeNewScreenInternal( sScreenName );

	/* Loading probably took a little while.  Let's reset stats.  This prevents us
	 * from displaying an unnaturally low FPS value, and the next FPS value we
	 * display will be accurate, which makes skips in the initial tween-ins more
	 * apparent. */
	DISPLAY->ResetStats();

	return ret;
}

void ScreenManager::PrepareScreen( const CString &sScreenName )
{
	// If the screen is already prepared, stop.
	for( int i = (int)m_vPreparedScreens.size()-1; i>=0; i-- )
	{
		Screen *&pScreen = m_vPreparedScreens[i];
		if( pScreen->m_sName == sScreenName )
			return;
	}

	Screen* pNewScreen = MakeNewScreen(sScreenName);
	m_vPreparedScreens.push_back( pNewScreen );

	/* Don't delete previously prepared versions of the screen's background,
	 * and only prepare it if it's different than the current background
	 * and not already loaded. */
	CString sNewBGA;
	if( pNewScreen->UsesBackground() )
		sNewBGA = THEME->GetPathB(sScreenName,"background");

	if( !sNewBGA.empty() && sNewBGA != m_pSharedBGA->GetName() )
	{
		Actor *pNewBGA = NULL;
		FOREACH( Actor*, m_vPreparedBackgrounds, a )
		{
			if( (*a)->m_sName == sNewBGA )
			{
				pNewBGA = *a;
				break;
			}
		}

		// Create the new background before deleting the previous so that we keep
		// any common textures loaded.
		if( pNewBGA == NULL )
		{
			pNewBGA = ActorUtil::MakeActor( sNewBGA );
			pNewBGA->SetName( sNewBGA );
			m_vPreparedBackgrounds.push_back( pNewBGA );
		}
	}
}

void ScreenManager::DeletePreparedScreens()
{
	ZeroNextUpdate();

	FOREACH( Screen*, m_vPreparedScreens, s )
		SAFE_DELETE( *s );
	m_vPreparedScreens.clear();
	FOREACH( Actor*, m_vPreparedBackgrounds, a )
		SAFE_DELETE( *a );
	m_vPreparedBackgrounds.clear();

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
	m_vScreensToDelete.insert( m_vScreensToDelete.end(), m_ScreenStack.begin(), m_ScreenStack.end() );
	m_ScreenStack.clear();
}

/* Add a screen to m_ScreenStack.  This is the only function that adds to m_ScreenStack. */
void ScreenManager::SetFromNewScreen( Screen *pNewScreen )
{
	if( m_ScreenStack.size() )
		m_ScreenStack.back()->HandleScreenMessage( SM_LoseFocus );

	m_ScreenStack.push_back( pNewScreen );
	
	RefreshCreditsMessages();

	PostMessageToTopScreen( SM_GainFocus, 0 );
}

void ScreenManager::SetNewScreen( const CString &sScreenName )
{
	ASSERT( sScreenName != "" );
	m_sDelayedScreen = sScreenName;
}

void ScreenManager::LoadDelayedScreen()
{
	const bool bWasOnSystemMenu = !m_ScreenStack.empty() && m_ScreenStack.back()->GetScreenType() == system_menu;

	/*
	 * We have a screen to display.  Delete the current screens and load it.
	 * If DelayedScreenLoad is true, clear the old screen first; this lowers
	 * memory requirements, but results in redundant loads as we unload common
	 * data.  If we don't unload the old screen here, it'll be deleted below.
	 */
	if( PREFSMAN->m_bDelayedScreenLoad )
	{
		ClearScreenStack();
		EmptyDeleteQueue();
	}

	CString sScreenName = m_sDelayedScreen;
	m_sDelayedScreen = "";


	// Load the screen, if it's not already prepared.
	PrepareScreen( sScreenName );

	//
	// Find the prepped screen.
	//
	Screen* pNewScreen = NULL;
	FOREACH( Screen*, m_vPreparedScreens, s )
	{
		if( (*s)->m_sName == sScreenName )
		{
			pNewScreen = *s;
			m_vPreparedScreens.erase( s );
			break;
		}
	}
	ASSERT( pNewScreen != NULL );

	if( m_sDelayedScreen != "" )
	{
		// While constructing this Screen, its constructor called
		// SetNewScreen again!  That SetNewScreen Command should
		// override this older one.

		// This is no longer allowed.  Instead, figure out which screen
		// you really wanted in the first place with Lua, and don't waste
		// time constructing an extra screen.

		FAIL_M( ssprintf("%s, %s", sScreenName.c_str(), m_sDelayedScreen.c_str()) );
	}
	
	// Find the prepared shared background (if any), and activate it.
	CString sNewBGA;
	if( pNewScreen->UsesBackground() )
		sNewBGA = THEME->GetPathB(sScreenName,"background");
	if( sNewBGA != m_pSharedBGA->GetName() )
	{
		Actor *pNewBGA = NULL;
		if( sNewBGA.empty() )
			pNewBGA = new Actor;
		else
		{
			FOREACH( Actor*, m_vPreparedBackgrounds, a )
			{
				if( (*a)->m_sName == sNewBGA )
				{
					pNewBGA = *a;
					m_vPreparedBackgrounds.erase( a );
					break;
				}
			}
		}
		ASSERT( pNewBGA != NULL );

		SAFE_DELETE( m_pSharedBGA );
		m_pSharedBGA = pNewBGA;
		m_pSharedBGA->PlayCommand( "On" );
	}

	bool bIsOnSystemMenu = pNewScreen->GetScreenType() == system_menu;
	
	// If we're exiting a system menu, persist settings in case we don't exit normally
	if( bWasOnSystemMenu && !bIsOnSystemMenu )
		PREFSMAN->SaveGlobalPrefsToDisk();

	if( !PREFSMAN->m_bDelayedScreenLoad )
	{
		ClearScreenStack();
		EmptyDeleteQueue();
	}

	LOG->Trace("... SetFromNewScreen");
	SetFromNewScreen( pNewScreen );
	ZeroNextUpdate();
}

void ScreenManager::AddNewScreenToTop( const CString &sScreenName )
{
	ZeroNextUpdate();

	Screen* pNewScreen = MakeNewScreen(sScreenName);
	SetFromNewScreen( pNewScreen );
}

void ScreenManager::PopTopScreen( ScreenMessage SM )
{
	ASSERT( m_ScreenStack.size() > 0 );

	Screen* pScreenToPop = m_ScreenStack.back();	// top menu
	pScreenToPop->HandleScreenMessage( SM_LoseFocus );
	m_ScreenStack.erase( m_ScreenStack.end()-1, m_ScreenStack.end() );
	m_vScreensToDelete.push_back( pScreenToPop );

	/* Post to the new top.  This must be done now; otherwise, we'll have a single
	 * frame between popping and these messages, which can result in a frame where eg.
	 * input is accepted where it shouldn't be. */
	SendMessageToTopScreen( SM );
	SendMessageToTopScreen( SM_GainFocus );
}

void ScreenManager::PostMessageToTopScreen( ScreenMessage SM, float fDelay )
{
	if( m_ScreenStack.size() )
	{
		Screen* pTopScreen = m_ScreenStack.back();
		pTopScreen->PostScreenMessage( SM, fDelay );
	}
}

void ScreenManager::SendMessageToTopScreen( ScreenMessage SM )
{
	if( m_ScreenStack.size() )
	{
		Screen* pTopScreen = m_ScreenStack.back();
		pTopScreen->HandleScreenMessage( SM );
	}
}


void ScreenManager::SystemMessage( const CString &sMessage )
{
	m_sSystemMessage = sMessage;
	LOG->Trace( "%s", sMessage.c_str() );
	MESSAGEMAN->Broadcast( "SystemMessage" );
}

void ScreenManager::SystemMessageNoAnimate( const CString &sMessage )
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
	CString joined;
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
	m_bZeroNextUpdate = true;
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

void ScreenManager::PlaySharedBackgroundOffCommand()
{
	m_pSharedBGA->PlayCommand("Off");
}

// lua start
#include "LuaBinding.h"

class LunaScreenManager: public Luna<ScreenManager>
{
public:
	LunaScreenManager() { LUA->Register( Register ); }

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

	static void Register(lua_State *L)
	{
		ADD_METHOD( SetNewScreen )
		ADD_METHOD( GetTopScreen )
		ADD_METHOD( SystemMessage )

		Luna<T>::Register( L );

		// Add global singleton if constructed already.  If it's not constructed yet,
		// then we'll register it later when we reinit Lua just before 
		// initializing the display.
		if( SCREENMAN )
		{
			lua_pushstring(L, "SCREENMAN");
			SCREENMAN->PushSelf( L );
			lua_settable(L, LUA_GLOBALSINDEX);
		}
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

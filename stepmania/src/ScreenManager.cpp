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
#include "ScreenSystemLayer.h"
#include "BGAnimation.h"
#include "Foreach.h"

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
	m_pSharedBGA = new BGAnimation;
	m_SystemLayer = NULL;

	m_MessageSendOnPop = SM_None;

	/* By the time this is constructed, THEME has already been set up and set to
	 * the current theme.  Call ThemeChanged(), to handle the starting theme
	 * and set up m_SystemLayer. */
	ASSERT( THEME );
	ASSERT( !THEME->GetCurThemeName().empty() );
	this->ThemeChanged();
}


ScreenManager::~ScreenManager()
{
	LOG->Trace( "ScreenManager::~ScreenManager()" );

	EmptyDeleteQueue();

	SAFE_DELETE( m_pSharedBGA );
	for( unsigned i=0; i<m_ScreenStack.size(); i++ )
		SAFE_DELETE( m_ScreenStack[i] );
	DeletePreparedScreens();
	SAFE_DELETE( m_SystemLayer );
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
	SAFE_DELETE( m_SystemLayer );
	m_SystemLayer = new ScreenSystemLayer;
	m_SystemLayer->Init();
	m_SystemLayer->RefreshCreditsMessages();
	
	// reload shared BGA
	m_pSharedBGA->LoadFromAniDir( m_sLastLoadedBackgroundPath );
}

void ScreenManager::EmptyDeleteQueue()
{
	if(!m_vScreensToDelete.size())
		return;

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
	ASSERT( !m_ScreenStack.empty() || m_sDelayedScreen != "" );	// Why play the game if there is nothing showing?

	Screen* pScreen = m_ScreenStack.empty() ? NULL : GetTopScreen();

	/* Loading a new screen can take seconds and cause a big jump on the new 
	 * Screen's first update.  Clamp the first update delta so that the 
	 * animations don't jump. */
	if( pScreen && pScreen->IsFirstUpdate() )
		fDeltaTime = 0;

	if( pScreen )
		pScreen->Update( fDeltaTime );

	m_pSharedBGA->Update( fDeltaTime );

	m_SystemLayer->Update( fDeltaTime );
	
	EmptyDeleteQueue();

	if(m_sDelayedScreen.size() != 0)
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


Screen* ScreenManager::MakeNewScreen( const CString &sScreenName )
{
	/* By default, RageSounds handles the song timer.  When we change screens, reset this;
	 * screens turn this off in SM_GainFocus if they handle timers themselves (edit). 
	 * XXX: screens should turn this on in SM_LoseFocus if they handle timers themselves, too */
	SOUND->HandleSongTimer( true );

	/* Cleanup song data.  This can free up a fair bit of memory, so do it before
	 * creating the new screen, to lower peak memory usage slightly. */
	SONGMAN->Cleanup();

	RageTimer t;
	LOG->Trace( "Loading screen name '%s'", sScreenName.c_str() );

	CString sClassName = sScreenName;
	// Look up the class in the metrics group sName
	if( THEME->HasMetric(sClassName,"Class") )
		sClassName = THEME->GetMetric(sClassName,"Class");
	
	map<CString,CreateScreenFn>::iterator iter = g_pmapRegistrees->find( sClassName );
	ASSERT_M( iter != g_pmapRegistrees->end(), ssprintf("Screen '%s' has an invalid class '%s'",sScreenName.c_str(),sClassName.c_str()) )

	CreateScreenFn pfn = iter->second;
	Screen* ret = pfn( sScreenName );

	LOG->Trace( "Loaded '%s' ('%s') in %f", sScreenName.c_str(), sClassName.c_str(), t.GetDeltaTime());

	/* Loading probably took a little while.  Let's reset stats.  This prevents us
	 * from displaying an unnaturally low FPS value, and the next FPS value we
	 * display will be accurate, which makes skips in the initial tween-ins more
	 * apparent. */
	DISPLAY->ResetStats();

	return ret;
}

void ScreenManager::PrepareScreen( const CString &sScreenName )
{
	// Delete previously prepared versions of the screen.
	FOREACH( Screen*, m_vPreparedScreens, s )
	{
		if( (*s)->m_sName == sScreenName )
		{
			SAFE_DELETE( *s );
			break;
		}
	}

	m_vPreparedScreens.push_back( MakeNewScreen(sScreenName) );
}

void ScreenManager::DeletePreparedScreens()
{
	FOREACH( Screen*, m_vPreparedScreens, s )
		SAFE_DELETE( *s );
	m_vPreparedScreens.clear();

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
	m_vScreensToDelete.insert(m_vScreensToDelete.end(), m_ScreenStack.begin(), m_ScreenStack.end());
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

void ScreenManager::SetNewScreen( const CString &sScreenName )
{
	m_sDelayedScreen = sScreenName;

	/* If we're not delaying screen loads, load it now.  Otherwise, we'll load
	 * it on the next iteration.  Only delay if we already have a screen
	 * loaded; otherwise, there's no reason to delay. */
	if(!PREFSMAN->m_bDelayedScreenLoad) // || m_ScreenStack.empty() )
		LoadDelayedScreen();
}

void ScreenManager::LoadDelayedScreen()
{
retry:
	CString sScreenName = m_sDelayedScreen;
	m_sDelayedScreen = "";

	Screen* pOldTopScreen = m_ScreenStack.empty() ? NULL : m_ScreenStack.back();


	//
	// Search prepped screens to see if we already have this screen available.
	// If not prepped, then make it.
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
	if( pNewScreen == NULL )
		pNewScreen = MakeNewScreen(sScreenName);


	if( pOldTopScreen!=NULL  &&  m_ScreenStack.back()!=pOldTopScreen )
	{
		// While constructing this Screen, it's constructor called
		// SetNewScreen again!  That SetNewScreen Command should
		// override this older one.
		SAFE_DELETE( pNewScreen );
		return;
	}

	if( PREFSMAN->m_bDelayedScreenLoad && m_sDelayedScreen != "" )
	{
		/* Same deal: the ctor called SetNewScreen again.  Delete the screen
		 * we just made, but don't delay again. */
		SAFE_DELETE( pNewScreen );
		goto retry;
	}
	
	// Load shared BGAnimation
	CString sNewBGA;
	if( pNewScreen->UsesBackground() )
		sNewBGA = THEME->GetPathToB( sScreenName+" background", true );
	if( sNewBGA.empty() )
	{
		m_pSharedBGA->Unload();
		m_sLastLoadedBackgroundPath = "";
	}
	else if( m_sLastLoadedBackgroundPath != sNewBGA )
	{
		// Create the new background before deleting the previous so that we keep
		// any common textures loaded.
		BGAnimation *pNewBGA = new BGAnimation;
		pNewBGA->LoadFromAniDir( sNewBGA );
		SAFE_DELETE( m_pSharedBGA );
		m_pSharedBGA = pNewBGA;

		m_sLastLoadedBackgroundPath = sNewBGA;
	}

	bool bWasOnSystemMenu = GAMESTATE->m_bIsOnSystemMenu;

	/* If this is a system menu, don't let the operator key touch it! 
		However, if you add an options screen, please include it here -- Miryokuteki */
	/* TODO: Don't have a hard-coded list of system menu names.  Make it a property
	 * of the Screen. */
	if(	sScreenName == "ScreenOptionsMenu" ||
		sScreenName == "ScreenMachineOptions" || 
		sScreenName == "ScreenInputOptions" || 
		sScreenName == "ScreenGraphicOptions" || 
		sScreenName == "ScreenGameplayOptions" || 
		sScreenName == "ScreenMapControllers" || 
		sScreenName == "ScreenAppearanceOptions" || 
		sScreenName == "ScreenEdit" || 
		sScreenName == "ScreenEditMenu" || 
		sScreenName == "ScreenSoundOptions" ) 
		GAMESTATE->m_bIsOnSystemMenu = true;
	else 
		GAMESTATE->m_bIsOnSystemMenu = false;
	
	// If we're exiting a system menu, persist settings in case we don't exit normally
	if( bWasOnSystemMenu && !GAMESTATE->m_bIsOnSystemMenu )
		PREFSMAN->SaveGlobalPrefsToDisk();

	LOG->Trace("... SetFromNewScreen");
	SetFromNewScreen( pNewScreen, false );
}

void ScreenManager::AddNewScreenToTop( const CString &sScreenName, ScreenMessage messageSendOnPop )
{	
	/* Send this before making the new screen, since it might set things that will be re-set
	 * in the new screen's ctor. */
	if( m_ScreenStack.size() )
		m_ScreenStack.back()->HandleScreenMessage( SM_LoseFocus );

	Screen* pNewScreen = MakeNewScreen(sScreenName);
	SetFromNewScreen( pNewScreen, true );
	m_MessageSendOnPop = messageSendOnPop;
}

#include "ScreenPrompt.h"
#include "ScreenTextEntry.h"
#include "ScreenMiniMenu.h"

void ScreenManager::Prompt( ScreenMessage SM_SendWhenDone, const CString &sText, bool bYesNo, bool bDefaultAnswer, void(*OnYes)(void*), void(*OnNo)(void*), void* pCallbackData )
{
	if( m_ScreenStack.size() )
		m_ScreenStack.back()->HandleScreenMessage( SM_LoseFocus );

	// add the new state onto the back of the array
	Screen *pNewScreen = new ScreenPrompt( sText, bYesNo, bDefaultAnswer, OnYes, OnNo, pCallbackData);
	pNewScreen->Init();
	SetFromNewScreen( pNewScreen, true );

	m_MessageSendOnPop = SM_SendWhenDone;
}

void ScreenManager::TextEntry( ScreenMessage SM_SendWhenDone, CString sQuestion, CString sInitialAnswer, void(*OnOK)(CString sAnswer), void(*OnCancel)() )
{	
	if( m_ScreenStack.size() )
		m_ScreenStack.back()->HandleScreenMessage( SM_LoseFocus );

	// add the new state onto the back of the array
	Screen *pNewScreen = new ScreenTextEntry( "ScreenTextEntry", sQuestion, sInitialAnswer, OnOK, OnCancel );
	pNewScreen->Init();
	SetFromNewScreen( pNewScreen, true );

	m_MessageSendOnPop = SM_SendWhenDone;
}

void ScreenManager::MiniMenu( Menu* pDef, ScreenMessage SM_SendOnOK, ScreenMessage SM_SendOnCancel )
{
	if( m_ScreenStack.size() )
		m_ScreenStack.back()->HandleScreenMessage( SM_LoseFocus );

	// add the new state onto the back of the array
	Screen *pNewScreen = new ScreenMiniMenu( pDef, SM_SendOnOK, SM_SendOnCancel );
	pNewScreen->Init();
	SetFromNewScreen( pNewScreen, true );
}

void ScreenManager::PopTopScreen( ScreenMessage SM )
{
	Screen* pScreenToPop = m_ScreenStack.back();	// top menu
	pScreenToPop->HandleScreenMessage( SM_LoseFocus );
	m_ScreenStack.erase(m_ScreenStack.end()-1, m_ScreenStack.end());
	m_vScreensToDelete.push_back( pScreenToPop );

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


void ScreenManager::SystemMessage( const CString &sMessage )
{
	LOG->Trace( "%s", sMessage.c_str() );
	m_SystemLayer->SystemMessage( sMessage );
}

void ScreenManager::SystemMessageNoAnimate( const CString &sMessage )
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

void ScreenManager::PlaySharedBackgroundOffCommand()
{
	m_pSharedBGA->PlayCommand("Off");
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

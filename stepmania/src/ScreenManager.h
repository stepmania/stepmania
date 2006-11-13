/* ScreenManager - Manager/container for Screens. */

#ifndef SCREEN_MANAGER_H
#define SCREEN_MANAGER_H

#include "ScreenMessage.h"
#include "RageSound.h"

class Actor;
class Screen;
struct Menu;
struct lua_State;
class InputEventPlus;


class ScreenManager
{
public:
	ScreenManager();
	~ScreenManager();

	// pass these messages along to the current state
	void Update( float fDeltaTime );
	void Draw();
	void Input( const InputEventPlus &input );

	// Main screen stack management
	void SetNewScreen( const RString &sName );
	void AddNewScreenToTop( const RString &sName, ScreenMessage SendOnPop=SM_None );
	void PrepareScreen( const RString &sScreenName );	// creates and caches screen so that the next call to SetNewScreen for the prep'd screen will be very quick.
	void GroupScreen( const RString &sScreenName );
	void PersistantScreen( const RString &sScreenName );
	void PopTopScreen( ScreenMessage SM );
	void PopAllScreens();
	Screen *GetTopScreen();
	Screen *GetScreen( int iPosition );
	bool AllowOperatorMenuButton() const;

	// System messages
	void SystemMessage( const RString &sMessage );
	void SystemMessageNoAnimate( const RString &sMessage );
	void HideSystemMessage();
	RString GetCurrentSystemMessage() const { return m_sSystemMessage; }

	// Screen messages
	void PostMessageToTopScreen( ScreenMessage SM, float fDelay );
	void SendMessageToTopScreen( ScreenMessage SM );

	void RefreshCreditsMessages();
	void ThemeChanged();


	/* Return true if the given screen is in the main screen stack, but not the bottommost
	 * screen.  If true, the screen should usually exit by popping itself, not by loading
	 * another screen. */
	bool IsStackedScreen( const Screen *pScreen ) const;

	// Lua
	void PushSelf( lua_State *L );

	void	PlaySharedBackgroundOffCommand();
	void    ZeroNextUpdate();
private:
	Screen		*m_pInputFocus; // NULL = top of m_ScreenStack

	RString		m_sSystemMessage;
	
	// Screen loads, removals, and concurrent prepares are delayed until the next update.
	RString		m_sDelayedScreen;
	RString		m_sDelayedConcurrentPrepare;
	ScreenMessage	m_OnDonePreparingScreen;
	ScreenMessage	m_PopTopScreen;

	// Set this to true anywhere we create of delete objects.  These 
	// operations take a long time, and will cause a skip on the next update.
	bool		m_bZeroNextUpdate;

	Screen *MakeNewScreen( const RString &sName );
	void LoadDelayedScreen();
	bool ActivatePreparedScreenAndBackground( const RString &sScreenName );
	ScreenMessage PopTopScreenInternal( bool bSendLoseFocus = true );

	// Keep these sounds always loaded, because they could be 
	// played at any time.  We want to eliminate SOUND->PlayOnce
public:
	void PlayStartSound();
	void PlayCoinSound();
	void PlayCancelSound();
	void PlayInvalidSound();
	void PlayScreenshotSound();

private:
	RageSound	m_soundStart;
	RageSound	m_soundCoin;
	RageSound	m_soundCancel;
	RageSound	m_soundInvalid;
	RageSound	m_soundScreenshot;
};


extern ScreenManager*	SCREENMAN;	// global and accessable from anywhere in our program

#endif

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

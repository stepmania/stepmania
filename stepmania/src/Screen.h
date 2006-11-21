/* Screen - Class that holds a screen-full of Actors. */

#ifndef SCREEN_H
#define SCREEN_H

#include "ActorFrame.h"
#include "ScreenMessage.h"
#include "InputFilter.h"
#include "ThemeMetric.h"
#include "PlayerNumber.h"

class InputEventPlus;
class Screen;
typedef Screen* (*CreateScreenFn)(const RString& sClassName);
// Each Screen class should have a REGISTER_SCREEN_CLASS in its CPP file.
struct RegisterScreenClass { RegisterScreenClass( const RString &sClassName, CreateScreenFn pfn ); };
#define REGISTER_SCREEN_CLASS( className ) \
	static Screen* Create##className( const RString &sName ) { LuaThreadVariable var( "LoadingScreen", sName ); Screen *pRet = new className; pRet->SetName( sName ); Screen::InitScreen( pRet ); return pRet; } \
	static RegisterScreenClass register_##className( #className, Create##className )

enum ScreenType
{
	attract,
	game_menu,
	gameplay,
	system_menu
};

class Screen : public ActorFrame
{
public:
	static void InitScreen( Screen *pScreen );

	virtual ~Screen();

	/* This is called immediately after construction, to allow initializing after all
	 * derived classes exist.  (Don't call it directly; use InitScreen.) */
	virtual void Init();

	/* This is called immediately before the screen is used. */
	virtual void BeginScreen();

	/* This is called when the screen is popped. */
	virtual void EndScreen() { }

	virtual void Update( float fDeltaTime );
	virtual bool OverlayInput( const InputEventPlus &input );
	virtual void Input( const InputEventPlus &input );
	virtual void HandleScreenMessage( const ScreenMessage SM );
	void SetLockInputSecs( float f ) { m_fLockInputSecs = f; }

	void PostScreenMessage( const ScreenMessage SM, float fDelay );
	void ClearMessageQueue();
	void ClearMessageQueue( const ScreenMessage SM );	// clear of a specific SM

	virtual ScreenType GetScreenType() const { return ALLOW_OPERATOR_MENU_BUTTON ? game_menu : system_menu; }
	bool AllowOperatorMenuButton() const { return ALLOW_OPERATOR_MENU_BUTTON; }

	//
	// Lua
	//
	virtual void PushSelf( lua_State *L );

protected:
	// structure for holding messages sent to a Screen
	struct QueuedScreenMessage {
		ScreenMessage SM;  
		float fDelayRemaining;
	};
	vector<QueuedScreenMessage>	m_QueuedMessages;
	static bool SortMessagesByDelayRemaining(const QueuedScreenMessage &m1, const QueuedScreenMessage &m2);

	ThemeMetric<bool>	ALLOW_OPERATOR_MENU_BUTTON;

	// If left blank, the NextScreen metric will be used.
	RString m_sNextScreen;
	ScreenMessage m_smSendOnPop;

	float                           m_fLockInputSecs;

public:
	RString GetNextScreen() const;
	RString GetPrevScreen() const;

	// let subclass override if they want
	virtual void MenuUp(	const InputEventPlus &input ) { }
	virtual void MenuDown(	const InputEventPlus &input ) { }
	virtual void MenuLeft(	const InputEventPlus &input ) { }
	virtual void MenuRight( const InputEventPlus &input ) { }
	virtual void MenuStart( const InputEventPlus &input ) { }
	virtual void MenuSelect( const InputEventPlus &input ) { }
	virtual void MenuBack(	const InputEventPlus &input ) { }
	virtual void MenuCoin(	const InputEventPlus &input ) { }
};

#endif

/*
 * (c) 2001-2004 Chris Danford
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

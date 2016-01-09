#ifndef SCREEN_H
#define SCREEN_H

#include "ActorFrame.h"
#include "ScreenMessage.h"
#include "InputFilter.h"
#include "ThemeMetric.h"
#include "PlayerNumber.h"
#include "InputQueue.h"
#include "CodeSet.h"
#include "LightsManager.h"
#include "EnumHelper.h"

class InputEventPlus;
class Screen;
typedef Screen* (*CreateScreenFn)(const RString& sClassName);
/**
 * @brief Allow registering the screen for easier access.
 *
 * Each Screen class should have a REGISTER_SCREEN_CLASS in its CPP file.
 */
struct RegisterScreenClass { RegisterScreenClass( const RString &sClassName, CreateScreenFn pfn ); };
#define REGISTER_SCREEN_CLASS( className ) \
	static Screen* Create##className( const RString &sName ) { LuaThreadVariable var( "LoadingScreen", sName ); Screen *pRet = new className; pRet->SetName( sName ); Screen::InitScreen( pRet ); return pRet; } \
	static RegisterScreenClass register_##className( #className, Create##className )

/** @brief The different types of screens available. */
enum ScreenType
{
	attract, /**< The attract/demo mode, inviting players to play. */
	game_menu, /**< The menu screens, where options can be set before playing. */
	gameplay, /**< The gameplay screen, where the actual game takes place. */
	system_menu, /**< The system/operator menu, where special options are set. */
	NUM_ScreenType, /**< The number of screen types. */
	ScreenType_Invalid
};
const RString& ScreenTypeToString( ScreenType st );
LuaDeclareType( ScreenType );

/** @brief Class that holds a screen-full of Actors. */
class Screen : public ActorFrame
{
public:
	static void InitScreen( Screen *pScreen );

	virtual ~Screen();

	/**
	 * @brief This is called immediately after construction, 
	 * to allow initializing after all derived classes exist.
	 *
	 * Don't call it directly; use InitScreen instead. */
	virtual void Init();

	/** @brief This is called immediately before the screen is used. */
	virtual void BeginScreen();

	/** @brief This is called when the screen is popped. */
	virtual void EndScreen();

	virtual void Update( float fDeltaTime );
	virtual bool Input( const InputEventPlus &input );
	virtual void HandleScreenMessage( const ScreenMessage SM );
	void SetLockInputSecs( float f ) { m_fLockInputSecs = f; }

	/**
	 * @brief Put the specified message onto the screen for a specified time.
	 * @param SM the message to put on the screen.
	 * @param fDelay The length of time it stays up. */
	void PostScreenMessage( const ScreenMessage SM, float fDelay );
	/** @brief Clear the entire message queue. */
	void ClearMessageQueue();
	/**
	 * @brief Clear the message queue of a specific ScreenMessage.
	 * @param SM the specific ScreenMessage to get out of the Queue. */
	void ClearMessageQueue( const ScreenMessage SM );

	virtual ScreenType GetScreenType() const { return ALLOW_OPERATOR_MENU_BUTTON ? game_menu : system_menu; }
	bool AllowOperatorMenuButton() const { return ALLOW_OPERATOR_MENU_BUTTON; }
	/**
	 * @brief Determine if we allow extra players to join in on this screen.
	 * @return false, for players should never be able to join while in progress. */
	virtual bool AllowLateJoin() const { return false; }

	// Lua
	virtual void PushSelf( lua_State *L );

protected:
	/** @brief Holds the messages sent to a Screen. */
	struct QueuedScreenMessage {
		/** @brief The message being held. */
		ScreenMessage SM;  
		/** @brief How long the message is up. */
		float fDelayRemaining;
	};
	/** @brief The list of messages that are sent to a Screen. */
	vector<QueuedScreenMessage>	m_QueuedMessages;
	static bool SortMessagesByDelayRemaining(const QueuedScreenMessage &m1, const QueuedScreenMessage &m2);

	InputQueueCodeSet	m_Codes;

	/** @brief Do we allow the operator menu button to be pressed here? */
	ThemeMetric<bool>	ALLOW_OPERATOR_MENU_BUTTON;
	/** @brief Do we handle the back button being pressed here? */
	ThemeMetric<bool>	HANDLE_BACK_BUTTON;
	ThemeMetric<float>	REPEAT_RATE;
	ThemeMetric<float>	REPEAT_DELAY;
	ThemeMetric<LightsMode> LIGHTS_MODE;

	/**
	 * @brief The next screen to go to once this screen is done.
	 *
	 * If this is blank, the NextScreen metric will be used. */
	RString m_sNextScreen;
	RString m_sPrevScreen;
	ScreenMessage m_smSendOnPop;

	float m_fLockInputSecs;

	// If currently between BeginScreen/EndScreen calls:
	bool m_bRunning;

public:
	RString GetNextScreenName() const;
	RString GetPrevScreen() const;
	void SetNextScreenName(RString const& name);
	void SetPrevScreenName(RString const& name);

	bool PassInputToLua(const InputEventPlus& input);
	void AddInputCallbackFromStack(lua_State* L);
	void RemoveInputCallback(lua_State* L);
	virtual bool AllowCallbackInput() { return true; }

	// let subclass override if they want
	virtual bool MenuUp(const InputEventPlus &) { return false; }
	virtual bool MenuDown(const InputEventPlus &) { return false; }
	virtual bool MenuLeft(const InputEventPlus &) { return false; }
	virtual bool MenuRight(const InputEventPlus &) { return false; }
	virtual bool MenuStart(const InputEventPlus &) { return false; }
	virtual bool MenuSelect(const InputEventPlus &) { return false; }
	virtual bool MenuBack(const InputEventPlus &) { return false; }
	virtual bool MenuCoin(const InputEventPlus &) { return false; }
	// todo? -aj
	//virtual bool LeftClick(const InputEventPlus &) { }
	//virtual bool RightClick(const InputEventPlus &) { }
	//virtual bool MiddleClick(const InputEventPlus &) { }
	//virtual bool MouseWheelUp(const InputEventPlus &) { }
	//virtual bool MouseWheelDown(const InputEventPlus &) { }

private:
	// void* is the key so that we can use lua_topointer to find the callback
	// to remove when removing a callback.
	typedef void const* callback_key_t;
	map<callback_key_t, LuaReference> m_InputCallbacks;
	vector<callback_key_t> m_DelayedCallbackRemovals;
	bool m_CallingInputCallbacks;
	void InternalRemoveCallback(callback_key_t key);
};

#endif

/**
 * @file
 * @author Chris Danford (c) 2001-2004
 * @section LICENSE
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

#ifndef SCREEN_TEXT_ENTRY_H
#define SCREEN_TEXT_ENTRY_H

#include "ScreenWithMenuElements.h"
#include "BitmapText.h"
#include "RageSound.h"
#include "ThemeMetric.h"
#include "InputEventPlus.h"

/** @brief The list of possible keyboard rows. */
enum KeyboardRow
{
	R1, 
	R2, 
	R3, 
	R4, 
	R5, 
	R6, 
	R7,
	KEYBOARD_ROW_SPECIAL,
	NUM_KeyboardRow,
	KeyboardRow_Invalid
};
/** @brief A special foreach loop for the KeyboardRow enum. */
#define FOREACH_KeyboardRow( i ) FOREACH_ENUM( KeyboardRow, i )
/** @brief The maximum number of keys per row. */
const int KEYS_PER_ROW = 13;
/** @brief The list of very special keys inside some rows. */
enum KeyboardRowSpecialKey
{
	SPACEBAR=2, /**< The space bar key. */
	BACKSPACE=5, /**< The backspace key. */
	CANCEL=8,
	DONE=11
};

/** @brief Displays a text entry box over the top of another screen. */
class ScreenTextEntry : public ScreenWithMenuElements
{
public:
	static void SetTextEntrySettings( 
		RString sQuestion, 
		RString sInitialAnswer, 
		int iMaxInputLength, 
		bool(*Validate)(const RString &sAnswer,RString &sErrorOut) = nullptr, 
		void(*OnOK)(const RString &sAnswer) = nullptr, 
		void(*OnCancel)() = nullptr,
		bool bPassword = false,
		bool (*ValidateAppend)(const RString &sAnswerBeforeChar, RString &sAppend) = nullptr,
		RString (*FormatAnswerForDisplay)(const RString &sAnswer) = nullptr
		);
	static void TextEntry( 
		ScreenMessage smSendOnPop, 
		RString sQuestion, 
		RString sInitialAnswer, 
		int iMaxInputLength, 
		bool(*Validate)(const RString &sAnswer,RString &sErrorOut) = nullptr, 
		void(*OnOK)(const RString &sAnswer) = nullptr, 
		void(*OnCancel)() = nullptr,
		bool bPassword = false,
		bool (*ValidateAppend)(const RString &sAnswerBeforeChar, RString &sAppend) = nullptr,
		RString (*FormatAnswerForDisplay)(const RString &sAnswer) = nullptr
		);
	static void Password( 
		ScreenMessage smSendOnPop, 
		const RString &sQuestion, 
		void(*OnOK)(const RString &sPassword) = nullptr, 
		void(*OnCancel)() = nullptr )
	{
		TextEntry( smSendOnPop, sQuestion, "", 255, nullptr, OnOK, OnCancel, true );
	}

	struct TextEntrySettings {
		TextEntrySettings(): smSendOnPop(), sQuestion(""),
			sInitialAnswer(""), iMaxInputLength(0),
			bPassword(false), Validate(), OnOK(), OnCancel(),
			ValidateAppend(), FormatAnswerForDisplay() { }
		ScreenMessage smSendOnPop;
		RString sQuestion;
		RString sInitialAnswer;
		int iMaxInputLength;
		/** @brief Is there a password involved with this setting?
		 *
		 * This parameter doesn't have to be used. */
		bool bPassword;
		LuaReference Validate; // (RString sAnswer, RString sErrorOut; optional)
		LuaReference OnOK; // (RString sAnswer; optional)
		LuaReference OnCancel; // (optional)
		LuaReference ValidateAppend; // (RString sAnswerBeforeChar, RString sAppend; optional)
		LuaReference FormatAnswerForDisplay; // (RString sAnswer; optional)

		// see BitmapText.cpp Attribute::FromStack()  and
		// OptionRowHandler.cpp LoadInternal() for ideas on how to implement the
		// main part, and ImportOption() from OptionRowHandler.cpp for functions.
		void FromStack( lua_State *L );
	};
	void LoadFromTextEntrySettings( const TextEntrySettings &settings );

	static bool FloatValidate( const RString &sAnswer, RString &sErrorOut );
	static bool IntValidate( const RString &sAnswer, RString &sErrorOut );

	virtual void Init();
	virtual void BeginScreen();

	virtual void Update( float fDelta );
	virtual bool Input( const InputEventPlus &input );

	static RString s_sLastAnswer;
	static bool s_bCancelledLast;

	// Lua
	virtual void PushSelf( lua_State *L );

protected:
	void TryAppendToAnswer( RString s );
	void BackspaceInAnswer();
	virtual void TextEnteredDirectly() { }

	virtual void End( bool bCancelled );
private:
	virtual bool MenuStart( const InputEventPlus &input );
	virtual bool MenuBack( const InputEventPlus &input );

	void UpdateAnswerText();

	wstring		m_sAnswer;
	bool		m_bShowAnswerCaret;
	// todo: allow Left/Right to change caret location -aj
	//int			m_iCaretLocation;

	BitmapText	m_textQuestion;
	BitmapText	m_textAnswer;

	RageSound	m_sndType;
	RageSound	m_sndBackspace;

	RageTimer	m_timerToggleCursor;
};

/** @brief Displays a text entry box and keyboard over the top of another screen. */
class ScreenTextEntryVisual: public ScreenTextEntry
{
public:
	~ScreenTextEntryVisual();
	void Init();
	void BeginScreen();

protected:
	void MoveX( int iDir );
	void MoveY( int iDir );
	void PositionCursor();

	virtual void TextEnteredDirectly();

	virtual bool MenuLeft( const InputEventPlus &input );
	virtual bool MenuRight( const InputEventPlus &input );
	virtual bool MenuUp( const InputEventPlus &input );
	virtual bool MenuDown( const InputEventPlus &input );

	virtual bool MenuStart( const InputEventPlus &input );

	int			m_iFocusX;
	KeyboardRow m_iFocusY;

	AutoActor	m_sprCursor;
	BitmapText	*m_ptextKeys[NUM_KeyboardRow][KEYS_PER_ROW];

	RageSound	m_sndChange;

	ThemeMetric<float>	ROW_START_X;
	ThemeMetric<float>	ROW_START_Y;
	ThemeMetric<float>	ROW_END_X;
	ThemeMetric<float>	ROW_END_Y;
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

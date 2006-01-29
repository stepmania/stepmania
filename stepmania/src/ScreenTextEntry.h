/* ScreenTextEntry - Displays a text entry box over the top of another screen. */

#ifndef SCREEN_TEXT_ENTRY_H
#define SCREEN_TEXT_ENTRY_H

#include "ScreenWithMenuElements.h"
#include "BitmapText.h"
#include "RageSound.h"
#include "ThemeMetric.h"

enum KeyboardRow
{
	R1, R2, R3, R4, R5, R6, R7,
	KEYBOARD_ROW_SPECIAL,
	NUM_KEYBOARD_ROWS
};
#define FOREACH_KeyboardRow( i ) FOREACH_ENUM( KeyboardRow, NUM_KEYBOARD_ROWS, i )
const int KEYS_PER_ROW = 13;
enum KeyboardRowSpecialKey
{
	SPACEBAR=2, BACKSPACE=5, CANCEL=8, DONE=11
};

class ScreenTextEntry : public ScreenWithMenuElements
{
public:
	static void TextEntry( 
		ScreenMessage smSendOnPop, 
		RString sQuestion, 
		RString sInitialAnswer, 
		int iMaxInputLength, 
		bool(*Validate)(const RString &sAnswer,RString &sErrorOut) = NULL, 
		void(*OnOK)(const RString &sAnswer) = NULL, 
		void(*OnCanel)() = NULL,
		bool bPassword = false );
	static void Password( 
		ScreenMessage smSendOnPop, 
		const RString &sQuestion, 
		void(*OnOK)(const RString &sPassword) = NULL, 
		void(*OnCanel)() = NULL )
	{
		TextEntry( smSendOnPop, sQuestion, "", 255, NULL, OnOK, OnCanel, true );
	}

	~ScreenTextEntry();
	virtual void Init();
	virtual void BeginScreen();

	virtual void Update( float fDelta );
	virtual void Input( const InputEventPlus &input );
	virtual void TweenOffScreen();

	static RString s_sLastAnswer;
	static bool s_bCancelledLast;

protected:
	void MoveX( int iDir );
	void MoveY( int iDir );
	
	void AppendToAnswer( RString s );
	void BackspaceInAnswer();

	void End( bool bCancelled );

	virtual void MenuLeft( PlayerNumber pn )	{ MoveX(-1); }
	virtual void MenuRight( PlayerNumber pn )	{ MoveX(+1); }
	virtual void MenuUp( PlayerNumber pn )		{ MoveY(-1); }
	virtual void MenuDown( PlayerNumber pn )	{ MoveY(+1); }
	virtual void MenuStart( PlayerNumber pn );
	virtual void MenuBack( PlayerNumber pn );

	void UpdateAnswerText();

	BitmapText		m_textQuestion;
	AutoActor		m_sprAnswerBox;
	wstring			m_sAnswer;
	BitmapText		m_textAnswer;
	bool			m_bShowAnswerCaret;
	
	AutoActor		m_sprCursor;

	int			m_iFocusX;
	KeyboardRow		m_iFocusY;
	
	void PositionCursor();

	BitmapText		*m_ptextKeys[NUM_KEYBOARD_ROWS][KEYS_PER_ROW];
	ThemeMetric<float>	ROW_START_X;
	ThemeMetric<float>	ROW_START_Y;
	ThemeMetric<float>	ROW_END_X;
	ThemeMetric<float>	ROW_END_Y;

	RageSound		m_sndType;
	RageSound		m_sndBackspace;
	RageSound		m_sndChange;
	
	RageTimer		m_timerToggleCursor;
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

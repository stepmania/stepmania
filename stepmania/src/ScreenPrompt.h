/* ScreenPrompt - Displays a prompt on top of another screen.  Use by calling SCREENMAN->AddScreenToTop( new ScreenPrompt(...) ); */

#ifndef SCREEN_PROMPT_H
#define SCREEN_PROMPT_H

#include "Screen.h"
#include "BitmapText.h"
#include "Transition.h"
#include "Quad.h"
#include "RandomSample.h"


class ScreenPrompt : public Screen
{
public:
	ScreenPrompt( CString sName );
	ScreenPrompt( CString sText, bool bYesNoPrompt=false, bool bDefaultAnswer = false, void(*OnYes)(void*) = NULL, void(*OnNo)(void*) = NULL, void* pCallbackData = NULL );

	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();
	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	virtual void HandleScreenMessage( const ScreenMessage SM );

	static bool s_bLastAnswer;

protected:
	void MenuLeft( PlayerNumber pn );
	void MenuRight( PlayerNumber pn );
	void MenuBack( PlayerNumber pn );
	void MenuStart( PlayerNumber pn );


	BGAnimation					m_Background;
	BitmapText		m_textQuestion;
	Quad			m_rectAnswerBox;
	BitmapText		m_textAnswer[2];	// "YES" or "NO"
	bool			m_bYesNoPrompt;		// false = OK prompt, true = YES/NO prompt
	bool			m_bAnswer;		// true = "YES", false = "NO";
	ScreenMessage	m_SMSendWhenDone;
	void(*m_pOnYes)(void*);
	void(*m_pOnNo)(void*);
	void* m_pCallbackData;
	Transition		m_In;
	Transition		m_Out;
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

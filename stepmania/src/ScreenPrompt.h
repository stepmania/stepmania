/*
-----------------------------------------------------------------------------
 File: ScreenPrompt.h

 Desc: Area for testing.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/

#include "Screen.h"
#include "BitmapText.h"
#include "TransitionFade.h"
#include "Quad.h"
#include "RandomSample.h"


enum PromptType{ PROMPT_OK, PROMPT_YES_NO };


const int NUM_QUESTION_LINES = 10;


class ScreenPrompt : public Screen
{
public:
	ScreenPrompt( CString sText, PromptType pt, bool* pbAnswer = NULL );
	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();
	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	virtual void HandleScreenMessage( const ScreenMessage SM );

protected:
	void MenuLeft( PlayerNumber p );
	void MenuRight( PlayerNumber p );
	void MenuBack( PlayerNumber p );
	void MenuStart( PlayerNumber p );


	TransitionFade m_Fade;
	BitmapText		m_textTitle;
	BitmapText		m_textQuestion[NUM_QUESTION_LINES];
	Quad	m_rectAnswerBox;
	BitmapText		m_textAnswer[2];	// "YES" or "NO"
	PromptType		m_PromptType;
	bool*			m_pbAnswer;		// true = "YES", false = "NO";

	RandomSample m_soundSelect;
};


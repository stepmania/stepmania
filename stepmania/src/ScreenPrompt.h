/*
-----------------------------------------------------------------------------
 Class: ScreenPrompt

 Desc: Displays a prompt over the top of another screen.  Must use by calling
	SCREENMAN->AddScreenToTop( new ScreenPrompt(...) );

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Screen.h"
#include "BitmapText.h"
#include "TransitionFade.h"
#include "Quad.h"
#include "RandomSample.h"


enum PromptType{ PROMPT_OK, PROMPT_YES_NO };



class ScreenPrompt : public Screen
{
public:
	ScreenPrompt();
	ScreenPrompt( ScreenMessage SM_SendWhenDone, CString sText, PromptType pt, bool bDefaultAnswer = false, void(*OnYes)(void*) = NULL, void(*OnNo)(void*) = NULL );

	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();
	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	virtual void HandleScreenMessage( const ScreenMessage SM );

protected:
	void MenuLeft( const PlayerNumber p );
	void MenuRight( const PlayerNumber p );
	void MenuBack( const PlayerNumber p );
	void MenuStart( const PlayerNumber p );


	TransitionFade	m_Fade;
	BitmapText		m_textQuestion;
	Quad			m_rectAnswerBox;
	BitmapText		m_textAnswer[2];	// "YES" or "NO"
	PromptType		m_PromptType;
	bool			m_bAnswer;		// true = "YES", false = "NO";
	ScreenMessage	m_SMSendWhenDone;
	void(*m_pOnYes)(void* pContext);
	void(*m_pOnNo)(void* pContext);
};


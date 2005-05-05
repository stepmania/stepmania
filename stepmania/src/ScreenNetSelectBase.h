/* ScreenNetSelectBase - Base screen containing chat room & user list */

#ifndef SCREENNETSELECTBASE_H
#define SCREENNETSELECTBASE_H

#include "ScreenWithMenuElements.h"
#include "Sprite.h"
#include "Quad.h"
#include "BitmapText.h"

class ScreenNetSelectBase : public ScreenWithMenuElements
{
public:
	ScreenNetSelectBase( const CString& sName );
	virtual void Init();

	void Input( const DeviceInput& DeviceI, const InputEventType type,
						const GameInput& GameI, const MenuInput& MenuI,
						const StyleInput& StyleI );
	virtual void HandleScreenMessage( const ScreenMessage SM );
	virtual void TweenOffScreen();

	void UpdateUsers();
	void UpdateTextInput();
private:
	//Chatting
	ColorBitmapText	m_textChatInput;
	ColorBitmapText	m_textChatOutput;
	Sprite			m_sprChatInputBox;
	Sprite			m_sprChatOutputBox;
	CString			m_sTextInput;
	CString			m_actualText;

	//Users Rect
	Quad			m_rectUsersBG;
	vector <BitmapText>	m_textUsers;
};

//Eventually we won't be using quads in this method.
#define SET_QUAD_INIT( actor )	UtilSetQuadInit( actor, m_sName );
void UtilSetQuadInit( Actor& actor, const CString &sClassName );

#endif

/*
 * (c) 2004 Charles Lohr
 * All rights reserved.
 *
 *     based off of ScreenEz2SelectMusic by "Frieza"
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

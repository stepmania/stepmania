/* ScreenSelect - Base class for Style, Difficulty, and Mode selection screens. */

#ifndef SCREEN_SELECT_H
#define SCREEN_SELECT_H

#include "ScreenWithMenuElements.h"
#include "BGAnimation.h"
#include "ModeChoice.h"
#include "CodeDetector.h"

// Derived classes must send this when done
const ScreenMessage SM_AllDoneChoosing = (ScreenMessage)(SM_User+123);	// unique

#define MAX_CHOICES 30

class ScreenSelect : public ScreenWithMenuElements
{
public:
	ScreenSelect( CString sClassName );
	virtual ~ScreenSelect();

	virtual void Update( float fDelta );
	virtual void DrawPrimitives();
	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	virtual void HandleScreenMessage( const ScreenMessage SM );

	virtual void MenuBack( PlayerNumber pn );
	

protected:
	virtual int GetSelectionIndex( PlayerNumber pn ) = 0;
	virtual void UpdateSelectableChoices() = 0;		// derived screens must handle this
	void FinalizeChoices();
	
	vector<BGAnimation*> m_vpBGAnimations;
	vector<ModeChoice>	m_aModeChoices;		// derived classes should look here for what choices are available

	vector<CodeItem>	m_aCodes;
	vector<ModeChoice>	m_aCodeChoices;
	vector<CString>		m_aCodeActions;
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

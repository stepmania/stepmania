/* ScreenSelectStyle - Deprecated. Replaced by ScreenSelectMaster. */

#ifndef ScreenSelectStyle_H
#define ScreenSelectStyle_H

#include "ScreenSelect.h"
#include "Sprite.h"
#include "BitmapText.h"
#include "RageSound.h"

#define MAX_MODE_CHOICES 30

class ScreenSelectStyle : public ScreenSelect
{
public:
	virtual void Init();
	virtual void BeginScreen();

	virtual void MenuLeft( PlayerNumber pn );
	virtual void MenuRight( PlayerNumber pn );
	virtual void MenuStart( PlayerNumber pn );

protected:
	virtual int GetSelectionIndex( PlayerNumber pn );
	virtual void UpdateSelectableChoices();

	void BeforeChange();
	void AfterChange();

	Sprite		m_sprIcon[MAX_MODE_CHOICES];
	// Artists don't make graphics for every single Game, so
	// have a text representation if textures are missing.
	BitmapText	m_textIcon[MAX_MODE_CHOICES];
	Sprite		m_sprPicture[MAX_MODE_CHOICES];
	Sprite		m_sprInfo[MAX_MODE_CHOICES];
	Sprite		m_sprExplanation;
	Sprite		m_sprWarning;
	Sprite		m_sprPremium;
	
	RageSound m_soundChange;

	int m_iSelection;
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

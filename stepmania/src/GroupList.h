#ifndef GROUP_LIST_H
#define GROUP_LIST_H

#include "ActorFrame.h"
#include "Sprite.h"
#include "BitmapText.h"

class GroupList: public ActorFrame
{
	ActorFrame			 m_Frame;
	vector<Sprite *>	 m_sprButtons;
	vector<BitmapText *> m_textLabels;
	vector<ActorFrame *> m_ButtonFrames;
	vector<CString>		 m_asLabels;

	vector<bool>		 m_bHidden;

	/* Currently selected label. */
	int m_iSelection;
	/* Label that's currently at the top of the screen. */
	int m_iTop;

	void BeforeChange();
	void AfterChange();
	bool ItemIsOnScreen( int n ) const;
	void ResetTextSize( int item );

public:
	GroupList();
	~GroupList();

	void SetSelection(unsigned sel);
	int GetSelection() const { return m_iSelection; }
	CString GetSelectionName() const { return m_asLabels[m_iSelection]; }
	void Up();
	void Down();
	void Load( const CStringArray& asGroupNames );
	void TweenOffScreen();
	void TweenOnScreen();
};

#endif

/*
 * (c) 2001-2003 Chris Danford, Glenn Maynard
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

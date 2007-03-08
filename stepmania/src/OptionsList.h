#ifndef SCREEN_OPTIONS_LIST_H
#define SCREEN_OPTIONS_LIST_H

#include "ScreenWithMenuElements.h"
#include "RageSound.h"
#include "Steps.h"
#include "Trail.h"
#include "OptionRowHandler.h"
#include "BitmapText.h"
#include "OptionsCursor.h"

class OptionListRow: public ActorFrame
{
public:
	void Load( const RString &sType );
	void SetFromHandler( const OptionRowHandler *pHandler );
	void SetUnderlines( const vector<bool> &aSelections );

	void PositionCursor( Actor *pCursor, int iSelection );

	void Start();

private:
	vector<BitmapText> m_Text;
	// underline for each ("self or child has selection")
	vector<AutoActor> m_Underlines;

	bool	m_bItemsInTwoRows;

	ThemeMetric<float>	ITEMS_SPACING_Y;
};

class OptionsList: public ActorFrame
{
public:
	OptionsList();
	~OptionsList();

	void Load( RString sType, PlayerNumber pn );

	/* Show the top-level menu. */
	void Open();

	/* Close all menus (for menu timer). */
	void Close();

	void Input( const InputEventPlus &input );
	bool IsOpened() const { return m_asMenuStack.size() > 0; }

	bool Start( PlayerNumber pn );	/* return true if the last menu was popped in response to this press */

private:
	void SwitchMenu( int iDir );
	void PositionCursor();
	const OptionRowHandler *GetCurrentHandler();
	void SwitchToCurrentRow();
	void TweenOnCurrentRow( bool bForward );
	void SetDefaultCurrentRow();
	void Push( RString sDest );
	void Pop();
	void ImportRow( RString sRow );
	void ExportRow( RString sRow );
	bool RowIsMenusOnly( RString sRow ) const;

	vector<RString> m_asLoadedRows;
	map<RString, OptionRowHandler *> m_Rows;
	map<RString, vector<bool> > m_bSelections;

	PlayerNumber m_pn;
	AutoActor m_Cursor;
	OptionListRow m_Row[2];
	int m_iCurrentRow;

	vector<RString> m_asMenuStack;
	int m_iMenuStackSelection;
};


#endif

/*
 * Copyright (c) 2006 Glenn Maynard
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


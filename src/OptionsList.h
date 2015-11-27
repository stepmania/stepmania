#ifndef SCREEN_OPTIONS_LIST_H
#define SCREEN_OPTIONS_LIST_H

#include "ScreenWithMenuElements.h"
#include "RageSound.h"
#include "Steps.h"
#include "Trail.h"
#include "OptionRowHandler.h"
#include "BitmapText.h"
#include "OptionsCursor.h"
#include "CodeSet.h"
#include "ThemeMetric.h"

class OptionsList;
class OptionListRow: public ActorFrame
{
public:
	void Load( OptionsList *pOptions, const std::string &sType );
	void SetFromHandler( const OptionRowHandler *pHandler );
	void SetTextFromHandler( const OptionRowHandler *pHandler );
	void SetUnderlines( const std::vector<bool> &aSelections, const OptionRowHandler *pHandler );

	void PositionCursor( Actor *pCursor, int iSelection );

	void Start();

private:
	OptionsList *m_pOptions;

	std::vector<BitmapText> m_Text;
	// underline for each ("self or child has selection")
	std::vector<AutoActor> m_Underlines;

	bool	m_bItemsInTwoRows;

	ThemeMetric<float>	ITEMS_SPACING_Y;
};
/** @brief A popup options list. */
class OptionsList: public ActorFrame
{
public:
	friend class OptionListRow;

	OptionsList();
	~OptionsList();

	void Load( std::string sType, PlayerNumber pn );
	void Reset();

	void Link( OptionsList *pLink ) { m_pLinked = pLink; }

	/** @brief Show the top-level menu. */
	void Open();

	/** @brief Close all menus (for menu timer). */
	void Close();

	bool Input( const InputEventPlus &input );
	bool IsOpened() const { return m_asMenuStack.size() > 0; }

	bool Start();	// return true if the last menu was popped in response to this press

private:
	ThemeMetric<std::string> TOP_MENU;

	void SelectItem( const std::string &sRowName, int iMenuItem );
	void MoveItem( const std::string &sRowName, int iMove );
	void SwitchMenu( int iDir );
	void PositionCursor();
	void SelectionsChanged( const std::string &sRowName );
	void UpdateMenuFromSelections();
	std::string GetCurrentRow() const;
	const OptionRowHandler *GetCurrentHandler();
	int GetOneSelection( std::string sRow, bool bAllowFail=false ) const;
	void SwitchToCurrentRow();
	void TweenOnCurrentRow( bool bForward );
	void SetDefaultCurrentRow();
	void Push( std::string sDest );
	void Pop();
	void ImportRow( std::string sRow );
	void ExportRow( std::string sRow );
	static int FindScreenInHandler( const OptionRowHandler *pHandler, std::string sScreen );

	InputQueueCodeSet	m_Codes;

	OptionsList		*m_pLinked;

	bool			m_bStartIsDown;
	bool			m_bAcceptStartRelease;

	std::vector<std::string> m_asLoadedRows;
	std::map<std::string, OptionRowHandler *> m_Rows;
	std::map<std::string, std::vector<bool> > m_bSelections;
	std::set<std::string> m_setDirectRows;
	std::set<std::string> m_setTopMenus; // list of top-level menus, pointing to submenus

	PlayerNumber m_pn;
	AutoActor m_Cursor;
	OptionListRow m_Row[2];
	int m_iCurrentRow;

	std::vector<std::string> m_asMenuStack;
	int m_iMenuStackSelection;
};


#endif

/**
 * @file
 * @author 2006 Glenn Maynard (c)
 * @section LICENSE
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


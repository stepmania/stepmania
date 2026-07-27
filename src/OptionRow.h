/* OptionRow - One line in ScreenOptions. */

#ifndef OptionRow_H
#define OptionRow_H

#include "ActorFrame.h"
#include "BitmapText.h"
#include "OptionsCursor.h"
#include "ModIcon.h"
#include "ThemeMetric.h"
#include "AutoActor.h"

#include <array>
#include <cstddef>
#include <vector>


class OptionRowHandler;
class GameCommand;
struct OptionRowDefinition;

RString ITEMS_LONG_ROW_X_NAME( std::size_t p );
RString MOD_ICON_X_NAME( std::size_t p );

class OptionRowType
{
public:
	void Load( const RString &sMetricsGroup, Actor *pParent );

private:
	RString			m_sMetricsGroup;

	BitmapText		m_textItem;
	OptionsCursor	m_Underline[NUM_PLAYERS];
	AutoActor		m_sprFrame;
	BitmapText		m_textTitle;
	ModIcon			m_ModIcon;

	ThemeMetric<float>		ITEMS_START_X;
	ThemeMetric<float>		ITEMS_END_X;
	ThemeMetric<float>		ITEMS_GAP_X;
	ThemeMetric<float>		ITEMS_MIN_BASE_ZOOM;
	ThemeMetric1D<float>	ITEMS_LONG_ROW_X;
	ThemeMetric<float>		ITEMS_LONG_ROW_SHARED_X;
	ThemeMetric1D<float>	MOD_ICON_X;
	ThemeMetric<RageColor>	COLOR_SELECTED;
	ThemeMetric<RageColor>	COLOR_NOT_SELECTED;
	ThemeMetric<RageColor>	COLOR_DISABLED;
	ThemeMetric<float>		TWEEN_SECONDS;
	ThemeMetric<bool>		SHOW_BPM_IN_SPEED_TITLE;
	ThemeMetric<bool>		SHOW_MOD_ICONS;
	ThemeMetric<bool>		SHOW_UNDERLINES;
	ThemeMetric<RString>	MOD_ICON_METRICS_GROUP;

	friend class OptionRow;
};

class OptionRow : public ActorFrame
{
public:
	OptionRow( const OptionRowType *pType );
	~OptionRow();

	void Clear();
	void LoadNormal( OptionRowHandler *pHand, bool bFirstItemGoesDown );
	void LoadExit();

	void SetModIcon( PlayerNumber pn, const RString &sText, GameCommand &gc );

	void ImportOptions( const std::vector<PlayerNumber> &vpns );
	int ExportOptions( const std::vector<PlayerNumber> &vpns, bool bRowHasFocus[NUM_PLAYERS] );

	enum RowType
	{
		RowType_Normal,
		RowType_Exit
	};

	void InitText( RowType type );
	void AfterImportOptions( PlayerNumber pn );

	RString GetRowTitle() const;

	void ChoicesChanged( RowType type, bool reset_focus= true );
	void PositionUnderlines( PlayerNumber pn );
	void PositionIcons( PlayerNumber pn );
	void UpdateText( PlayerNumber pn );
	bool GetRowHasFocus( PlayerNumber pn ) const { return m_bRowHasFocus[pn]; }
	void SetRowHasFocus( PlayerNumber pn, bool bRowHasFocus );
	void UpdateEnabledDisabled();

	int GetOneSelection( PlayerNumber pn, bool bAllowFail=false ) const;
	int GetOneSharedSelection( bool bAllowFail=false ) const;
	void SetOneSelection( PlayerNumber pn, int iChoice );
	void SetOneSharedSelection( int iChoice );
	void SetOneSharedSelectionIfPresent( const RString &sChoice );

	int GetChoiceInRowWithFocus( PlayerNumber pn ) const;
	int GetChoiceInRowWithFocusShared() const;
	void SetChoiceInRowWithFocus( PlayerNumber pn, int iChoice );
	void ResetFocusFromSelection( PlayerNumber pn );

	bool GetSelected( PlayerNumber pn, int iChoice ) const;
	// SetSelected returns true if the choices changed because of setting.
	bool SetSelected( PlayerNumber pn, int iChoice, bool b );

	bool NotifyHandlerOfSelection(PlayerNumber pn, int choice);

	const OptionRowDefinition &GetRowDef() const;
	OptionRowDefinition &GetRowDef();
	RowType GetRowType() const { return m_RowType; }
	const OptionRowHandler *GetHandler() const { return m_pHand; }

	const BitmapText &GetTextItemForRow( PlayerNumber pn, int iChoiceOnRow ) const;
	void GetWidthXY( PlayerNumber pn, int iChoiceOnRow, int &iWidthOut, int &iXOut, int &iYOut ) const;

	// ScreenOptions calls positions m_FrameDestination, then m_Frame tween to that same TweenState.
	unsigned GetTextItemsSize() const { return m_textItems.size(); }
	bool GetFirstItemGoesDown() const { return m_bFirstItemGoesDown; }
	bool GoToFirstOnStart();

	RString GetThemedItemText( int iChoice ) const;

	void SetExitText( RString sExitText );

	void Reload();

	// Messages
	virtual void HandleMessage( const Message &msg );

	// Lua
	void PushSelf( lua_State *L );

protected:
	const OptionRowType *m_pParentType;
	RowType m_RowType;
	OptionRowHandler* m_pHand;

	ActorFrame m_Frame;

	std::vector<BitmapText *>	m_textItems;				// size depends on m_bRowIsLong and which players are joined
	std::vector<OptionsCursor *>	m_Underline[NUM_PLAYERS];	// size depends on m_bRowIsLong and which players are joined

	Actor		*m_sprFrame;
	BitmapText	*m_textTitle;
	ModIcon		*m_ModIcons[NUM_PLAYERS];

	bool		m_bFirstItemGoesDown;
	std::array<bool, NUM_PLAYERS> m_bRowHasFocus;

	int			m_iChoiceInRowWithFocus[NUM_PLAYERS];	// this choice has input focus
	// Only one will true at a time if m_pHand->m_Def.bMultiSelect
	std::vector<bool>		m_vbSelected[NUM_PLAYERS];	// size = m_pHand->m_Def.choices.size()
	Actor::TweenState	m_tsDestination;	// this should approach m_tsDestination.

public:
	void SetDestination( Actor::TweenState &ts, bool bTween );
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

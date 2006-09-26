/* OptionRowHandler - Shows PlayerOptions and SongOptions in icon form. */

#ifndef OptionRowHandler_H
#define OptionRowHandler_H

#include "GameCommand.h"
#include "LuaReference.h"
#include "RageUtil.h"
#include <set>
struct MenuRowDef;

struct ConfOption;

enum SelectType
{
	SELECT_ONE,
	SELECT_MULTIPLE,
	SELECT_NONE,
	NUM_SelectType,
	SelectType_Invalid
};
const RString& SelectTypeToString( SelectType pm );
SelectType StringToSelectType( const RString& s );

enum LayoutType
{
	LAYOUT_SHOW_ALL_IN_ROW,
	LAYOUT_SHOW_ONE_IN_ROW,
	NUM_LayoutType,
	LayoutType_Invalid
};
const RString& LayoutTypeToString( LayoutType pm );
LayoutType StringToLayoutType( const RString& s );

struct OptionRowDefinition
{
	RString m_sName;
	RString m_sExplanationName;
	bool m_bOneChoiceForAllPlayers;
	SelectType m_selectType;
	LayoutType m_layoutType;
	vector<RString> m_vsChoices;
	set<PlayerNumber> m_vEnabledForPlayers;	// only players in this set may change focus to this row
	bool	m_bExportOnChange;
	bool	m_bAllowThemeItems;	// Should be true for dynamic strings.
	bool	m_bAllowThemeTitle;	// Should be true for dynamic strings.
	bool	m_bAllowExplanation;	// if false, ignores ScreenOptions::SHOW_EXPLANATIONS.  Should be true for dynamic strings.
	bool	m_bShowChoicesListOnSelect;

	bool IsEnabledForPlayer( PlayerNumber pn ) const 
	{
		return m_vEnabledForPlayers.find(pn) != m_vEnabledForPlayers.end(); 
	}

	OptionRowDefinition() { Init(); }
	void Init()
	{
		m_sName = "";
		m_sExplanationName = "";
		m_bOneChoiceForAllPlayers = false;
		m_selectType = SELECT_ONE;
		m_layoutType = LAYOUT_SHOW_ALL_IN_ROW;
		m_vsChoices.clear();
		m_vEnabledForPlayers.clear();
		FOREACH_PlayerNumber( pn )
			m_vEnabledForPlayers.insert( pn );
		m_bExportOnChange = false;
		m_bAllowThemeItems = true;
		m_bAllowThemeTitle = true;
		m_bAllowExplanation = true;
		m_bShowChoicesListOnSelect = false;
	}

	OptionRowDefinition( const char *n, bool b, const char *c0=NULL, const char *c1=NULL, const char *c2=NULL, const char *c3=NULL, const char *c4=NULL, const char *c5=NULL, const char *c6=NULL, const char *c7=NULL, const char *c8=NULL, const char *c9=NULL, const char *c10=NULL, const char *c11=NULL, const char *c12=NULL, const char *c13=NULL, const char *c14=NULL, const char *c15=NULL, const char *c16=NULL, const char *c17=NULL, const char *c18=NULL, const char *c19=NULL )
	{
		Init();
		m_sName=n;
		m_bOneChoiceForAllPlayers=b;
#define PUSH( c )	if(c) m_vsChoices.push_back(c);
		PUSH(c0);PUSH(c1);PUSH(c2);PUSH(c3);PUSH(c4);PUSH(c5);PUSH(c6);PUSH(c7);PUSH(c8);PUSH(c9);PUSH(c10);PUSH(c11);PUSH(c12);PUSH(c13);PUSH(c14);PUSH(c15);PUSH(c16);PUSH(c17);PUSH(c18);PUSH(c19);
#undef PUSH
	}
};

class OptionRowHandler
{
public:
	Commands m_cmds;
	OptionRowDefinition m_Def;
	vector<RString> m_vsReloadRowMessages;	// refresh this row on on these messages
	
	OptionRowHandler() { Init(); }
	virtual ~OptionRowHandler() { }
	virtual void Init()
	{
		m_cmds.v.clear();
		m_Def.Init();
		m_vsReloadRowMessages.clear();
	}
	void Load( const Commands &cmds )
	{
		Init();
		m_cmds = cmds;
		this->LoadInternal( cmds );
	}
	RString OptionTitle() const;
	RString GetThemedItemText( int iChoice ) const;

	virtual void LoadInternal( const Commands &cmds ) { }

	/*
	 * We may re-use OptionRowHandlers.  This is called before each
	 * use.  If the contents of the row are dependent on external
	 * state (for example, the current song), clear the row contents
	 * and reinitialize them.  As an optimization, rows which do not
	 * change can be initialized just once and left alone.
	 *
	 * If the row has been reinitialized, return RELOAD_CHANGED_ALL, and the
	 * graphic elements will also be reinitialized.  If only m_vEnabledForPlayers
	 * has been changed, return RELOAD_CHANGED_ENABLED.  If the row is static, and
	 * nothing has changed, return RELOAD_CHANGED_NONE.
	 */
	enum ReloadChanged { RELOAD_CHANGED_NONE, RELOAD_CHANGED_ENABLED, RELOAD_CHANGED_ALL };
	virtual ReloadChanged Reload() { return RELOAD_CHANGED_NONE; }

	virtual void ImportOption( const vector<PlayerNumber> &vpns, vector<bool> vbSelectedOut[NUM_PLAYERS] ) const { }
	/* Returns an OPT mask. */
	virtual int ExportOption( const vector<PlayerNumber> &vpns, const vector<bool> vbSelected[NUM_PLAYERS] ) const { return 0; }
	virtual void GetIconTextAndGameCommand( int iFirstSelection, RString &sIconTextOut, GameCommand &gcOut ) const;
	virtual RString GetScreen( int iChoice ) const { return RString(); }
};


namespace OptionRowHandlerUtil
{
	OptionRowHandler* Make( const Commands &cmds );
	OptionRowHandler* MakeNull();
	OptionRowHandler* MakeSimple( const MenuRowDef &mrd );

	void SelectExactlyOne( int iSelection, vector<bool> &vbSelectedOut );
	int GetOneSelection( const vector<bool> &vbSelected );
}

inline void VerifySelected( SelectType st, const vector<bool> &vbSelected, const RString &sName )
{
	int iNumSelected = 0;
	if( st == SELECT_ONE )
	{
		ASSERT_M( vbSelected.size() > 0, ssprintf("%s: %i/%i", sName.c_str(), iNumSelected, int(vbSelected.size())) );
		for( unsigned e = 0; e < vbSelected.size(); ++e )
			if( vbSelected[e] )
				iNumSelected++;
		ASSERT_M( iNumSelected == 1, ssprintf("%s: %i/%i", sName.c_str(), iNumSelected, int(vbSelected.size())) );
	}
}

#endif

/*
 * (c) 2002-2004 Chris Danford
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

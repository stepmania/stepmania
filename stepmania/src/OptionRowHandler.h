/* OptionRowHandler - Shows PlayerOptions and SongOptions in icon form. */

#ifndef OptionRowHandler_H
#define OptionRowHandler_H

#include "OptionRow.h"
#include "GameCommand.h"
#include "LuaReference.h"

struct ConfOption;

class OptionRowHandler
{
public:
	CString m_sName;
	bool	m_bExportOnChange;
	vector<CString> m_vsRefreshRowNames;	// refresh these rows when the value of this row changes
	
	OptionRowHandler::OptionRowHandler() { Init(); }
	virtual void Init()
	{
		m_sName = "";
		m_vsRefreshRowNames.clear();
		m_bExportOnChange = false;
	}
	virtual void ImportOption( const OptionRowDefinition &row, PlayerNumber pn, vector<bool> &vbSelectedOut ) const = 0;
	/* Returns an OPT mask. */
	virtual int ExportOption( const OptionRowDefinition &def, PlayerNumber pn, const vector<bool> &vbSelected ) const = 0;
	virtual void Reload( OptionRowDefinition &def ) {}
	virtual CString GetIconText( const OptionRowDefinition &def, int iFirstSelection ) const { return ""; }
	virtual CString GetAndEraseScreen( int iChoice ) { return ""; }
};

class OptionRowHandlerList : public OptionRowHandler
{
public:
	vector<GameCommand> ListEntries;
	GameCommand Default;
	bool m_bUseModNameForIcon;

	OptionRowHandlerList::OptionRowHandlerList() { Init(); }
	virtual void Init()
	{
		OptionRowHandler::Init();
		ListEntries.clear();
		Default.Init();
		m_bUseModNameForIcon = false;
	}
	virtual void ImportOption( const OptionRowDefinition &row, PlayerNumber pn, vector<bool> &vbSelectedOut ) const;
	virtual int ExportOption( const OptionRowDefinition &def, PlayerNumber pn, const vector<bool> &vbSelected ) const;
	virtual CString GetIconText( const OptionRowDefinition &def, int iFirstSelection ) const
	{
		return m_bUseModNameForIcon ?
			ListEntries[iFirstSelection].m_sModifiers :
			def.choices[iFirstSelection];
	}
	virtual CString GetAndEraseScreen( int iChoice )
	{ 
		GameCommand &mc = ListEntries[iChoice];
		if( mc.m_sScreen != "" )
		{
			/* Hack: instead of applying screen commands here, store them in
			* m_sNextScreen and apply them after we tween out.  If we don't set
			* m_sScreen to "", we'll load it twice (once for each player) and
			* then again for m_sNextScreen. */
			CString sNextScreen = mc.m_sScreen;
			mc.m_sScreen = "";
			return sNextScreen;
		}
		return "";
	}
};

class OptionRowHandlerLua : public OptionRowHandler
{
public:
	LuaExpression *m_pLuaTable;

	OptionRowHandlerLua::OptionRowHandlerLua() { m_pLuaTable = NULL; Init(); }
	void Init()
	{
		OptionRowHandler::Init();
		delete m_pLuaTable;
		m_pLuaTable = new LuaExpression;
	}
	virtual void ImportOption( const OptionRowDefinition &row, PlayerNumber pn, vector<bool> &vbSelectedOut ) const;
	virtual int ExportOption( const OptionRowDefinition &def, PlayerNumber pn, const vector<bool> &vbSelected ) const;
	virtual void Reload( OptionRowDefinition &def );
};

class OptionRowHandlerConfig : public OptionRowHandler
{
public:
	const ConfOption *opt;

	OptionRowHandlerConfig::OptionRowHandlerConfig() { Init(); }
	void Init()
	{
		OptionRowHandler::Init();
		opt = NULL;
	}
	virtual void ImportOption( const OptionRowDefinition &row, PlayerNumber pn, vector<bool> &vbSelectedOut ) const;
	virtual int ExportOption( const OptionRowDefinition &def, PlayerNumber pn, const vector<bool> &vbSelected ) const;
};

namespace OptionRowHandlerUtil
{
	void FillList( OptionRowHandlerList* pHand, OptionRowDefinition &def, CString param );
	void FillLua( OptionRowHandlerLua* pHand, OptionRowDefinition &def, CString sLuaFunction );
	void FillSteps( OptionRowHandlerList* pHand, OptionRowDefinition &def );
	void FillConf( OptionRowHandlerConfig* pHand, OptionRowDefinition &def, CString param );
	void FillCharacters( OptionRowHandlerList* pHand, OptionRowDefinition &def );
	void FillStyles( OptionRowHandlerList* pHand, OptionRowDefinition &def );
	void FillGroups( OptionRowHandlerList* pHand, OptionRowDefinition &def );
	void FillDifficulties( OptionRowHandlerList* pHand, OptionRowDefinition &def );

	inline OptionRowHandler* MakeList( OptionRowDefinition &def, CString param )		{ OptionRowHandlerList *pHand = new OptionRowHandlerList; FillList( pHand, def, param );		return pHand; }
	inline OptionRowHandler* MakeLua( OptionRowDefinition &def, CString sLuaFunction )	{ OptionRowHandlerLua *pHand = new OptionRowHandlerLua;   FillLua( pHand, def, sLuaFunction );	return pHand; }
	inline OptionRowHandler* MakeSteps( OptionRowDefinition &def )						{ OptionRowHandlerList *pHand = new OptionRowHandlerList; FillSteps( pHand, def );				return pHand; }
	inline OptionRowHandler* MakeConf( OptionRowDefinition &def, CString param )		{ OptionRowHandlerConfig *pHand = new OptionRowHandlerConfig; FillConf( pHand, def, param );	return pHand; }
	inline OptionRowHandler* MakeCharacters( OptionRowDefinition &def )				{ OptionRowHandlerList *pHand = new OptionRowHandlerList; FillCharacters( pHand, def );			return pHand; }
	inline OptionRowHandler* MakeStyles( OptionRowDefinition &def )					{ OptionRowHandlerList *pHand = new OptionRowHandlerList; FillStyles( pHand, def );				return pHand; }
	inline OptionRowHandler* MakeGroups( OptionRowDefinition &def )					{ OptionRowHandlerList *pHand = new OptionRowHandlerList; FillGroups( pHand, def );				return pHand; }
	inline OptionRowHandler* MakeDifficulties( OptionRowDefinition &def )				{ OptionRowHandlerList *pHand = new OptionRowHandlerList; FillDifficulties( pHand, def );		return pHand; }
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

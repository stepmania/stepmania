#ifndef SCREEN_OPTIONS_MASTER_H
#define SCREEN_OPTIONS_MASTER_H

#include "ScreenOptions.h"
#include "GameCommand.h"
#include "LuaReference.h"

struct ConfOption;

enum OptionRowHandlerType
{
	ROW_LIST, /* list of custom settings */
	ROW_LUA, /* lua tells us what to do */
	ROW_CONFIG,	/* global pref */
	NUM_OPTION_ROW_TYPES
};

class ScreenOptionsMaster: public ScreenOptions
{
public:
	ScreenOptionsMaster( CString sName );
	virtual ~ScreenOptionsMaster();
	void Update( float fDelta );

private:

	struct OptionRowHandler
	{
		OptionRowHandler() { m_pLuaTable=NULL; Init(); }
		void Init()
		{
			type = ROW_LIST;
			m_sName = "";
			m_vsRefreshRowNames.clear();
			ListEntries.clear();
			Default.Init();
			m_bUseModNameForIcon = false;
			delete m_pLuaTable;
			m_pLuaTable = new LuaExpression;
			opt = NULL;
		}

		OptionRowHandlerType type;
		CString m_sName;
		bool	m_bExportOnChange;
		vector<CString> m_vsRefreshRowNames;	// refresh these rows when the value of this row changes

		/* ROW_LIST: */
		vector<GameCommand> ListEntries;
		GameCommand Default;
		bool m_bUseModNameForIcon;

		/* ROW_LUA: */
		LuaExpression *m_pLuaTable;

		/* ROW_CONFIG: */
		const ConfOption *opt;
	};

	CString m_sNextScreen;

	vector<OptionRowHandler> OptionRowHandlers;
	OptionRowDefinition *m_OptionRowAlloc;

	int ExportOption( const OptionRowDefinition &def, const OptionRowHandler &hand, PlayerNumber pn, const vector<bool> &vbSelected );
	void ImportOption( const OptionRowDefinition &def, const OptionRowHandler &hand, PlayerNumber pn, int rowno, vector<bool> &vbSelectedOut );
	int ExportOptionForAllPlayers( int iRow );
	
	static void SetList( OptionRowDefinition &def, OptionRowHandler &hand, CString param );
	static void SetLua( OptionRowDefinition &def, OptionRowHandler &hand, const CString &sLuaFunction );
	static void SetSteps( OptionRowDefinition &def, OptionRowHandler &hand );
	static void SetConf( OptionRowDefinition &def, OptionRowHandler &hand, CString param );
	static void SetCharacters( OptionRowDefinition &def, OptionRowHandler &hand );
	static void SetStyles( OptionRowDefinition &def, OptionRowHandler &hand );
	static void SetGroups( OptionRowDefinition &def, OptionRowHandler &hand );
	static void SetDifficulties( OptionRowDefinition &def, OptionRowHandler &hand );

protected:
	virtual void ChangeValueInRow( PlayerNumber pn, int iDelta, bool Repeat );

	virtual void ImportOptions();
	virtual void ExportOptions();
	virtual void ImportOptionsForPlayer( PlayerNumber pn ); // used by ScreenPlayerOptions

	virtual void GoToNextScreen();
	virtual void GoToPrevScreen();

	virtual void RefreshIcons();
};

#endif

/*
 * (c) 2003-2004 Glenn Maynard
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

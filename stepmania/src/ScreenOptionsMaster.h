#ifndef SCREEN_OPTIONS_MASTER_H
#define SCREEN_OPTIONS_MASTER_H

#include "ScreenOptions.h"
#include "GameCommand.h"

struct ConfOption;

class ScreenOptionsMaster: public ScreenOptions
{
public:
	ScreenOptionsMaster( CString sName );
	virtual ~ScreenOptionsMaster();

private:

	enum OptionRowType
	{
		ROW_LIST, /* list of custom settings */
		ROW_CONFIG,	/* global pref */
		NUM_OPTION_ROW_TYPES
	};

	struct OptionRowHandler
	{
		OptionRowHandler() { opt = NULL; m_bUseModNameForIcon = false; }

		OptionRowType type;

		/* ROW_LIST: */
		vector<GameCommand> ListEntries;
		GameCommand Default;
		bool m_bUseModNameForIcon;

		/* ROW_CONFIG: */
		const ConfOption *opt;
	};

	CString m_sNextScreen;

	vector<OptionRowHandler> OptionRowHandlers;
	OptionRowData *m_OptionRowAlloc;

	int ExportOption( const OptionRowData &row, const OptionRowHandler &hand, PlayerNumber pn, const vector<bool> &vbSelected );
	void ImportOption( const OptionRowData &row, const OptionRowHandler &hand, PlayerNumber pn, int rowno, vector<bool> &vbSelectedOut );
	
	static void SetList( OptionRowData &row, OptionRowHandler &hand, CString param );
	static void SetSteps( OptionRowData &row, OptionRowHandler &hand );
	static void SetConf( OptionRowData &row, OptionRowHandler &hand, CString param );
	static void SetCharacters( OptionRowData &row, OptionRowHandler &hand );
	static void SetStyles( OptionRowData &row, OptionRowHandler &hand );
	static void SetGroups( OptionRowData &row, OptionRowHandler &hand );
	static void SetDifficulties( OptionRowData &row, OptionRowHandler &hand );

protected:
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

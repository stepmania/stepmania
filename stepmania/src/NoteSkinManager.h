#ifndef NoteSkinMANAGER_H
#define NoteSkinMANAGER_H
/*
-----------------------------------------------------------------------------
 Class: NoteSkinManager

 Desc: Manages which graphics and sounds are chosed to load.  Every time 
	a sound or graphic is loaded, it gets the path from the NoteSkinManager.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "RageTypes.h"
#include "Game.h"
#include "PlayerNumber.h"
#include "IniFile.h"
#include <map>


class NoteSkinManager
{
public:
	NoteSkinManager();
	~NoteSkinManager();

	void RefreshNoteSkinData( Game game );
	void GetNoteSkinNames( Game game, CStringArray &AddTo );
	void GetNoteSkinNames( CStringArray &AddTo );	// looks up current Game in GAMESTATE
	bool DoesNoteSkinExist( CString sSkinName );	// looks up current Game in GAMESTATE

	CString GetPathTo( PlayerNumber pn, int col, CString sFileName );
	CString GetPathTo( PlayerNumber pn, CString sButtonName, CString sFileName );
	CString GetPathTo( CString sSkinName, CString sButtonName, CString sFileName, bool bGlobalDefault = false );

	CString		GetMetric( PlayerNumber pn, CString sButtonName, CString sValueName );
	int			GetMetricI( PlayerNumber pn, CString sButtonName, CString sValueName );
	float		GetMetricF( PlayerNumber pn, CString sButtonName, CString sValueName );
	bool		GetMetricB( PlayerNumber pn, CString sButtonName, CString sValueName );
	RageColor	GetMetricC( PlayerNumber pn, CString sButtonName, CString sValueName );

	static CString ColToButtonName(int col);

	CString GetNoteSkinDir( CString sSkinName );

protected:

	struct NoteSkinData
	{
		CString sName;	
		IniFile metrics;
	};
	void LoadNoteSkinData( CString sNoteSkinName, NoteSkinData& data_out );
	map<CString,NoteSkinData> m_mapNameToData;
};



extern NoteSkinManager*	NOTESKIN;	// global and accessable from anywhere in our program

#endif

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
#include "GameConstantsAndTypes.h"

class IniFile;

class NoteSkinManager
{
public:
	NoteSkinManager();
	~NoteSkinManager();

	void GetNoteSkinNames( Game game, CStringArray &AddTo ) const;
	void GetNoteSkinNames( CStringArray &AddTo ) const;	// looks up current Game in GAMESTATE
	bool DoesNoteSkinExist( CString sSkinName ) const;	// looks up current Game in GAMESTATE
	void SwitchNoteSkin( PlayerNumber pn, CString sNewNoteSkin );	// looks up current Game in GAMESTATE
	CString GetCurNoteSkinName( PlayerNumber pn ) const { return m_sCurNoteSkinName[pn]; };

	CString GetPathTo( PlayerNumber pn, int col, CString sFileName );
	CString GetPathTo( CString sSkinName, CString sButtonName, CString sFileName );

	CString		GetMetric( PlayerNumber pn, CString sButtonName, CString sValueName );
	int			GetMetricI( PlayerNumber pn, CString sButtonName, CString sValueName );
	float		GetMetricF( PlayerNumber pn, CString sButtonName, CString sValueName );
	bool		GetMetricB( PlayerNumber pn, CString sButtonName, CString sValueName );
	RageColor	GetMetricC( PlayerNumber pn, CString sButtonName, CString sValueName );

	static CString NoteSkinManager::ColToButtonName(int col);

protected:
	CString GetNoteSkinDir( CString sSkinName );

	CString m_sCurNoteSkinName[NUM_PLAYERS];	
	IniFile* m_pIniMetrics[NUM_PLAYERS];
	unsigned m_uHashForCurThemeMetrics[NUM_PLAYERS];
	unsigned m_uHashForBaseThemeMetrics[NUM_PLAYERS];
};



extern NoteSkinManager*	NOTESKIN;	// global and accessable from anywhere in our program

#endif

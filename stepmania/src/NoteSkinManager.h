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

class IniFile;

class NoteSkinManager
{
public:
	NoteSkinManager();
	~NoteSkinManager();

	void GetNoteSkinNames( Game game, CStringArray &AddTo ) const;
	void GetNoteSkinNames( CStringArray &AddTo ) const;	// looks up current Game in GAMESTATE
	bool DoesNoteSkinExist( CString sSkinName ) const;	// looks up current Game in GAMESTATE
	void SwitchNoteSkin( CString sNewNoteSkin );	// looks up current Game in GAMESTATE
	CString GetCurNoteSkinName() const { return m_sCurNoteSkinName; };

	CString GetPathTo( int col, CString sFileName );

	CString		GetMetric( CString sButtonName, CString sValueName );
	int			GetMetricI( CString sButtonName, CString sValueName );
	float		GetMetricF( CString sButtonName, CString sValueName );
	bool		GetMetricB( CString sButtonName, CString sValueName );
	RageColor	GetMetricC( CString sButtonName, CString sValueName );

	static CString NoteSkinManager::ColToButtonName(int col);

protected:
	CString GetPathTo( CString sSkinName, CString sButtonName, CString sFileName );
	CString GetNoteSkinDir( CString sSkinName );

	CString m_sCurNoteSkinName;	
	IniFile* m_pIniMetrics;
	unsigned m_uHashForCurThemeMetrics;
	unsigned m_uHashForBaseThemeMetrics;
};



extern NoteSkinManager*	NOTESKIN;	// global and accessable from anywhere in our program

#endif

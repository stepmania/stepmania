#ifndef NoteSkinMANAGER_H
#define NoteSkinMANAGER_H

#include "RageTypes.h"
#include "PlayerNumber.h"
#include "IniFile.h"
#include <map>

class GameDef;

class NoteSkinManager
{
public:
	NoteSkinManager();
	~NoteSkinManager();

	void RefreshNoteSkinData( const GameDef* game );
	void GetNoteSkinNames( const GameDef* game, CStringArray &AddTo );
	void GetNoteSkinNames( CStringArray &AddTo );	// looks up current const GameDef* in GAMESTATE
	bool DoesNoteSkinExist( CString sSkinName );	// looks up current const GameDef* in GAMESTATE

	CString GetPathToFromPlayerAndCol( PlayerNumber pn, int col, CString sElement, bool bOptional=false );
	CString GetPathToFromNoteSkinAndButton( CString NoteSkin, CString sButtonName, CString sElement, bool bOptional=false );
	CString GetPathToFromPlayerAndButton( PlayerNumber pn, CString sButtonName, CString sElement, bool bOptional=false );

	CString		GetMetric( CString sNoteSkinName, CString sButtonName, CString sValue );
	int			GetMetricI( CString sNoteSkinName, CString sButtonName, CString sValueName );
	float		GetMetricF( CString sNoteSkinName, CString sButtonName, CString sValueName );
	bool		GetMetricB( CString sNoteSkinName, CString sButtonName, CString sValueName );
	RageColor	GetMetricC( CString sNoteSkinName, CString sButtonName, CString sValueName );

	static CString ColToButtonName(int col);

	CString GetNoteSkinDir( CString sSkinName );

protected:
	CString GetPathToFromDir( CString sDir, CString sFileName );

	struct NoteSkinData
	{
		CString sName;	
		IniFile metrics;
	};
	void LoadNoteSkinData( CString sNoteSkinName, NoteSkinData& data_out );
	map<CString,NoteSkinData> m_mapNameToData;
	const GameDef* m_pCurGame;
};



extern NoteSkinManager*	NOTESKIN;	// global and accessable from anywhere in our program

#endif

/*
 * (c) 2003-2004 Chris Danford
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

#ifndef NoteSkinMANAGER_H
#define NoteSkinMANAGER_H

#include "Command.h"
#include "RageTypes.h"
#include "PlayerNumber.h"
#include "IniFile.h"
#include <map>
#include <deque>

class Game;

class NoteSkinManager
{
public:
	NoteSkinManager();
	~NoteSkinManager();

	void RefreshNoteSkinData( const Game* game );
	void GetNoteSkinNames( const Game* game, CStringArray &AddTo, bool bFilterDefault=true );
	void GetNoteSkinNames( CStringArray &AddTo );	// looks up current const Game* in GAMESTATE
	bool DoesNoteSkinExist( const CString &sSkinName );	// looks up current const Game* in GAMESTATE

	CString GetPathToFromNoteSkinAndButton( const CString &sNoteSkin, const CString &sButtonName, const CString &sElement, bool bOptional=false );

	CString		GetMetric( const CString &sNoteSkinName, const CString &sButtonName, const CString &sValue );
	int			GetMetricI( const CString &sNoteSkinName, const CString &sButtonName, const CString &sValueName );
	float		GetMetricF( const CString &sNoteSkinName, const CString &sButtonName, const CString &sValueName );
	bool		GetMetricB( const CString &sNoteSkinName, const CString &sButtonName, const CString &sValueName );
	RageColor	GetMetricC( const CString &sNoteSkinName, const CString &sButtonName, const CString &sValueName );
	Commands   GetMetricA( const CString &sNoteSkinName, const CString &sButtonName, const CString &sValueName );

	CString GetNoteSkinDir( const CString &sSkinName );

protected:
	CString GetPathToFromDir( const CString &sDir, const CString &sFileName );

	struct NoteSkinData
	{
		CString sName;	
		IniFile metrics;

		// When looking for an element, search these dirs from head to tail.
		deque<CString> vsDirSearchOrder;
	};
	void LoadNoteSkinData( const CString &sNoteSkinName, NoteSkinData& data_out );
	void LoadNoteSkinDataRecursive( const CString &sNoteSkinName, NoteSkinData& data_out );
	map<CString,NoteSkinData> m_mapNameToData;
	const Game* m_pCurGame;
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

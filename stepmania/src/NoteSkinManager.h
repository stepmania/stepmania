/* NoteSkinManager - Loads note skins. */

#ifndef NOTE_SKIN_MANAGER_H
#define NOTE_SKIN_MANAGER_H

#include "ActorCommands.h"
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
	void GetNoteSkinNames( const Game* game, vector<CString> &AddTo, bool bFilterDefault=true );
	void GetNoteSkinNames( vector<CString> &AddTo );	// looks up current const Game* in GAMESTATE
	bool DoesNoteSkinExist( const CString &sNoteSkin );	// looks up current const Game* in GAMESTATE

	void SetCurrentNoteSkin( const CString &sNoteSkin ) { m_sCurrentNoteSkin = sNoteSkin; }
	const CString &GetCurrentNoteSkin() { return m_sCurrentNoteSkin; }

	CString GetPath( const CString &sButtonName, const CString &sElement );

	CString		GetMetric( const CString &sButtonName, const CString &sValue );
	int			GetMetricI( const CString &sButtonName, const CString &sValueName );
	float		GetMetricF( const CString &sButtonName, const CString &sValueName );
	bool		GetMetricB( const CString &sButtonName, const CString &sValueName );
	apActorCommands   GetMetricA( const CString &sButtonName, const CString &sValueName );

	// Lua
	void PushSelf( lua_State *L );

protected:
	CString GetNoteSkinDir( const CString &sSkinName );
	CString GetPathFromDirAndFile( const CString &sDir, const CString &sFileName );

	struct NoteSkinData
	{
		CString sName;	
		IniFile metrics;

		// When looking for an element, search these dirs from head to tail.
		deque<CString> vsDirSearchOrder;
	};
	void LoadNoteSkinData( const CString &sNoteSkinName, NoteSkinData& data_out );
	void LoadNoteSkinDataRecursive( const CString &sNoteSkinName, NoteSkinData& data_out );
	CString m_sCurrentNoteSkin;
	map<CString,NoteSkinData> m_mapNameToData;
	const Game* m_pCurGame;
};



extern NoteSkinManager*	NOTESKIN;	// global and accessable from anywhere in our program

class LockNoteSkin
{
public:
	LockNoteSkin( const CString &sNoteSkin ) { ASSERT( NOTESKIN->GetCurrentNoteSkin().empty() ); NOTESKIN->SetCurrentNoteSkin( sNoteSkin ); }
	~LockNoteSkin() { NOTESKIN->SetCurrentNoteSkin( CString() ); }
};


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

#pragma once
/*
-----------------------------------------------------------------------------
 Class: GameManager

 Desc: Manages GameDefs (which define different games, like "dance" and "pump")
	and StyleDefs (which define different games, like "single" and "couple")

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/
#include "GameDef.h"
#include "StyleDef.h"


class GameManager
{
public:
	GameManager();

	GameDef*	GetGameDefForGame( Game g );
	StyleDef*	GetStyleDefForStyle( Style s );

	Style		GetStyleThatPlaysNotesType( NotesType nt );

	void GetGameNames( CStringArray &AddTo );
	bool DoesGameExist( CString sGameName );

	void GetNoteSkinNames( CStringArray &AddTo );	// looks up current Game in GAMESTATE
	bool DoesNoteSkinExist( CString sSkinName );	// looks up current Game in GAMESTATE
	void SwitchNoteSkin( CString sNewNoteSkin );	// looks up current Game in GAMESTATE
	CString GetCurNoteSkin() { return m_sCurNoteSkin; };

	CString GetPathTo( const int col, const SkinElement gbg );	// looks in GAMESTATE for the current Style
	void GetTweenColors( const int col, CArray<D3DXCOLOR,D3DXCOLOR> &aTweenColorsAddTo );	// looks in GAMESTATE for the current Style

protected:

	CString GetPathTo( Game g, CString sSkinName, CString sButtonName, const SkinElement gbg );
	void GetTweenColors( Game g, CString sSkinName, CString sButtonName, CArray<D3DXCOLOR,D3DXCOLOR> &aTweenColorsAddTo );

	CString m_sCurNoteSkin;	

};

extern GameManager*	GAMEMAN;	// global and accessable from anywhere in our program

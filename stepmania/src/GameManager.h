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
#include "Style.h"
#include "Game.h"


class GameManager
{
public:
	GameManager();

	GameDef*	GetGameDefForGame( Game g );
	const StyleDef*	GetStyleDefForStyle( Style s );

	void		GetGameplayStylesForGame( Game game, CArray<Style,Style>& aStylesAddTo, bool editor=false );
	void		GetNotesTypesForGame( Game game, CArray<NotesType,NotesType>& aNotesTypeAddTo );	// only look at edit-specific styles

//	void GetGameNames( CStringArray &AddTo );
//	bool DoesGameExist( CString sGameName );

	void GetNoteSkinNames( CStringArray &AddTo );	// looks up current Game in GAMESTATE
	bool DoesNoteSkinExist( CString sSkinName );	// looks up current Game in GAMESTATE
	void SwitchNoteSkin( CString sNewNoteSkin );	// looks up current Game in GAMESTATE
	CString GetCurNoteSkin() { return m_sCurNoteSkin; };

	CString GetPathTo( const int col, const SkinElement gbg );	// looks in GAMESTATE for the current Style
	void GetTapTweenColors( const int col, CArray<D3DXCOLOR,D3DXCOLOR> &aTapTweenColorsAddTo );	// looks in GAMESTATE for the current Style
	void GetHoldTweenColors( const int col, CArray<D3DXCOLOR,D3DXCOLOR> &aHoldTweenColorsAddTo );	// looks in GAMESTATE for the current Style

	void GetEnabledGames( CArray<Game,Game>& aGamesOut );

	static int NotesTypeToNumTracks( NotesType nt );
	static NotesType StringToNotesType( CString sNotesType );
	static CString NotesTypeToString( NotesType nt );

protected:
	void GetNoteSkinNames( Game game, CStringArray &AddTo );

	CString m_sCurNoteSkin;	

};

extern GameManager*	GAMEMAN;	// global and accessable from anywhere in our program

#ifndef MODECHOICE_H
#define MODECHOICE_H
/*
-----------------------------------------------------------------------------
 Class: ModeChoice

 Desc: .

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Game.h"
#include "Style.h"
#include "GameConstantsAndTypes.h"
#include "PlayerNumber.h"

class Song;
class Steps;
class Character;

struct ModeChoice		// used in SelectMode
{
	ModeChoice() { Init(); }
	void Init();

	void Load( int iIndex, CString str );
	void ApplyToAllPlayers() const;
	void Apply( PlayerNumber pn ) const;
	bool DescribesCurrentMode( PlayerNumber pn ) const;
	bool DescribesCurrentModeForAllPlayers() const;
	bool IsPlayable( CString *why = NULL ) const;
	bool IsZero() const;

	CString		m_sName;	// display name
	bool		m_bInvalid;
	int			m_iIndex;
	Game		m_game;
	Style		m_style;
	PlayMode	m_pm;
	Difficulty	m_dc;
	CString		m_sAnnouncer;
	CString		m_sModifiers;
	CString		m_sScreen;
	Song*		m_pSong;
	Steps*		m_pSteps;
	Character*	m_pCharacter;
	CourseDifficulty	m_CourseDifficulty;
};

#endif

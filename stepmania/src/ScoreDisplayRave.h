#ifndef ScoreDisplayRave_H
#define ScoreDisplayRave_H
/*
-----------------------------------------------------------------------------
 Class: ScoreDisplayRave

 Desc: Shows point score during gameplay and used in some menus.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScoreDisplay.h"
#include "GameConstantsAndTypes.h"
#include "OptionIcon.h"


class ScoreDisplayRave : public ScoreDisplay
{
public:
	ScoreDisplayRave();
	virtual void Init( PlayerNumber pn );

	virtual void Update( float fDelta );

protected:
	Sprite		m_sprFrame;
	BitmapText	m_textCurrentAttack;

	CString m_sLastSeenAttack;
};

#endif

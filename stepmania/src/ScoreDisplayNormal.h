#ifndef SCOREDISPLAYNORMAL_H
#define SCOREDISPLAYNORMAL_H
/*
-----------------------------------------------------------------------------
 Class: ScoreDisplayNormal

 Desc: Shows point score during gameplay and used in some menus.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScoreDisplay.h"
#include "BitmapText.h"



class ScoreDisplayNormal : public ScoreDisplay
{
public:
	ScoreDisplayNormal();

	virtual void Init( PlayerNumber pn );

	virtual void Update( float fDeltaTime );

	virtual void SetScore( int iNewScore );
	virtual void SetText( CString s );

protected:
	Sprite		m_sprFrame;
	BitmapText	m_text;

	int   m_iScore;			// the actual score
	int   m_iTrailingScore;	// what is displayed temporarily
	int   m_iScoreVelocity;	// how fast trailing approaches real score
};

#endif

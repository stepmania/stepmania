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

	virtual void SetScore( float fNewScore );
	virtual void SetText( CString s );

protected:
	BitmapText m_text;

	float m_fScore;			// the actual score
	float m_fTrailingScore;	// what is displayed temporarily
	float m_fScoreVelocity;	// how fast trailing approaches real score
};

#endif

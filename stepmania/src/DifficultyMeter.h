#ifndef DifficultyMeter_H
#define DifficultyMeter_H
/*
-----------------------------------------------------------------------------
 Class: DifficultyMeter

 Desc: A meter represention of how hard Steps is.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "BitmapText.h"
#include "PlayerNumber.h"
#include "ActorFrame.h"
#include "GameConstantsAndTypes.h"
#include "ActorUtil.h"

class Steps;
class Course;


class DifficultyMeter: public ActorFrame
{
public:
	DifficultyMeter();

	void Load();
	void SetFromGameState( PlayerNumber pn );
	void SetFromNotes( const Steps* pNotes );
	void SetFromCourse( const Course* pCourse, PlayerNumber pn );
	void Unset();

private:
	void SetMeter( int iMeter, Difficulty dc );
	void SetDifficulty( CString diff );
	BitmapText		m_textFeet;

	CString			m_CurDifficulty;
	AutoActor		m_Difficulty;

	BitmapText		m_textMeter;
};

#endif

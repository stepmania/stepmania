/*
-----------------------------------------------------------------------------
 File: Combo.h

 Desc: A graphic displayed in the Combo during Dancing.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/


#ifndef _Combo_H_
#define _Combo_H_


#include "ActorFrame.h"
#include "Sprite.h"
#include "ScoreDisplayRolling.h"



class Combo : public ActorFrame
{
public:
	Combo();

	void ContinueCombo();
	void EndCombo();

	int GetCurrentCombo();
	int GetMaxCombo();

protected:
	int			m_iCurCombo;
	int			m_iMaxCombo;

	bool		m_bComboVisible;
	Sprite		m_sprCombo;
	BitmapText	m_textComboNumber;
};

#endif
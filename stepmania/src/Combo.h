#pragma once
/*
-----------------------------------------------------------------------------
 Class: Combo

 Desc: Text that displays the size of the current combo.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ActorFrame.h"
#include "Sprite.h"
#include "BitmapText.h"



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

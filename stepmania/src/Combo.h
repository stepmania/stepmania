#ifndef COMBO_H
#define COMBO_H
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
#include "GameConstantsAndTypes.h"	// for TapNoteScore



class Combo : public ActorFrame
{
public:
	Combo();

	void SetScore( TapNoteScore score, int iNumNotesInThisRow );

	int GetCurrentCombo() const { return m_iCurCombo; }
	int GetMaxCombo() const { return m_iMaxCombo; }
	void Reset();

protected:
	int			m_iCurCombo;
	int			m_iMaxCombo;
	int			m_iCurComboOfPerfects;

	Sprite		m_sprCombo;
	BitmapText	m_textComboNumber;
};

#endif

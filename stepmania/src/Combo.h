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
#include "PlayerNumber.h"
#include "GameConstantsAndTypes.h"	// for TapNoteScore
class Inventory;


class Combo : public ActorFrame
{
public:
	Combo();

	void Init( PlayerNumber pn ) { m_PlayerNumber = pn; }

	void SetCombo( int iCombo );

protected:
	PlayerNumber m_PlayerNumber;

	Sprite		m_sprCombo;
	BitmapText	m_textComboNumber;
};

#endif

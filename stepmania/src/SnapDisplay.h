#pragma once
/*
-----------------------------------------------------------------------------
 Class: SnapDisplay

 Desc: Graphics on ends of receptors on Edit screen that show the current snap type.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "GrayArrow.h"
#include "ActorFrame.h"
#include "StyleDef.h"
#include "GameConstantsAndTypes.h"
#include "Sprite.h"
#include "StyleDef.h"



class SnapDisplay : public ActorFrame
{
public:
	SnapDisplay();
	
	void Load( PlayerNumber pn );

	bool PrevSnapMode();
	bool NextSnapMode();

	NoteType GetNoteType() { return m_NoteType; };

protected:
	int m_iNumCols;

	void SnapModeChanged();

	NoteType	m_NoteType;		// snap to this note type
	Sprite		m_sprIndicators[2];	// left and right side
};


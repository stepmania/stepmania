#ifndef SNAPDISPLAY_H
#define SNAPDISPLAY_H

#include "ActorFrame.h"
#include "NoteTypes.h"
#include "Sprite.h"

/** @brief Graphics on ends of receptors on Edit screen that show the current snap type. */
class SnapDisplay : public ActorFrame
{
public:
	SnapDisplay();

	void Load();

	bool PrevSnapMode();
	bool NextSnapMode();

	NoteType GetNoteType() const { return m_NoteType; }

protected:
	int m_iNumCols;

	void SnapModeChanged();

	/** @brief the NoteType to snap to. */
	NoteType	m_NoteType;
	/**
	 * @brief Indicators showing what NoteType is currently snapped to.
	 * 
	 * TODO: Convert to an AutoActor. -aj */
	Sprite		m_sprIndicators[2];	// left and right side
};


#endif

/**
 * @file
 * @author Chris Danford (c) 2001-2002
 * @section LICENSE
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

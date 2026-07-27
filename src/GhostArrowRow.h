#ifndef GHOSTARROWROW_H
#define GHOSTARROWROW_H

#include "ActorFrame.h"
#include "GameConstantsAndTypes.h"
#include "NoteTypes.h"
#include "NoteDisplay.h"

#include <vector>


class PlayerState;
/** @brief Row of GhostArrow Actors. */
class GhostArrowRow : public ActorFrame
{
public:
	virtual ~GhostArrowRow();
	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();

	void Load( const PlayerState* pPlayerState, float fYReverseOffset );
	void SetColumnRenderers(std::vector<NoteColumnRenderer>& renderers);

	void DidTapNote( int iCol, TapNoteScore tns, bool bBright );
	void DidHoldNote( int iCol, HoldNoteScore hns, bool bBright );
	void SetHoldShowing( int iCol, const TapNote &tn );

protected:
	float m_fYReverseOffsetPixels;
	const PlayerState* m_pPlayerState;

	std::vector<NoteColumnRenderer> const* m_renderers;
	std::vector<Actor *> 	m_Ghost;
	std::vector<TapNoteSubType> m_bHoldShowing;
	std::vector<TapNoteSubType> m_bLastHoldShowing;
};


#endif

/**
 * @file
 * @author Chris Danford (c) 2001-2004
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

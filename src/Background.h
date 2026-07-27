#ifndef BACKGROUND_H
#define BACKGROUND_H

#include "ActorFrame.h"
#include "Quad.h"
#include "PlayerNumber.h"
#include "BackgroundUtil.h"

#include <vector>


class DancingCharacters;
class Song;
class BackgroundImpl;
/** @brief the Background that is behind the notes while playing. */
class Background : public ActorFrame
{
public:
	Background();
	~Background();
	void Init();

	virtual void LoadFromSong( const Song *pSong );
	virtual void Unload();

	void FadeToActualBrightness();
	void SetBrightness( float fBrightness ); // overrides pref and Cover

	// One more piece of the puzzle that puts the notefield board above the bg
	// and under everything else.  m_disable_draw exists so that
	// ScreenGameplay can draw the background manually, and still have it as a
	// child. -Kyz
	bool m_disable_draw;
	virtual bool EarlyAbortDraw() const { return m_disable_draw; }

	/**
	 * @brief Retrieve whatever dancing characters are in use.
	 * @return the dancing characters. */
	DancingCharacters* GetDancingCharacters();

	void GetLoadedBackgroundChanges( std::vector<BackgroundChange> **pBackgroundChangesOut );

protected:
	BackgroundImpl *m_pImpl;
};

#endif

/**
 * @file
 * @author Chris Danford, Ben Nordstrom (c) 2001-2004
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
